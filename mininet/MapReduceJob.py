#!/usr/bin/python

from mininet.node import *
from JobStatistic import *
from MapReduceHost import *
from multiprocessing import Process
from FileConstants import *
import shutil
import os
import time
import sys

class MapReduceJob( Process ):
	"""
	This class runs a map reduce job and logs its stats
	"""
	def __init__( self, schedulerPipe, hostMapReduceList, hostMapperPipes ):
		# init the parent class
		super( MapReduceJob, self ).__init__()
		# grab the scheduler pipe, hosts list, and pipe list
		self.schedulerPipe = schedulerPipe
		self.hostMapReduceList = hostMapReduceList
		self.hostMapperPipes = hostMapperPipes

	def removeHostsBandwidthLog( self, hostList ):	
		for host in hostList:
			hostName = self.hostMapReduceList[ host.hostName ].getName()
			command = "%s%s/%s" % ( FileConstants.hostBaseDirectory, hostName, FileConstants.hostBandwidthLogFile )
			while True:			
				try:
					os.remove( command )
					break
				except os.error as error:
					continue
				time.sleep( 0.025 )

	def logHostsBandwidth( self, hostList, filePrefix ):
		for host in hostList:
			hostName = self.hostMapReduceList[ host.hostName ].getName()
			originalFile = "%s%s/%s" % ( FileConstants.hostBaseDirectory, hostName, FileConstants.hostBandwidthLogFile )
			newFile = "%s%s/%s%s%s" % ( FileConstants.hostBaseDirectory, hostName, hostName, filePrefix, FileConstants.hostJobLogFile )
			while True:			
				try:
					shutil.copy2( originalFile, newFile )
					break
				except IOError as error:
					continue
					print "log io %s" % error		
				except os.error as error:
					continue
					print "log os %s" % error		
				time.sleep( 0.025 )

	@staticmethod
	def getTime():
		# get the time
		return time.time()

	def clearHostsPipes( self, mapperList ):
		for host in mapperList:
			# clear both hosts buffers
			while True:
				# get the poll value
				poll = self.hostMapperPipes[ host.hostName ][ host.hostIndex ].parentConnection.poll()

				# if there is data
				if poll == True:
					# get the message
					message = self.hostMapperPipes[ host.hostName ][ host.hostIndex ].parentConnection.recv()
					if message == HostStates.Ready:
						# if the message is ready, leave the loop
						break
				else: # if there is no data, leave the loop
					break

	def startMappers( self, mapperList, reducerList, numberOfBytesToSend, outputFile ):
		# get the ip address of both hosts
		numberOfBytesToSend = numberOfBytesToSend / len( mapperList )
		mapCounter = 0
		mapperPipes = []
	
		for host in mapperList:
			# get what port to use
			if reducerList[ mapCounter ].hostIndex % 2 == 0:
				serverPort = 5001
			else:
				serverPort = 5002

			destIP = self.hostMapReduceList[ reducerList[ mapCounter ].hostName ].getIP()
			self.hostMapReduceList[ host.hostName ].addMapper( host.hostIndex, numberOfBytesToSend, destIP, outputFile, serverPort )
			mapperPipes.append( self.hostMapperPipes[ host.hostName ][ host.hostIndex ] )

			mapCounter += 1

		return mapperPipes

	def terminateMappers( self, mapperList ):
		for host in mapperList:
			self.hostMapReduceList[ host.hostName ].terminateMapper( host.hostIndex )

	def startReducers( self, reducerList ):
		# get the ip address of both hosts
		for host in reducerList:
			self.hostMapReduceList[ host.hostName ].addReducer( host.hostIndex )

	def terminateReducers( self, reducerList ):	
		for host in reducerList:
			self.hostMapReduceList[ host.hostName ].terminateReducer( host.hostIndex )

	_retryThreshold = 5

	def run( self ):
		hasNoError = True
		while True:
			mapperPipes = []

			if hasNoError == True:
				retryCount = 0
				# get the job from the scheduler
				receiveMessage = self.schedulerPipe.recv()

				# "cast" the message to a job statistic
				jobStatistic = receiveMessage

				# print jobStatistic

			hasNoError = True
			self.clearHostsPipes( jobStatistic.mapHostList )
			self.clearHostsPipes( jobStatistic.reduceHostList )

			self.removeHostsBandwidthLog( jobStatistic.mapHostList )
			# self.removeHostsBandwidthLog( jobStatistic.reduceHostList )

			self.startReducers( jobStatistic.reduceHostList )
			jobStatistic.startTime = self.getTime()
			mapperPipes = self.startMappers( jobStatistic.mapHostList, jobStatistic.reduceHostList, jobStatistic.bytesToSend, jobStatistic.job )
					
			# log the start time
			# wait for the mappers to be completed

			while True:
				# clear the count variable
				count = 0
				# loop through the job pipes
				for i in mapperPipes:
					# get the poll value
					poll = i.parentConnection.poll()
					# if there is data
					if poll == True:
						# get the message
						message = i.parentConnection.recv()
						# print "%s got %s" % ( jobStatistic.job, message )
						# if the message said both mappers are complete
						if message == HostStates.Ready:
							# remove the pipe from the pipe list
							# print "deling %s" % count
							del mapperPipes[ count ]
						elif message == HostStates.Error:
							self.terminateMappers( jobStatistic.mapHostList )
							self.terminateReducers( jobStatistic.reduceHostList )
							hasNoError = False
							time.sleep( 0.20 )
							print "!!!!error job %s" % jobStatistic.job
							break
					# increment the counter
					count += 1

				# the pipes have been polled, so if the job pipes is empty
				if len( mapperPipes ) == 0 or hasNoError == False:
					# job is complete, so break the loop
					break
				
				time.sleep( 0.0020 )

			if hasNoError == False:
				retryCount += 1
				if retryCount > self._retryThreshold:
					hasNoError = True
					jobStatistic.endTime = 0
					jobStatistic.startTime = 0		
					self.terminateReducers( jobStatistic.reduceHostList )

					# send the stat to the scheduler
					self.schedulerPipe.send( jobStatistic )
					print "%s MAX THRESHOLD MET!!!" % jobStatistic.job
				continue

			# get the end end time
			jobStatistic.endTime = self.getTime()

			self.logHostsBandwidth( jobStatistic.mapHostList, jobStatistic.job )
			# self.logHostsBandwidth( jobStatistic.reduceHostList, jobStatistic.job )

			self.terminateReducers( jobStatistic.reduceHostList )

			# send the stat to the scheduler
			self.schedulerPipe.send( jobStatistic )
