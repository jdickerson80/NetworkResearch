#!/usr/bin/python

from mininet.node import *
from PerProcessPipes import *
from multiprocessing import *
from enum import IntEnum
import time
import sys



class HostMapReduce( object ):
	"""
	This class starts 4 processes per host. Two iperf3 servers and clients.
	"""
	_speedSafetyFactor = 500
	@staticmethod
	def getLinkSpeed( host ):
		linkSpeed = []

		while len ( linkSpeed ) == 0:
			linkSpeed = []
			speed = host.cmd( "ethtool %s | grep -i speed" % host.intf() )	
			# print "comman speed %s" % speed
			speed = speed.replace( "\n", "" )
			speed = speed.replace( "\t", "" )
			speed = speed.replace( " ", "" )
			speed = speed.replace( "Speed:", "" )
			speed = speed.replace( "Mb/s", "" )
			linkSpeed = [ float(s) for s in speed.split() if s.isdigit() ]
		
		return linkSpeed[ 0 ] 
		# return int( speed )

	@staticmethod
	def runMapperMethod( list, host, ip ):
		# maxTime = int( ( float( list[ 1 ] ) / HostMapReduce.getLinkSpeed( host ) ) ) 
		# if maxTime == 0:
		# 	maxTime = 1

		# maxTime *= HostMapReduce._speedSafetyFactor

		# print maxTime
		# create the command
		command = "iperf3 -c %s -n %s -p %s > /dev/null" % ( list[0], list[1], list[2] )
		# print ip + " " + command
		# start another process and exec the command
		# @todo use the err to make sure the command ran successfully
		pOpenObject = host.pexecNoWait( command )

		timeCounter = 0
		while timeCounter <= 10000:
			returnCode = pOpenObject.poll()
			if returnCode != None:
				return returnCode;	

			timeCounter += 1
			time.sleep( 0.0125 )

		pOpenObject.kill()
		pOpenObject.wait()
		print "killed mapper"

	class MapReduceClassIndex( IntEnum ):
		# just an enum to hold max reducers and mappers
		NumberOfMappers = 2
		NumberOfReducers = 2

	def __init__( self, host, schedularPipe ):
		# init the variables
		self.host 			= host
		self.schedularPipe 	= schedularPipe
		self.mapPipes    	= [ PerProcessPipes() for i in range( self.MapReduceClassIndex.NumberOfMappers ) ]
		self.mapWorkers  	= []
		self.reduceWorkers 	= []
		self.name 			= "%s" % host

		self.availableMappers = 0
		portNumber = 5001
		self.ipAddress = host.IP()

		# init the map pipes
		for i in self.mapPipes:
			i.parentConnection, i.childConnection = Pipe()

		# @note there are no reducer pipes because the iperf server runs forever

		# create the list of mappers and reducers
		for i in xrange( self.MapReduceClassIndex.NumberOfMappers ):
			self.mapWorkers.append( Process(target=self.runMapper, args=(self.mapPipes[ i ].childConnection, self.host, self.ipAddress, ) ) )

		for i in xrange( self.MapReduceClassIndex.NumberOfReducers ):
			pOpen = host.pexecNoWait( "iperf3 -s -p %s > /dev/null" % portNumber )
			# stdOut, stdError = pOpen.communicate()
			# self.reduceWorkers.append( [ pOpen, stdOut, stdError ] )
			self.reduceWorkers.append( pOpen )
			portNumber += 1

		# start the mappers and reducers
		for p in self.mapWorkers:
			p.start()

		# start the process for handling the mappers communication
		self.handler = Process( target=self.handleHostProcesses )
		self.handler.start()

	@staticmethod
	def runMapper( connection, host, ip ):
		# send ready
		connection.send( 1 )
		# infinite loop
		while True:
			# wait for a message
			receiveMessage = connection.recv()
			# send busy command
			connection.send( 0 )
			# run the iperf client
			HostMapReduce.runMapperMethod( receiveMessage, host, ip )
			# send the ready command
			connection.send( 1 )

	def getIP( self ):
		# get the ip
		return self.ipAddress

	def getName( self ):
		return self.name

	def addMapper( self, whatMapper, bytesToTransmit, destinationIPAddress ):
		# get what port to use
		if whatMapper % 2 == 0:
			port = 5001
		else:
			port = 5002

		# send the job to the mappers
		self.mapPipes[ whatMapper ].parentConnection.send( [ destinationIPAddress, bytesToTransmit, port ] )

	def handleHostProcesses( self ):
		# send the available mappers to the schedulers
		self.schedularPipe.send( self.availableMappers )
		# store the mappers in a temp variable to only send data when mappers have changed
		availableMappers = self.availableMappers
		while True:
			# set the count to zero
			count = 0
			# loop through the map pipes
			for i in self.mapPipes:
				# get the poll variable
				poll = i.parentConnection.poll()

				# if there is data to receive
				if poll == True:
					# get the message
					message = i.parentConnection.recv()
					# if this a ready message
					if message == 1:
						# increment mappers
						availableMappers += 1
					else: # if this is a busy message
						# reduce the mappers
						availableMappers -= 1
			# if the mappers has changed this scan
			if self.availableMappers != availableMappers:
				# set the lists equal and send it
				self.availableMappers = availableMappers
				self.schedularPipe.send( self.availableMappers )
				# print "sending %s" % self.availableMappers
			# sleep for the loop	
			time.sleep( 0.02 )

	def terminate( self ):
		# terminate the mappers and reducers
		for p in self.mapWorkers:
			p.terminate()

		for p in self.reduceWorkers:
			p.kill()
			p.wait()

		# join the mappers and reducers
		for p in self.mapWorkers:
			p.join()

		# for p in self.reduceWorkers:
			# p.returncode()

		# terminate host handler and join it	
		self.handler.terminate()
		self.handler.join()
