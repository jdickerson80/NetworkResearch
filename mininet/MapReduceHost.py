#!/usr/bin/python

from mininet.node import Host
from PerProcessPipes import *
from HostStates import *
from multiprocessing import *
from Mapper import *
from Reducer import *
import time
import sys
import threading

class AvailabilityStatus( object ):
	def __init__( self, onesStatus, twosStatus ):
		self.onesStatus = onesStatus
		self.twosStatus = twosStatus

	def __str__( self ):
		return "One: %s Two: %s" % ( self.onesStatus, self.twosStatus )
	
class HostMapReduce( object ):
	"""
	This class starts 4 processes per host. Two iperf3 servers and clients.
	"""
	class MapReduceClassIndex( IntEnum ):
		# just an enum to hold max reducers and mappers
		MapperOne = 0
		MapperTwo = 1

		ReducerOne = 0
		ReducerTwo = 1

		NumberOfMappers = 2
		NumberOfReducers = 2

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

	def __init__( self, host, mapperOnePipe, mapperTwoPipe ):
		# init the variables
		self.host 				= host
		self.availableMappers	= [ AvailabilityStatus( HostStates.Busy, HostStates.Busy )  for i in xrange( self.MapReduceClassIndex.NumberOfMappers ) ]
		self.availableReducers	= [ AvailabilityStatus( HostStates.Ready, HostStates.Ready ) for i in xrange( self.MapReduceClassIndex.NumberOfReducers ) ]
		self.mapPipes    		= [ PerProcessPipes() for i in range( self.MapReduceClassIndex.NumberOfMappers ) ]
		self.reducePipe   		= PerProcessPipes()
		self.mapWorkers  		= []
		self.reduceWorkers 		= []
		self.name 				= "%s" % host
		self.availableList 		= []
		self.ipAddress 			= host.IP()
		self.mapperOnePipe 		= mapperOnePipe
		self.mapperTwoPipe 		= mapperTwoPipe
		self.killQueue			= []


		# for reducer in self.availableReducers:
			# print reducer
		# print "%s %s %s" % ( self.name, self.mapperOnePipe, self.mapperTwoPipe )

		portNumber = 5001

		# init the map pipes
		for i in self.mapPipes:
			i.parentConnection, i.childConnection = Pipe()
		
		self.reducePipe.parentConnection, self.reducePipe.childConnection = Pipe()

		# create the list of mappers and reducers
		for i in xrange( self.MapReduceClassIndex.NumberOfMappers ):
			mapper = Mapper( self.mapPipes[ i ].childConnection, self.host, self.ipAddress )
			mapper.start()
			self.mapWorkers.append( mapper )

		# start the process for handling the mappers communication
		self.handler = Process( target=self.monitorMappersAndReducers )
		self.handler.start()

	def terminate( self ):
		self.reducePipe.parentConnection.send( 100 )
		time.sleep( 0.025 )

		# terminate the mappers and reducers
		for p in self.mapWorkers:
			p.terminate()

		# join the mappers and reducers
		for p in self.mapWorkers:
			p.join()

		# send the job to the mappers

		# terminate host handler and join it	
		self.handler.terminate()
		self.handler.join()

	def getIP( self ):
		# get the ip
		return self.ipAddress

	def getName( self ):
		return self.name

	def addMapper( self, whatMapper, bytesToTransmit, destinationIPAddress, outputFile, serverPort ):
		# print "%s sending %s" % ( self.name, [ destinationIPAddress, bytesToTransmit, port, outputFile ] )
		# send the job to the mappers
		self.mapPipes[ whatMapper ].parentConnection.send( [ destinationIPAddress, bytesToTransmit, outputFile, serverPort ] )

	def terminateMapper( self, whatMapper ):
		if self.availableMappers[ whatMapper ] == HostStates.Ready:
			print "terminate called on completed mappers"
			return -1

		# send the job to the mappers
		self.mapPipes[ whatMapper ].parentConnection.send( HostStates.Terminate )
		return 0

	def addReducer( self, whatReducer ):
		if self.availableReducers[ whatReducer ] == HostStates.Busy:
			return -1
			
		# send the job to the mappers
		self.availableReducers[ whatReducer ] = HostStates.Busy
		return 0

	def terminateReducer( self, whatReducer ):
		if self.availableReducers[ whatReducer ] == HostStates.Ready:
			return -1

		self.availableReducers[ whatReducer ] = HostStates.Ready
		return 0

	def killReducer( self, whatReducer ):
		# send the job to the mappers
		self.reducePipe.parentConnection.send( whatReducer )

	def startAllReducers( self ):
		portNumber = 5001

		for i in xrange( self.MapReduceClassIndex.NumberOfReducers ):
			command = "iperf3 -s -J -p %s 2>&1 > /dev/null" % portNumber
			pOpen = self.host.pexecNoWait( command )
			self.reduceWorkers.append( [ pOpen, command ] )
			portNumber += 1

	def monitorMappersAndReducers( self ):
		# store the mappers in a temp variable to only send data when mappers have changed
		availableMappers = [ i for i in self.availableMappers ]
		# availableReducers = [ i for i in self.availableReducers ]

		self.startAllReducers()
		
		while True:
			counter = 0
			# loop through the map pipes
			for i in self.mapPipes:
				# get the poll variable
				poll = i.parentConnection.poll()

				# if there is data to receive
				if poll == True:
					# get the message
					message = i.parentConnection.recv()

					availableMappers[ counter ] = message

				counter += 1

			# get the poll variable
			poll = self.reducePipe.childConnection.poll()

			# if there is data to receive
			if poll == True:
				# get the message
				message = self.reducePipe.childConnection.recv()

				if message == 100:
					self.reduceWorkers[ 0 ][ 0 ].kill()
					self.reduceWorkers[ 0 ][ 0 ].wait()	
					self.reduceWorkers[ 1 ][ 0 ].kill()
					self.reduceWorkers[ 1 ][ 0 ].wait()	
					break

				command = self.reduceWorkers[ message ][ 1 ]
				self.reduceWorkers[ message ][ 0 ].kill()
				self.reduceWorkers[ message ][ 0 ].wait()


				self.reduceWorkers[ message ][ 0 ] = self.host.pexecNoWait( command )
				print "%s killed and restarted reducer %s" % ( self.host, message )
				# stdOut, stdError = pOpen.communicate()
				# self.reduceWorkers.append( [ pOpen, stdOut, stdError ] )
					

			# if the mappers has changed this scan
			if self.availableMappers != availableMappers:
				# set the lists equal and send it
				self.availableMappers = [ i for i in availableMappers ]
				self.mapperOnePipe.send( self.availableMappers[ self.MapReduceClassIndex.MapperOne ] ) 
				self.mapperTwoPipe.send( self.availableMappers[ self.MapReduceClassIndex.MapperTwo ] ) 
				# print "mappers %s %s" % ( self.name, self.availableMappers )

			time.sleep( 0.02 )

