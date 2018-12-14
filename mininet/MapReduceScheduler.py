#!/usr/bin/python

from enum import IntEnum
from MapReduceJob import *
from MapReduceHost import *
from PerProcessPipes import *
from multiprocessing import Pipe
from Queue import Queue
from JobStatistic import *
import threading
import time
import sys
import time

class MapReduceScheduler( object ):
	""" 
	This class handles the scheduling of the map reduce jobs. It figures out the hosts to 
	run the job, and sends the info to the MapReduceJob to handle the running of the job.
	"""
	
	# Bytes per host to divide up the job
	class BytesPerHostEnum( IntEnum ):
		BytesPerHost = 64000000

	# constructor
	def __init__( self, hostList, workFile ):
		# cache the work file lines
		with open( workFile ) as f:
			data = f.readlines()
		# create the queue
		self.workQueue = Queue()
		# get the length of the hosts list, which is the number of the hosts
		self.numberOfHosts = int ( len( hostList ) )
		self.hostList = hostList
		# preprocess the job list
		self.__preprocessJobList( data )
		# add the jobs to the queue
		self.__processJobs( data )
		self.jobList = []
		# create the available job list, which is used to figure out what map reduce job to assign the job
		self.availableJobList = [ True for i in xrange( self.numberOfHosts / 2 ) ]
		self.hostMapReduceList = []
		# available hosts. two mappers per host
		self.availableList = [ 2 for i in xrange( self.numberOfHosts ) ]
		self.jobPipeList = [ PerProcessPipes() for i in xrange( self.numberOfHosts / 2 ) ]
		self.hostPipeList = [ PerProcessPipes() for i in xrange( self.numberOfHosts ) ]
		self.handleJobPipeThreadRunning = True
		counter = 0

		# create the pipes
		for i in self.jobPipeList:
			i.parentConnection, i.childConnection = Pipe()
		for i in self.hostPipeList:
			i.parentConnection, i.childConnection = Pipe()

		# create the hosts' map reduce clients
		for i in self.hostList:
			self.hostMapReduceList.append( HostMapReduce( host = i, schedularPipe = self.hostPipeList[ counter ].childConnection ) )
			counter += 1

		# create the map reduce jobs. It is half of the number of hosts because each job takes two hosts at a minimum
		for i in xrange( self.numberOfHosts / 2 ):
			job = MapReduceJob( schedulerPipe = self.jobPipeList[ i ].childConnection, hostMapReduceList = self.hostMapReduceList, hostPipeList = self.hostPipeList )
			job.start()
			self.jobList.append( job )

		print len( self.jobList )	
		# set the list to be updated, so the job can be scheduled
		self.hasListBeenUpdated = True
		self.jobStats = []

		# get the thread to handle the MapReduceJob communications
		self.handleJobPipeThread = threading.Thread( target=self.handleJobPipe )

		# start the thread
		self.handleJobPipeThread.start()
		print "Queue size: %i" % self.workQueue.qsize()

		# flush the hosts' buffer
		for i in self.hostPipeList:
			i.parentConnection.recv()


	def terminate( self ):
		"""
		Shut down all of threads and processes	
		"""
		# set the pipe thread running to false to shut it down
		self.handleJobPipeThreadRunning = False

		# turn off all of the hosts' map reduce list
		for i in self.hostMapReduceList:
			i.terminate()

		# turn off all of the map reduce jobs list
		for i in self.jobList:
			i.terminate()

		# join the job list processes
		for i in self.jobList:
			i.join()

		# join the pipe thread
		self.handleJobPipeThread.join()

	def handleJobPipe( self ):
		"""
		Get the stats back from the MapReduce job and add the appropriate data to the available lists
		"""
		time.sleep( 0.5 )
		# while the thread should run
		while self.handleJobPipeThreadRunning == True:
			# loop through the job pipes
			for i in self.jobPipeList:
				# get the data from poll
				poll = i.parentConnection.poll()
				# if poll is true
				if poll == True:
					# get the data
					message = i.parentConnection.recv()

					# "cast" it to a jobStatistic
					jobStatistic = message

					# set all the hosts back to available
					for i in jobStatistic.hosts:
						self.availableList[ i ] = 2

					# set the job back to available
					self.availableJobList[ jobStatistic.reduceJob ] = True

					# find the job in the job list
					for i in xrange( len( self.jobStats ) ):
						if self.jobStats[ i ].job == jobStatistic.job:
							# set the job equal to the appropriate list position
							self.jobStats[ i ] = jobStatistic
							print "job %s completed" % ( self.jobStats[ i ].job )
							break

					# set the list to updated		
					self.hasListBeenUpdated = True
			# loop sleep time
			time.sleep( 0.025 )

	@staticmethod
	def getTime():
		"""
		Get the time in hours, minutes, and seconds
		"""
		return time.strftime( "%H:%M:%S", time.localtime() )

	@staticmethod
	def __preprocessJobList( workFile ):
		"""
		Remove the \n, job , and all spaces from the work file. 
		"""
		for job in workFile:
			job = job.replace( "\n", "" )
			job = job.replace( "job ", "" )
			job = job.replace( " ", "" )

	def __processJobs( self, workFile ):
		"""
		Compute the proper values from the work file
		"""
		inputBytes = 0
		hostNeeded = 0

		# for each job in the file
		for job in workFile:
			# split line by tabs
			jobComponents = job.split( "\t" )

			# if the line is empty, continue
			if jobComponents[ 0 ] == '\n':
				continue

			# if the reduce bytes is empty
			if int( jobComponents[ 2 ] ) == 0:
				continue

			# convert the string to an int
			inputBytes = int( jobComponents[ 1 ] )

			# get the remainder of dividing the inputBytes / BytesPerHost
			remainder = inputBytes % self.BytesPerHostEnum.BytesPerHost

			# divide the inputBytes / BytesPerHost
			hostsNeeded = inputBytes / self.BytesPerHostEnum.BytesPerHost

			# if there is a remainder or no hosts, add one to the hosts needed
			if remainder != 0 or hostsNeeded == 0:
				hostsNeeded += 1

			# if there are more hosts needed than is available, ignore this job
			if int ( hostsNeeded ) > int ( self.numberOfHosts ):
				continue

			# add the job to the queue
			self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ), jobComponents[ 0 ] ], block=False )

	def findHosts( self, numberOfHosts, requiredHosts, counter ):
		"""
		Find the hosts to run the jobs
		"""
		# loop through the hosts, 0 to max
		for j in xrange( numberOfHosts ):
			# if the host has the available reducers
			if self.availableList[ j ] == 2:
				# store the mapper
				mapperHost = j
				# loop through the hosts backwards, max to 0
				for h in xrange( numberOfHosts - 1, 0, -1 ):
					# if the host has the available reducers
					if self.availableList[ h ] == 2:
						# if h is equal to j, found the same host, break the loop
						if h == j:
							print "same host"
							break
						# if h has gone past h, break the loop
						elif j > h:
							print "wrap around"
							break

						# capture the second reducer
						reducerHost = h
						# decrement the required hosts
						requiredHosts -= 2
						# clear both hosts
						self.availableList[ j ] = 0
						self.availableList[ h ] = 0
						# add the hosts to the job stats
						self.jobStats[ counter ].hosts.append( reducerHost )
						self.jobStats[ counter ].hosts.append( mapperHost )
						# found everything needed, so break the loop
						break
					else:
						# if we are at the end of the loop
						if h == 0:
							# reset the j value
							j = 0
				# if the program get here, break the loop
				break
		# return the required hosts
		return requiredHosts

	def runTest( self ):
		"""
		This method runs the tests in the file
		"""
		# if the queue is empty, throw a lookup error
		if self.workQueue.empty() == True:
			raise LookupError( "Running test on an empty queue" )

		# clear the current job list, get the number of hosts, and clears the counter
		currentJob = []
		numberOfHosts = self.numberOfHosts
		counter = 0

		# loop through the queue
		for i in xrange( self.workQueue.qsize() ):
			# get the current job
			currentJob = self.workQueue.get_nowait()
			# get the hosts
			requiredHosts = currentJob[ 0 ]
			# grab the required hosts and store it in a temp variable
			tempHosts = requiredHosts
			# grab the required hosts
			totalRequiredHosts = currentJob[ 0 ]
			# create the job statistics and add it to the list
			self.jobStats.append( JobStatistic() )

			# grab the job and bytes to send
			self.jobStats[ counter ].job = currentJob[ 2 ]
			self.jobStats[ counter ].bytesToSend = currentJob[ 1 ] / totalRequiredHosts
			self.jobStats[ counter ].numberOfHosts = requiredHosts

			# while there are hosts required 
			while requiredHosts > 0:
				# turn off the list updated flag
				self.hasListBeenUpdated = False

				# find the hosts
				tempHosts = self.findHosts( numberOfHosts, requiredHosts, counter )

				# if the hosts are the same, there are no available hosts
				if requiredHosts == tempHosts:
					timeCounter = 0
					# print "WAITING for %i job %s queue %s" % ( requiredHosts, self.jobStats[ counter ], currentJob )
					# wait forever
					while ( timeCounter < 100 ):
						# if the list has been updated, break the loop
						if self.hasListBeenUpdated == True:
							break
						# sleep
						timeCounter += 1
						time.sleep( 0.125 )
				# found hosts, so set the variables equal
				else:
					requiredHosts = tempHosts
				time.sleep( 0.125 )

			# time to find the job process to run the job
			for i in xrange( len( self.availableJobList ) ):
				# if the element is available
				if self.availableJobList[ i ] == True:
					# grab job from the job stats
					self.jobStats[ counter ].reduceJob = i
					# set this element as unavailable
					self.availableJobList[ i ] = False
					# send it to the job process
					self.jobPipeList[ i ].parentConnection.send( self.jobStats[ counter ] )
					break
			# increment the job stats counter
			counter += 1

		# done scheduling the jobs
		while True:
			# set the variable to true
			done = True
			# loop through the available job lists
			for i in self.availableJobList:
				# if the element is false, set done to false, and break the loop
				if i == False:
					done = False
					break
			# if the jobs are done, break the loop
			if done == True:
				print "BROKE!"
				break
			time.sleep( 0.125 )

		# log all of the job stats
		with open('jobLog%s.csv' % self.getTime(), 'w') as f:
			f.write("Job,NumberOfHosts,StartTime,EndTime,Hosts,BytesTransmitted,JobNumber\n");
			for item in self.jobStats:
				f.write("%s\n" % item)
