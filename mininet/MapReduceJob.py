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
	def __init__( self, schedulerPipe, hostMapReduceList, hostPipeList ):
		# init the parent class
		super( MapReduceJob, self ).__init__()
		# grab the scheduler pipe, hosts list, and pipe list
		self.schedulerPipe = schedulerPipe
		self.hostMapReduceList = hostMapReduceList
		self.hostPipeList = hostPipeList

	def removeHostsBandwidthLog( self, hostList ):
		for host in hostList:
			hostName = self.hostMapReduceList[ host ].getName()
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
			hostName = self.hostMapReduceList[ host ].getName()
			originalFile = "%s%s/%s" % ( FileConstants.hostBaseDirectory, hostName, FileConstants.hostBandwidthLogFile )
			newFile = "%s%s/%s%s%s" % ( FileConstants.hostBaseDirectory, hostName, hostName, filePrefix, FileConstants.hostJobLogFile )
			while True:			
				try:
					shutil.copy2( originalFile, newFile )
					break
				except IOError as error:
					continue
					# print "log %s" % error		
				time.sleep( 0.025 )

	def clearHostsPipes( self, hostList ):
		for host in hostList:
			# clear both hosts buffers
			while True:
				# get the poll value
				poll = self.hostPipeList[ host ].parentConnection.poll()

				# if there is data
				if poll == True:
					# get the message
					message = self.hostPipeList[ host ].parentConnection.recv()
					if message == 2:
						# if the message is ready, leave the loop
						break
				else: # if there is no data, leave the loop
					break

	@staticmethod
	def getTime():
		# get the time
		return time.time()

	def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend, outputFile ):
		# get the ip address of both hosts
		hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
		hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
		# divide the data in half
		numberOfBytesToSend = numberOfBytesToSend / 2

		# send the job to all the hosts
		self.hostMapReduceList[ hostOne ].addMapper( 0, numberOfBytesToSend, hostTwoIP, outputFile )
		self.hostMapReduceList[ hostTwo ].addMapper( 0, numberOfBytesToSend, hostOneIP, outputFile )

		self.hostMapReduceList[ hostOne ].addMapper( 1, numberOfBytesToSend, hostTwoIP, outputFile )
		self.hostMapReduceList[ hostTwo ].addMapper( 1, numberOfBytesToSend, hostOneIP, outputFile )

	def run( self ):
		while True:
			# clear the job pipes
			jobPipes = []
			# get the job from the scheduler
			receiveMessage = self.schedulerPipe.recv()

			# "cast" the message to a job statistic
			jobStatistic = receiveMessage

			self.clearHostsPipes( jobStatistic.hosts )
			self.removeHostsBandwidthLog( jobStatistic.hosts )

			# log the start time
			jobStatistic.startTime = self.getTime()

			# loop through the hosts by two
			for pair in xrange( 0, len( jobStatistic.hosts ), 2 ):
				# add the hosts to the job pipes list
				jobPipes.append( self.hostPipeList[ jobStatistic.hosts[ pair ] ] )
				jobPipes.append( self.hostPipeList[ jobStatistic.hosts[ pair + 1 ]  ] )
				# start the hosts' mappers
				self.setIperfPair( jobStatistic.hosts[ pair ], jobStatistic.hosts[ pair + 1 ], jobStatistic.bytesToSend, jobStatistic.job )

			# wait for the mappers to be completed
			while True:
				# clear the count variable
				count = 0
				# loop through the job pipes
				for i in jobPipes:
					# get the poll value
					poll = i.parentConnection.poll()
					# if there is data
					if poll == True:
						# get the message
						message = i.parentConnection.recv()
						# if the message said both mappers are complete
						if message == 2:
							# remove the pipe from the pipe list
							del jobPipes[ count ]
					# increment the counter
					count += 1

				# the pipes have been polled, so if the job pipes is empty
				if len( jobPipes ) == 0:
					# job is complete, so break the loop
					break
				
				time.sleep( 0.020 )

			# get the end end time
			jobStatistic.endTime = self.getTime()

			self.logHostsBandwidth( jobStatistic.hosts, jobStatistic.job )

			# send the stat to the scheduler
			self.schedulerPipe.send( jobStatistic )
