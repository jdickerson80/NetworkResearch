#!/usr/bin/python

from mininet.node import *
from JobStatistic import *
from MapReduceHost import *
# from multiprocessing import Process
import threading
from FileConstants import *
import shutil
import os
import time
import sys
from random import shuffle

class MapReduceJob( threading.Thread ):
	"""
	This class runs a map reduce job and logs its stats
	"""
	def __init__( self, jobQueue, jobStatistic, hostMapReduceList, hostMapperPipes ):
		# init the parent class
		super( MapReduceJob, self ).__init__()
		# grab the scheduler pipe, hosts list, and pipe list
		self.jobStatistic 		   = jobStatistic
		self.hostMapReduceList = hostMapReduceList
		self.hostMapperPipes = hostMapperPipes
		self.jobQueue = jobQueue

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
			mapperPipes.append( [ self.hostMapperPipes[ host.hostName ][ host.hostIndex ], host.hostName, host.hostIndex ] )

			mapCounter += 1

		return mapperPipes

	def terminateMappers( self, mapperList ):
		for host in mapperList:
			self.hostMapReduceList[ host.hostName ].terminateMapper( host.hostIndex )

	@staticmethod
	def terminateMappersPipes( mapperPipes ):
		for host in mapperPipes:
			host[ 0 ].parentConnection.send( HostStates.Terminate )

	def startReducers( self, reducerList ):
		# get the ip address of both hosts
		for host in reducerList:
			self.hostMapReduceList[ host.hostName ].addReducer( host.hostIndex )

	def terminateReducers( self, reducerList ):	
		for host in reducerList:
			self.hostMapReduceList[ host.hostName ].terminateReducer( host.hostIndex )

	def killReducers( self, reducerList ):
		for host in reducerList:
			self.hostMapReduceList[ host.hostName ].killReducer( host.hostIndex )

	_retryThreshold = 2

	def run( self ):
		hasNoError = True
		notComplete = True
		retryCount = 0
		while notComplete:
			mapperPipes = []		
			sameCounter = 0
			# print "got %s" % jobStatistic

			hasNoError = True
			self.clearHostsPipes( self.jobStatistic.mapHostList )
			self.clearHostsPipes( self.jobStatistic.reduceHostList )

			# self.removeHostsBandwidthLog( jobStatistic.mapHostList )
			# self.removeHostsBandwidthLog( jobStatistic.reduceHostList )

			self.startReducers( self.jobStatistic.reduceHostList )
			self.jobStatistic.startTime = self.getTime()
			mapperPipes = self.startMappers( self.jobStatistic.mapHostList, self.jobStatistic.reduceHostList, self.jobStatistic.bytesToSend, self.jobStatistic.job )
			mapperPipesLength = len( mapperPipes )
					
			# log the start time
			# wait for the mappers to be completed

			while True:
				# clear the count variable
				count = 0
				# loop through the job pipes
				for i in mapperPipes:
					# get the poll value
					poll = i[0].parentConnection.poll()
					# if there is data
					if poll == True:
						# get the message
						message = i[0].parentConnection.recv()
						# print "%s got %s" % ( jobStatistic.job, message )
						# if the message said both mappers are complete
						if message == HostStates.Ready:
							# remove the pipe from the pipe list
							# print "deling %s" % count
							del mapperPipes[ count ]
						elif message == HostStates.Error:
							# print "mapper %s failed" % mapperPipes[ count ][ 1 ]
							self.jobStatistic.failedHostList.append( "%s:%s"% ( mapperPipes[ count ][ 1 ], mapperPipes[ count ][ 2 ] ) )
							self.jobStatistic.errorCounts += 1
							del mapperPipes[ count ]

							# self.terminateMappersPipes( mapperPipes )
							# self.terminateReducers( jobStatistic.reduceHostList )
							# shuffle( jobStatistic.mapHostList )
							# shuffle( jobStatistic.reduceHostList )
							hasNoError = False
							print "!!!!error job %s" % self.jobStatistic.job
							# time.sleep( 2 )
							# break
					# increment the counter
					count += 1

				if mapperPipesLength == len( mapperPipes ):
					sameCounter += 1
				else:
					mapperPipesLength = len( mapperPipes )

				# the pipes have been polled, so if the job pipes is empty
				if len( mapperPipes ) == 0:# or hasNoError == False:
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
				self.terminateMappers( self.jobStatistic.mapHostList )

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

			self.terminateReducers( self.jobStatistic.reduceHostList )
			# self.terminateMappers( jobStatistic.mapHostList )

			time.sleep( 1 )

			notComplete = False
			self.jobQueue.put( self.jobStatistic )
