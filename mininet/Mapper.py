#!/usr/bin/python

from FileConstants import *
from mininet.node import Host
import multiprocessing 
import time
from HostStates import *

class Mapper( multiprocessing.Process ):
	def __init__( self, connection, host, ipAddress ):
		multiprocessing.Process.__init__( self )
		self.connection = connection
		self.host = host
		self.ipAddress = ipAddress

	def run( self ):
		# send ready
		self.connection.send( HostStates.Ready )

		# infinite loop
		while True:
			# wait for a message
			receiveMessage = self.connection.recv()

			if receiveMessage == HostStates.Terminate:
				self.connection.send( HostStates.Ready )
				# print "got terminate in mapper"
				continue

			# send busy command
			self.connection.send( HostStates.Busy )
			# run the iperf client
			returnValue = Mapper.handleIperf( self.connection, receiveMessage, self.host, self.ipAddress )

			if returnValue == 0:	
				# send the ready command
				self.connection.send( HostStates.Ready )
			else:
				# print "ERROR!!! %s mapper returned %s" % ( self.host, returnValue )
				self.connection.send( HostStates.Error )

	@staticmethod
	def handleIperf( connection, list, host, ip ):
		# maxTime = int( ( float( list[ 1 ] ) / HostMapReduce.getLinkSpeed( host ) ) ) 
		# if maxTime == 0:
		# 	maxTime = 1

		# maxTime *= HostMapReduce._speedSafetyFactor

		# print maxTime
		# create the command
		# print list
		logFile = "%s%s/%s%sPort%s.log" % ( FileConstants.hostBaseDirectory, host, host, list[ 2 ], list[ 3 ] )
		command = "iperf3 -c %s -n %s -p %s --logfile %s --get-server-output -J 2>&1 > /dev/null" % ( list[ 0 ], list[ 1 ], list[ 3 ], logFile )
		# print "%s %s" % ( host, command )
		pOpenObject = host.pexecNoWait( command )

		# print out, err
		timeCounter = 0
		while timeCounter <= 10000:
			returnCode = pOpenObject.poll()

			if returnCode != None:
				out, err = pOpenObject.communicate()
				return returnCode;	

			# get the poll variable
			poll = connection.poll()

			# if there is data to receive
			if poll == True:
				# get the message
				message = connection.recv()

				if message == HostStates.Terminate:
					# print "mapper got a terminate message"
					break

			timeCounter += 1
			time.sleep( 0.0125 )


		pOpenObject.kill()
		pOpenObject.wait()
		# print "killed mapper %s" % host
		return HostStates.Error

	