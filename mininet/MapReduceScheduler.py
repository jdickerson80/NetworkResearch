#!/usr/bin/python

from enum import IntEnum
import threading
from MapReduceJob import *
from MapReduceHost import *
from PerProcessPipes import *
from multiprocessing import Pipe
from Queue import Queue
import time
import sys
import time

class Statistics( object ):
	def __init__( self ):
		self.job = "Empty"
		self.startTime = 0
		self.endTime = 0
		self.hosts = []
		self.bytesToSend = None
		self.reduceJob = 0

	def __str__( self ):
		hosts = '[%s]' % ', '.join( map( str, self.hosts ) )
		return "%s, %s, %s, %s, %i, %i" % ( self.job, self.startTime, self.endTime, hosts, self.bytesToSend, self.reduceJob )

class MapReduceScheduler( object ):

	class BytesPerHostEnum( IntEnum ):
		BytesPerHost = 64000000

	def __init__( self, hostList, workFile ):
		with open( workFile ) as f:
			data = f.readlines()
		self.workQueue = Queue()
		self.numberOfHosts = int ( len( hostList ) )
		self.hostList = hostList
		self.__preprocessJobList( data )
		self.__processJobs( data )
		self.jobList = []
		self.availableJobList = [ True for i in xrange( self.numberOfHosts ) ]
		self.hostMapReduceList = []
		self.availableList = [ 2 for i in xrange( self.numberOfHosts ) ]
		self.jobPipeList = [ PerProcessPipes() for i in xrange( self.numberOfHosts ) ]
		self.hostPipeList = [ PerProcessPipes() for i in xrange( self.numberOfHosts ) ]
		self.handleJobPipeThreadRunning = True
		counter = 0

		print self.availableList
		for i in self.jobPipeList:
			i.parentConnection, i.childConnection = Pipe()

		for i in self.hostPipeList:
			i.parentConnection, i.childConnection = Pipe()

		for i in self.hostList:
			self.hostMapReduceList.append( HostMapReduce( host = i, schedularPipe = self.hostPipeList[ counter ].childConnection ) )
			counter = counter + 1

		for i in xrange( self.numberOfHosts ):
			job = MapReduceJob( schedulerPipe = self.jobPipeList[ i ].childConnection, hostMapReduceList = self.hostMapReduceList, hostPipeList = self.hostPipeList )
			job.start()
			self.jobList.append( job )

		self.hasListBeenUpdated = True
		self.jobStats = []
		self.handleJobPipeThread = threading.Thread( target=self.handleJobPipe )

		self.handleJobPipeThread.start()
		print "Queue size: %i" % self.workQueue.qsize()

		for i in self.hostPipeList:
			i.parentConnection.recv()

	def terminate( self ):
		self.handleJobPipeThreadRunning = False
		for i in self.hostList:
			i.terminate()

		for i in self.jobList:
			i.terminate()

		for i in self.jobList:
			i.join()

		self.handleJobPipeThread.join()

	def handleJobPipe( self ):
		jobStatistic = None
		time.sleep( 0.5 )
		while self.handleJobPipeThreadRunning == True:
			for i in self.jobPipeList:
				poll = i.parentConnection.poll()
				if poll == True:
					message = i.parentConnection.recv()
					jobStatistic = message
					for i in jobStatistic.hosts:
						self.availableList[ i ] = 2
						self.availableJobList[ jobStatistic.reduceJob ] = True

					for i in xrange( len( self.jobStats ) ):
						if self.jobStats[ i ].job == jobStatistic.job:
							self.jobStats[ i ] = jobStatistic
							print self.jobStats[ i ]
							break
					self.hasListBeenUpdated = True
		#                    print self.availableJobList
			time.sleep( 0.025 )

	@staticmethod
	def getTime():
		return time.strftime( "%H:%M:%S", time.localtime() )

	@staticmethod
	def __preprocessJobList( workFile ):
		for job in workFile:
			job = job.replace( "\n", "" )
			job = job.replace( "job ", "" )
			job = job.replace( " ", "" )

	def __processJobs( self, workFile ):
		inputBytes = 0
		hostNeeded = 0
		for job in workFile:
			jobComponents = job.split( "\t" )
			if jobComponents[ 0 ] == '\n':
				continue

			if int( jobComponents[ 2 ] ) == 0:
				continue

			inputBytes = int( jobComponents[ 1 ] )
			remainder = inputBytes % self.BytesPerHostEnum.BytesPerHost
			hostsNeeded = inputBytes / self.BytesPerHostEnum.BytesPerHost

			if remainder != 0 or hostsNeeded == 0:
				hostsNeeded = hostsNeeded + 1

			if ( int ( hostsNeeded ) > int ( self.numberOfHosts ) ):
				continue

			self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ), jobComponents[ 0 ] ], block=False )

	def findHosts( self, numberOfHosts, requiredHosts, counter ):
		currentAvailableHostList = self.availableList
		#        print "test %s" % currentAvailableHostList
		for j in xrange( numberOfHosts ):
			if currentAvailableHostList[ j ] == 2:
				mapperHost = j
				for h in xrange( numberOfHosts - 1, 0, -1 ):
					if currentAvailableHostList[ h ] == 2:
						if h == j:
							print "same host"
							break
						elif j > h:
							print "wrap around"
							break

						reducerHost = h
						requiredHosts = requiredHosts - 1
						currentAvailableHostList[ j ] = 0
						currentAvailableHostList[ h ] = 0
						self.jobStats[ counter ].hosts.append( reducerHost )
						self.jobStats[ counter ].hosts.append( mapperHost )
						break
					else:
						if h == 0:
							j = 0
							break
		return requiredHosts

	def runTest( self ):
		if self.workQueue.empty() == True:
			raise LookupError( "Running test on an empty queue" )
		currentJob = []
		numberOfHosts = self.numberOfHosts
		counter = 0
		#        for i in xrange( self.workQueue.qsize() ):
		#            print self.workQueue.get_nowait()

		for i in xrange( self.workQueue.qsize() ):
			currentAvailableHostList = self.availableList
			currentJob = self.workQueue.get_nowait()
			requiredHosts = currentJob[ 0 ]
			tempHosts = requiredHosts
			totalRequiredHosts = currentJob[ 0 ]
			#            print currentJob
			self.jobStats.append( Statistics() )
			self.jobStats[ counter ].job = currentJob[ 2 ]
			self.jobStats[ counter ].bytesToSend = currentJob[ 1 ] / totalRequiredHosts
			while requiredHosts > 0:
				self.hasListBeenUpdated = False
				tempHosts = self.findHosts( numberOfHosts, requiredHosts, counter )
				if requiredHosts == tempHosts:
					print "WAITING"
					while ( True ):
						if self.hasListBeenUpdated == True:
							break
						time.sleep( 0.125 )
				else:
					requiredHosts = tempHosts
					time.sleep( 0.125 )
				time.sleep( 0.125 )

			for i in xrange( len( self.availableJobList ) ):
				if self.availableJobList[ i ] == True:
					self.jobStats[ counter ].reduceJob = i
					self.availableJobList[ i ] = False
					self.jobPipeList[ i ].parentConnection.send( self.jobStats[ counter ] )
		#		    print i
		#		    print self.jobStats[ counter ]
		#		    print self.availableJobList
					break
			counter = counter + 1

		while True:
			done = True
			for i in self.availableJobList:
				if i == False:
					done = False
					break
			if done == True:
				print "BROKE!"
				print self.availableJobList
				break
			time.sleep( 0.125 )

		with open('jobLog%s.txt' % self.getTime(), 'w') as f:
			for item in self.jobStats:
				f.write("%s\n" % item)



#   for i in self.hostPipeList:
#       poll = i.parentConnection.poll()
#       if poll == True:
#       print "POLL TRUE"
#       message = i.parentConnection.recv()
#       print message

#   for i in self.hostPipeList:
#       i.parentConnection.recv()