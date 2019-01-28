#!/usr/bin/python

from mininet.node import *
from JobStatistic import *
import threading
from FileConstants import *
import shutil
import os
import time
import sys

class MapReduceJob( threading.Thread ):
	"""
	This class runs a map reduce job and logs its stats
	"""
	def __init__( self, jobQueue, jobStatistic, reducerManager, hostList ):
		# init the parent class
		super( MapReduceJob, self ).__init__()
		# grab the scheduler pipe, hosts list, and pipe list
		self.jobStatistic 		   = jobStatistic
		self.reducerManager = reducerManager
		self.hostList = hostList
		self.jobQueue = jobQueue

	def removeHostsBandwidthLog( self, hostList ):	
		for host in hostList:
			hostName = "%s" % self.hostList[ host.hostName ]
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
			hostName = "%s" % self.hostList[ host.hostName ]
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

	def startMappers( self, mapperList, reducerList, numberOfBytesToSend, job ):
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
			node = self.hostList[ host.hostName ]
			logFile = "%s%s/%s%sPort%s.log" % ( FileConstants.hostBaseDirectory, node, node, job, serverPort )
			destIP = self.hostList[ reducerList[ mapCounter ].hostName ].IP()
			command = "iperf3 -c %s -n %s -p %s --logfile %s --get-server-output -J 2>&1 > /dev/null" % ( destIP, numberOfBytesToSend, serverPort, logFile )
			# print "%s %s" % ( node, command )
			mapperPipes.append( node.pexecNoWait( command ) )

			mapCounter += 1

		return mapperPipes

	def terminateMappers( self, mapperList ):
		for host in mapperList:
			host.kill()
			host.wait()

	@staticmethod
	def terminateMappers( mappers ):
		for host in mappers:
			host[ 0 ].parentConnection.send( HostStates.Terminate )

	def killReducers( self, reducerList ):
		for host in reducerList:
			self.reducerManager.killAndRestartReducer( host.hostName, host.hostIndex )

	_retryThreshold = 2

	def run( self ):
		super( MapReduceJob, self ).run()
		hasNoError = True
		notComplete = True
		retryCount = 0
		while notComplete:
			mapperPipes = []		
			sameCounter = 0
			# print "got %s" % jobStatistic

			hasNoError = True
			# self.clearHostsPipes( self.jobStatistic.mapHostList )
			# self.clearHostsPipes( self.jobStatistic.reduceHostList )

			# self.removeHostsBandwidthLog( jobStatistic.mapHostList )
			# self.removeHostsBandwidthLog( jobStatistic.reduceHostList )

			# self.startReducers( self.jobStatistic.reduceHostList )
			self.jobStatistic.startTime = self.getTime()
			mappers = self.startMappers( self.jobStatistic.mapHostList, self.jobStatistic.reduceHostList, self.jobStatistic.bytesToSend, self.jobStatistic.job )
			mappersLength = len( mappers )
					
			# log the start time
			# wait for the mappers to be completed

			while True:
				# clear the count variable
				count = 0
				# loop through the job pipes
				for i in mappers:
					# get the poll value
					poll = i.poll()
					# if there is data
					if poll != None:
						del mappers[ count ]
						if poll != 0:
							hasNoError = False		
						
					count += 1

				if mappersLength == len( mappers ):
					sameCounter += 1
				else:
					mappersLength = len( mappers )

				# the pipes have been polled, so if the job pipes is empty
				if len( mappers ) == 0:# or hasNoError == False:
					# job is complete, so break the loop
					break
				
				if sameCounter >= 1000000:
					print "!!!!!!!SAME ERROR!!!!!!"
					hasNoError = False
					break
				# if printCounter % 10000 == 0:
					# print "%s waiting on %s pipes" % ( jobStatistic.job, len( mapperPipes ) )
				time.sleep( 0.0020 )

			if hasNoError == False:
				retryCount += 1
				self.terminateMappers( mappers )

				if retryCount == self._retryThreshold - 1 and self._retryThreshold > 1:
					self.killReducers( self.jobStatistic.reduceHostList )

				time.sleep( 2 )
				
				if retryCount >= self._retryThreshold:
					self.jobStatistic.endTime = 0
					self.jobStatistic.startTime = 0		

					# send the stat to the scheduler
					notComplete = False
					self.jobQueue.put( self.jobStatistic )
					print "%s MAX THRESHOLD MET!!!" % self.jobStatistic.job
					return

				continue

			# get the end end time
			self.jobStatistic.endTime = self.getTime()

			# self.logHostsBandwidth( jobStatistic.mapHostList, jobStatistic.job )
			# self.logHostsBandwidth( jobStatistic.reduceHostList, jobStatistic.job )

			# self.terminateReducers( self.jobStatistic.reduceHostList )
			# self.terminateMappers( jobStatistic.mapHostList )

			time.sleep( 1 )

			notComplete = False
			self.jobQueue.put( self.jobStatistic )
