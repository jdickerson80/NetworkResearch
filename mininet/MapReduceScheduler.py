#!/usr/bin/python

from enum import IntEnum
from MapReduceJob import *
from MapReduceHost import *
from PerProcessPipes import *
from multiprocessing import Pipe, Queue
# from Queue import Queue
from JobStatistic import *
from FileConstants import *
from JSONParser import *
import sys
import threading
import time
import time

class MapReduceScheduler( object ):
	""" 
	This class handles the scheduling of the map reduce jobs. It figures out the hosts to 
	run the job, and sends the info to the MapReduceJob to handle the running of the job.
	"""
	_maximumJobsPerTest = 10000
	_jobLogHeader = "Job#,NumberOfHosts,StartTime,EndTime,MapperHosts,ReducerHosts,ErrorCounts,ErroredHost,BytesTransmitted,ReceiveResults(bytes duration retransmit transmissionRate),SentResults(bytes duration retransmit transmissionRate)\n"
	# Bytes per host to divide up the job
	class BytesPerHostEnum( IntEnum ):
		BytesPerHost = 64000000

	# constructor
	def __init__( self, hostList ):
		# create the queue
		self.workQueue = []
		self.finishJobQueue = Queue()
		# get the length of the hosts list, which is the number of the hosts
		self.numberOfHosts = int ( len( hostList ) - 1 )
		self.hostList = hostList

		numberOfJobProcesses = self.numberOfHosts * 2 
		# create the available job list, which is used to figure out what map reduce job to assign the job
		self.jobPipeList = [ PerProcessPipes() for i in xrange( numberOfJobProcesses ) ]
		self.hostMapReduceList = []

		# available hosts. two mappers per host
		self.availableMapperList	= [ [ HostStates.Ready, HostStates.Ready ] for i in xrange( self.numberOfHosts ) ]
		self.availableReducerList	= [ [ HostStates.Ready, HostStates.Ready ] for i in xrange( self.numberOfHosts ) ]
		self.handleJobQueueThreadRunning = True
		self.mapperPipes = [ [ PerProcessPipes(), PerProcessPipes() ] for i in xrange( self.numberOfHosts ) ]
		
		for i in self.mapperPipes:
			i[ 0 ].parentConnection, i[ 0 ].childConnection = Pipe()
			i[ 1 ].parentConnection, i[ 1 ].childConnection = Pipe()

		# create the hosts' map reduce clients
		for i in xrange( self.numberOfHosts ):
			self.hostMapReduceList.append( HostMapReduce( host = self.hostList[ i ], mapperOnePipe = self.mapperPipes[ i ][ 0 ].childConnection, mapperTwoPipe = self.mapperPipes[ i ][ 1 ].childConnection ) )
		

		# set the list to be updated, so the job can be scheduled
		self.hasListBeenUpdated = True
		self.handleJobThreadsThreadRunning = True
		self.jobStats = []
		self.jobThreads = []

		# get the thread to handle the MapReduceJob communications
		self.handleJobQueueThread = threading.Thread( target=self.handleJobQueue )

		self.handleJobThread = threading.Thread( target=self.handleJobThreads )

		# start the thread
		self.handleJobQueueThread.start()
		self.handleJobThread.start()

		# flush the hosts' buffer
		for i in self.mapperPipes:
			i[ 0 ].parentConnection.recv()
			i[ 1 ].parentConnection.recv()

	def terminate( self ):
		"""
		Shut down all of threads and processes	
		"""
		# set the pipe thread running to false to shut it down
		self.handleJobQueueThreadRunning = False

		# turn off all of the hosts' map reduce list
		for i in self.hostMapReduceList:
			i.terminate()

		# join the pipe thread
		self.handleJobQueueThread.join()

	def handleJobThreads( self ):
		while self.handleJobQueueThreadRunning == True or len( self.jobThreads ) > 0:
			counter = 0
			for thread in self.jobThreads:
				thread.join
				del( self.jobThreads[ counter ] )
				counter += 1

			time.sleep( 0.5 )

		print "JOB THREADS EXITED"
			
	def handleJobQueue( self ):
		"""
		Get the stats back from the MapReduce job and add the appropriate data to the available lists
		"""
		# time.sleep( 0.5 )
		# while the thread should run
		while self.handleJobQueueThreadRunning == True or self.finishJobQueue.qsize() > 0:
			# loop through the job pipes
			try:
				jobStatistic = self.finishJobQueue.get( .1 )
			except Empty as e:
				print "Queue!!!!" % e
				pass
			# print "before %s" % self.availableMapperList

			# set all the hosts back to available
			for i in jobStatistic.mapHostList:
				self.availableMapperList[ i.hostName ][ i.hostIndex ] = HostStates.Ready

			# set all the hosts back to available
			for i in jobStatistic.reduceHostList:
				self.availableReducerList[ i.hostName ][ i.hostIndex ] = HostStates.Ready

			# print "Mappers are: %s" % self.availableMapperList
			# print "Reducers are: %s" % self.availableReducerList

			# find the job in the job list
			for i in xrange( len( self.jobStats ) ):
				if self.jobStats[ i ].job == jobStatistic.job:
					# set the job equal to the appropriate list position
					self.jobStats[ i ] = jobStatistic
					print "completed %s" % ( self.jobStats[ i ].job )
					# print "after %s" %self.availableMapperList
					break

			# set the list to updated		
			self.hasListBeenUpdated = True

		print "JOB QUEUE EXITED"

	@staticmethod
	def getTime():
		"""
		Get the time in hours, minutes, and seconds
		"""
		return time.strftime( "%H:%M:%S", time.localtime() )


	@staticmethod
	def getTimeTics():
		# get the time
		return time.time()

	@staticmethod
	def __preprocessJobList( workFile ):
		"""
		Remove the \n, job , and all spaces from the work file. 
		"""
		for job in workFile:
			job = job.replace( "\n", "" )
			job = job.replace( "job ", "" )
			job = job.replace( " ", "" )

	@staticmethod
	def __fixOutputFilePermissions( outputDirectory ):
		os.system( "chmod 777 -R %s" % outputDirectory )
		
		for f in os.listdir( outputDirectory ):
			# if re.search( "*.csv", f ) or re.search( "*.dat", f ):
			if re.search( ".csv", f ):
				os.system( "chmod 666 %s/%s" % ( outputDirectory, f ) )

	def __processJobs( self, workFile ):
		"""
		Compute the proper values from the work file
		"""
		inputBytes = 0
		hostNeeded = 0

		# for each job in the file
		for job in workFile:
			if len ( self.workQueue )  >= self._maximumJobsPerTest:
				print "MAX JOBS REACHED!!!!"
				break

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
			self.workQueue.append( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ), jobComponents[ 0 ] ] )

	def findMapperHosts( self, requiredHosts, counter ):
		"""
		Find the hosts to run the jobs
		"""
		if requiredHosts == 0:
			return 0

		for j in xrange( self.numberOfHosts ):
			for h in xrange( HostMapReduce.MapReduceClassIndex.NumberOfMappers ):
				if self.availableMapperList[ j ][ h ] == HostStates.Ready:
					requiredHosts -= 1
					self.availableMapperList[ j ][ h ] = HostStates.Busy
					self.jobStats[ counter ].mapHostList.append( JobHost( j, h ) )
					if requiredHosts == 0:
						return requiredHosts

		# return the required hosts
		return requiredHosts

	def findReducerHosts( self, requiredHosts, jobMappers, counter ):
		"""
		Find the hosts to run the jobs
		"""
		if requiredHosts == 0:
			return 0
		
		mapCounter = 0
		for j in xrange( self.numberOfHosts - 1, 0, -1  ):
			for h in xrange( HostMapReduce.MapReduceClassIndex.NumberOfReducers ):
				if self.availableReducerList[ j ][ h ] == HostStates.Ready:
					if mapCounter < len ( jobMappers ):
						if jobMappers[ mapCounter ].hostName == j:
							# print "SAME HOST!!!!"
							continue

					requiredHosts -= 1
					self.availableReducerList[ j ][ h ] = HostStates.Busy
					self.jobStats[ counter ].reduceHostList.append( JobHost( j, h ) )
					if requiredHosts == 0:
						return requiredHosts
			mapCounter += 1
		# return the required hosts
		return requiredHosts
		
	def clearHostsBandwidthLogs( self ):
		for host in self.hostList:
			hostDirectory = "%s%s/" % ( FileConstants.hostBaseDirectory, host )
			# print hostDirectory
			FileConstants.removeFiles( hostDirectory, ".csv" )
			FileConstants.removeFiles( hostDirectory, ".log" )	
	
	@staticmethod
	def clearTestFolder( outputDirectory ):
		try:
			shutil.rmtree( outputDirectory )
		except OSError as error:
			print "RMTree folders got %s error" % error

		try:
			os.mkdir( outputDirectory )
		except OSError as error:
			print "mkdir folders got %s error" % error

	def runTest( self, outputDirectory, workFile ):
		"""
		This method runs the tests in the file
		"""
		try:
			with open( workFile ) as f:
				data = f.readlines()
		except IOError as error:
			raise LookupError( "Caching work file failed." )

		# preprocess the job list
		self.__preprocessJobList( data )
		# add the jobs to the queue

		self.__processJobs( data )
		
		if len ( self.workQueue ) == True:
			raise LookupError( "Running test on an empty queue" )

		print "Queue size: %i" % len ( self.workQueue )

		# clear the current job list, get the number of hosts, and clears the counter
		currentJob = []
		counter = 0

		self.clearTestFolder( outputDirectory )

		self.clearHostsBandwidthLogs()

		startTime = self.getTimeTics()

		# loop through the queue
		for currentJob in self.workQueue:
			# get the hosts
			requiredMapperHosts = currentJob[ 0 ]
			requiredReducerHosts = currentJob[ 0 ]

			# grab the required hosts and store it in a temp variable
			tempMappers = requiredMapperHosts
			tempReducers = requiredReducerHosts

			# grab the required hosts
			totalRequiredHosts = currentJob[ 0 ]

			# create the job statistics and add it to the list
			self.jobStats.append( JobStatistic() )

			# grab the job and bytes to send
			self.jobStats[ counter ].job = currentJob[ 2 ]
			self.jobStats[ counter ].bytesToSend = currentJob[ 1 ] / totalRequiredHosts
			self.jobStats[ counter ].numberOfHosts = totalRequiredHosts

			# while there are hosts required 
			while requiredMapperHosts > 0 or requiredReducerHosts > 0:
				# turn off the list updated flag
				self.hasListBeenUpdated = False

				# print "Scheduling %s \nmapper: %s\n\nreducers: %s" % ( self.jobStats[ counter ], self.availableMapperList, self.availableReducerList )
				# find the hosts
				tempMappers = self.findMapperHosts( requiredMapperHosts, counter )
				tempReducers = self.findReducerHosts( requiredReducerHosts, self.jobStats[ counter ].mapHostList, counter )

				# if the hosts are the same, there are no available hosts
				if requiredMapperHosts == tempMappers or requiredReducerHosts == tempReducers:
					timeCounter = 0
					if requiredMapperHosts == tempMappers:
						waitingOn = "Mapper"
						lister =  self.availableMapperList
					elif requiredReducerHosts == tempReducers:
						waitingOn = "Reducer"
						lister =  self.availableReducerList


					print "WAITING on %s" % ( waitingOn )#, lister )# for %i job %s queue %s" % ( requiredMapperHosts, self.jobStats[ counter ], currentJob )
					# wait forever
					while ( timeCounter < 100 ):
						# print timeCounter
					# while ( True ):
						# if the list has been updated, break the loop
						if self.hasListBeenUpdated == True:
							# print "broke"
							break
						# sleep
						timeCounter += 1
						time.sleep( 0.125 )


				# if requiredMapperHosts != tempMappers:
				requiredMapperHosts = tempMappers

				# if requiredReducersHosts != tempReducers:
				requiredReducerHosts = tempReducers

				time.sleep( 0.125 )

			job = MapReduceJob( self.finishJobQueue, self.jobStats[ counter ], self.hostMapReduceList, self.mapperPipes )
			job.start()
			self.jobThreads.append( job ) 
			
			# increment the job stats counter
			counter += 1

		self.handleJobQueueThreadRunning = False
		self.handleJobQueueThread.join()
		
		self.handleJobThreadsThreadRunning = False
		self.handleJobThread.join()
		# ADD JOINING OF JOB THREAD WATCHER TO SIGNAL ALL DONE
		endTime = self.getTimeTics()
		print "DONE!!!!!!"
		time.sleep( 4 )

		JSONParser.storeJobStatInFile( outputDirectory, self.jobStats )
		self.jobStats = JSONParser.getJobStatFromFile( outputDirectory )

			# log all of the job stats
		with open('%sjobLog%sTEMP.csv' % ( outputDirectory, self.getTime() ), 'w' ) as f:
			f.write( "%s, %s\n" % ( endTime, startTime ) )
			f.write( self._jobLogHeader )
			for item in self.jobStats:
				try:
					f.write("%s\n" % item)
				except IOError as error:
					print "writing csv error: %s" % error  
					pass

		JSONParser.logTestResults( self.jobStats, outputDirectory, self.hostList )
		JSONParser.logIperfResults( outputDirectory, self.jobStats )

		# log all of the job stats
		with open('%sjobLog%s.csv' % ( outputDirectory, self.getTime() ), 'w' ) as f:
			f.write( "%s, %s\n" % ( endTime, startTime ) )
			f.write(self._jobLogHeader )
			for item in self.jobStats:
				try:
					f.write("%s\n" % item)
				except IOError as error:
					print "writing csv error: %s" % error  
					pass

		MapReduceScheduler.__fixOutputFilePermissions( outputDirectory )
		