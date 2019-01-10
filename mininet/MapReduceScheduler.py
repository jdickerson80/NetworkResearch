#!/usr/bin/python

from enum import IntEnum
from MapReduceJob import *
from MapReduceHost import *
from PerProcessPipes import *
from multiprocessing import Pipe
from Queue import Queue
from JobStatistic import *
from FileConstants import *
import json
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
		self.workQueue = Queue()
		# get the length of the hosts list, which is the number of the hosts
		self.numberOfHosts = int ( len( hostList ) - 1 )
		self.hostList = hostList
		self.jobList = []

		numberOfJobProcesses = self.numberOfHosts * 2 
		# create the available job list, which is used to figure out what map reduce job to assign the job
		self.availableJobList = [ True for i in xrange( numberOfJobProcesses ) ]
		self.jobPipeList = [ PerProcessPipes() for i in xrange( numberOfJobProcesses ) ]
		self.hostMapReduceList = []

		# available hosts. two mappers per host
		self.availableMapperList	= [ [ HostStates.Ready, HostStates.Ready ] for i in xrange( self.numberOfHosts ) ]
		self.availableReducerList	= [ [ HostStates.Ready, HostStates.Ready ] for i in xrange( self.numberOfHosts ) ]
		self.handleJobPipeThreadRunning = True
		self.mapperPipes = [ [ PerProcessPipes(), PerProcessPipes() ] for i in xrange( self.numberOfHosts ) ]
		
		# create the pipes
		for i in self.jobPipeList:
			i.parentConnection, i.childConnection = Pipe()

		for i in self.mapperPipes:
			i[ 0 ].parentConnection, i[ 0 ].childConnection = Pipe()
			i[ 1 ].parentConnection, i[ 1 ].childConnection = Pipe()

		# create the hosts' map reduce clients
		for i in xrange( self.numberOfHosts ):
			self.hostMapReduceList.append( HostMapReduce( host = self.hostList[ i ], mapperOnePipe = self.mapperPipes[ i ][ 0 ].childConnection, mapperTwoPipe = self.mapperPipes[ i ][ 1 ].childConnection ) )
		

		# set the list to be updated, so the job can be scheduled
		self.hasListBeenUpdated = True
		self.jobStats = []

		# get the thread to handle the MapReduceJob communications
		self.handleJobPipeThread = threading.Thread( target=self.handleJobPipe )

		# start the thread
		self.handleJobPipeThread.start()

		# create the map reduce jobs. It is half of the number of hosts because each job takes two hosts at a minimum
		for i in xrange( numberOfJobProcesses ):
			job = MapReduceJob( schedulerPipe = self.jobPipeList[ i ].childConnection, hostMapReduceList = self.hostMapReduceList, hostMapperPipes = self.mapperPipes )
			job.start()
			self.jobList.append( job )

		print "JOBS!! %s" % len ( self.jobList )

		# flush the hosts' buffer
		for i in self.mapperPipes:
			i[ 0 ].parentConnection.recv()
			i[ 1 ].parentConnection.recv()

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
		# time.sleep( 0.5 )
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

					# print "before %s" % self.availableMapperList

					# set all the hosts back to available
					for i in jobStatistic.mapHostList:
						self.availableMapperList[ i.hostName ][ i.hostIndex ] = HostStates.Ready

					# set all the hosts back to available
					for i in jobStatistic.reduceHostList:
						self.availableReducerList[ i.hostName ][ i.hostIndex ] = HostStates.Ready

					# set the job back to available
					self.availableJobList[ jobStatistic.reduceJob ] = True

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
			# loop sleep time
			time.sleep( 0.0025 )

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
			if self.workQueue.qsize() >= self._maximumJobsPerTest:
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
			self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ), jobComponents[ 0 ] ], block=False )

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

	def logTestResults( self, outputDirectory ):
		for item in self.jobStats:
			jobDirectory = "%s%s/" % ( outputDirectory, item.job )
			# print jobDirectory
			if not os.path.exists( jobDirectory ):
				os.mkdir( jobDirectory )

			for host in self.hostList:
				hostDirectory = "%s%s/" % ( FileConstants.hostBaseDirectory, host )
				self.copyJobLogs( hostDirectory, jobDirectory, item )

	@staticmethod	
	def findJobDirectory( jobDirectoryList, job ):
		for j in jobDirectoryList:
			if j == job.job:
				return j
		return None

	@staticmethod
	def deleteLinesInFile( file, lastLineToDelete ):
		infile = open( file,'r' ).readlines()
		with open( file,'w' ) as outfile:
			for index,line in enumerate( infile ):
				if index >= lastLineToDelete:
					outfile.write(line)		
			
	@staticmethod
	def logIperfResults( jobDirectory, job ):
		directoryJobs = os.listdir( jobDirectory )
		
		for j in job:
			jobFolder = MapReduceScheduler.findJobDirectory( directoryJobs, j )
			sourceDirectory = os.path.join( jobDirectory, jobFolder ) 
			sourceDirectory += "/"
			fileList = os.listdir( sourceDirectory )
	
			for jobFile in fileList:
				try:
					with open( os.path.join( sourceDirectory, jobFile ) , "r" ) as file:
						file.seek( 0 )
						char = file.read( 1 )
						if not char:
							print "file is empty" #first character is the empty string..
							continue
						else:
							file.seek( 0 ) #first character wasn't empty, return to start of file.

						try:
							jsonObject = json.load( file )
						except ValueError as e:
							# print "IperfResults %s %s" % ( jobFile, e )
							stringError = str( e )
							fileName = file.name

							start = stringError.find( "line " )
							start += 5
							end = stringError.find( "column " )
							end -= 1
 
							seekPosition = int( stringError[ start : end ] )
							file.close()
							MapReduceScheduler.deleteLinesInFile( fileName, seekPosition - 1 )
							print "file %s error %s %s " % ( jobFile, e, seekPosition )
							file = open( fileName, "r" )
							try:
								jsonObject = json.load( file )
							except ValueError as e:
								print "gave up on file %s error %s" % ( jobFile, e )
								continue


						if jsonObject == None:
							print "object none"
							continue

						if MapReduceScheduler.fillSumReceived( j.receiveResults, jsonObject ) == False: 
							print "%s receive HAS ERROR" % j.job

						if MapReduceScheduler.fillSumSent( j.sentResults, jsonObject ) == False:
							print "%s sent HAS ERROR" % j.job

				except IOError as error:
					print "IOERROR %s" % error
		
	@staticmethod
	def copyJobLogs( hostDirectory, jobDirectory, job ):
		for f in os.listdir( hostDirectory ):
			end = 0
			start = f.find( job.job )
			if start == -1:
				continue

			start += 3

			end = start
			while ( f[ end ] != 'P' ):
				end += 1

			jobNumber = f[ start : end ]

			if jobNumber == job.job[ 3 : ]:
				while True:         
					try:
						source = os.path.join( hostDirectory, f )
						destination = os.path.join( jobDirectory, f ) 
						shutil.copy2( source, destination )
						break
					except IOError as error:
						print "remove files %s" % error  


					time.sleep( 0.025 )

	@staticmethod
	def fillSumReceived( results, jsonObject ):
		asdf = jsonObject[ 'start' ][ 'connected' ]
		if not asdf:
			print "sum got none!!"
			return False
		byte = jsonObject[ 'end' ][ 'sum_received' ][ 'bytes' ]
		duration = jsonObject[ 'end' ][ 'sum_received' ][ 'seconds' ]
		retransmit = None
		transmissionRate = jsonObject[ 'end' ][ 'sum_received' ][ 'bits_per_second' ]
		results.append( IperfResults( byte, duration, retransmit, transmissionRate ) )
		return True

	@staticmethod
	def fillSumSent( results, jsonObject ):
		asdf = jsonObject[ 'start' ][ 'connected' ]
		if not asdf:
			print "sent got none!!"
			return False
		byte = jsonObject[ 'end' ][ 'sum_sent' ][ 'bytes' ]
		duration = jsonObject[ 'end' ][ 'sum_sent' ][ 'seconds' ]
		retransmit = jsonObject[ 'end' ][ 'sum_sent' ][ 'retransmits' ]
		transmissionRate = jsonObject[ 'end' ][ 'sum_sent' ][ 'bits_per_second' ]
		results.append( IperfResults( byte, duration, retransmit, transmissionRate ) )
		return True
		
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
		
		if self.workQueue.empty() == True:
			raise LookupError( "Running test on an empty queue" )

		print "Queue size: %i" % self.workQueue.qsize()

		# clear the current job list, get the number of hosts, and clears the counter
		currentJob = []
		counter = 0

		self.clearTestFolder( outputDirectory )

		self.clearHostsBandwidthLogs()

		# loop through the queue
		for i in xrange( self.workQueue.qsize() ):
			# get the current job
			currentJob = self.workQueue.get_nowait()

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

			while True:
				foundJob = False
				# print "Finding Job"
				# time to find the job process to run the job
				for i in xrange( len( self.availableJobList ) ):
					# if the element is available
					if self.availableJobList[ i ] == True:
						foundJob = True
						# grab job from the job stats
						self.jobStats[ counter ].reduceJob = i
						# set this element as unavailable
						self.availableJobList[ i ] = False
						# send it to the job process
						# print "Scheduled %s" % self.jobStats[ counter ]
						self.jobPipeList[ i ].parentConnection.send( self.jobStats[ counter ] )
						break

				if foundJob == True:
					break

				time.sleep( 0.025 )		
			# increment the job stats counter
			counter += 1

		# done scheduling the jobs
		# print "@@@@@@@@@@@@@@@checking for done"
		while True:
			# set the variable to true
			done = True
			# loop through the available job lists
			for i in xrange( len( self.availableJobList ) ):
				# if the element is false, set done to false, and break the loop
				if self.availableJobList[ i ] == False:
					done = False
					break
			# if the jobs are done, break the loop
			if done == True:
				# print "BROKE!"
				break
			time.sleep( 0.125 )

			# log all of the job stats
		with open('%sjobLog%sTEMP.csv' % ( outputDirectory, self.getTime() ), 'w' ) as f:
			f.write( self._jobLogHeader )
			for item in self.jobStats:
				try:
					f.write("%s\n" % item)
				except IOError as error:
					print "writing csv error: %s" % error  
					pass

		self.logTestResults( outputDirectory )
		self.logIperfResults( outputDirectory, self.jobStats )

		# log all of the job stats
		with open('%sjobLog%s.csv' % ( outputDirectory, self.getTime() ), 'w' ) as f:
			f.write(self._jobLogHeader )
			for item in self.jobStats:
				try:
					f.write("%s\n" % item)
				except IOError as error:
					print "writing csv error: %s" % error  
					pass