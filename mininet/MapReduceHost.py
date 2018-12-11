#!/usr/bin/python

from mininet.node import *
from PerProcessPipes import *
from multiprocessing import *
from enum import IntEnum
import time
import sys

def runReducer( port, host, ip ):
	# print "red start"
	command = "iperf3 -s -i 1 -p %s" % port
	# print ip + " " + command
	print host.cmd( command )
	# time.sleep( 3 )
	# print "reduce after sleep"

def runMapper( list, host, ip ):
	# print "map start"
	command = "iperf3 -c %s -i 1 -n %s -p %s" % ( list[0], list[1], list[2] )
	# print ip + " " + command
	print host.cmd( command )
	# time.sleep( 3 )
	# print "map after sleep"


class HostMapReduce( object ):
    class MapReduceClassIndex( IntEnum ):
		MapperOne = 0
		MapperTwo = 1
		ReducerOne = 2
		ReducerTwo = 3
		NumberOfMappers = 2
		NumberOfReducers = 2


    def __init__( self, host, schedularPipe ):
		self.host = host
		self.schedularPipe = schedularPipe
		self.mapPipes    = [ PerProcessPipes() for i in range( self.MapReduceClassIndex.NumberOfMappers  ) ]
		self.reducePipes = [ PerProcessPipes() for i in range( self.MapReduceClassIndex.NumberOfReducers ) ]
		self.mapWorkers  = []
		self.reduceWorkers = []

		self.availableListOne = []
		self.availableListTwo = []
		portNumber = 5001
		self.ipAddress = host.IP()

		self.availableListOne.append( 0 )
		self.availableListOne.append( 2 )

		self.availableListTwo.append( 0 )
		self.availableListTwo.append( 2 )

		for i in self.mapPipes:
		    i.parentConnection, i.childConnection = Pipe()

		for i in self.reducePipes:
		    i.parentConnection, i.childConnection = Pipe()

		for i in xrange( self.MapReduceClassIndex.NumberOfMappers ):
		    self.mapWorkers.append( Process(target=self.runMapper, args=(self.mapPipes[ i ].childConnection, self.host, self.ipAddress, ) ) )

		for i in xrange( self.MapReduceClassIndex.NumberOfReducers ):
		    # self.reduceWorkers.append( Process(target=self.runReducer, args=(self.reducePipes[ i ].childConnection, self.host, portNumber, self.ipAddress, ) ) )
		    self.reduceWorkers.append( Process(target=runReducer, args=(portNumber, self.host, self.ipAddress, ) ) )
		    portNumber = portNumber + 1

		for p in self.mapWorkers:
		    p.start()

		for p in self.reduceWorkers:
		    p.start()

		self.handler = Process( target=self.handleHostProcesses )
		self.handler.start()

    @staticmethod
    def runReducer( connection, host, port, ip ):
		time.sleep( 0.5 )
		connection.send( 1 )
		while True:
		    # receiveMessage = connection.recv()
		    connection.send( 1 )
		    runReducer( port, host, ip )
		    connection.send( 1 )

    @staticmethod
    def runMapper( connection, host, ip ):
		time.sleep( 0.5 )
		connection.send( 1 )
		while True:
		    receiveMessage = connection.recv()
		    connection.send( 0 )
		    runMapper( receiveMessage, host, ip )
		    connection.send( 1 )

    def getIP( self ):
		return self.ipAddress

    def addMapper( self, whatMapper, bytesToTransmit, destinationIPAddress ):
		if whatMapper % 2 == 0:
		    port = 5001
		else:
		    port = 5002
		self.mapPipes[ whatMapper ].parentConnection.send( [ destinationIPAddress, bytesToTransmit, port ] )

    def addReducer( self, whatReducer ):
    	pass
		# self.reducePipes[ whatReducer ].parentConnection.send( "go" )

    def handleHostProcesses( self ):
		self.schedularPipe.send( self.availableListOne )
		while True:
			count = 0
			for i in self.mapPipes:
				poll = i.parentConnection.poll()
				if poll == True:
					message = i.parentConnection.recv()
					if message == 1:
						# print "map %i got a one"  % count
						self.availableListTwo[ count ] += 1
					else:
						# print "map %i got a zero"  % count
						self.availableListTwo[ count ] -= 1
			#		    print tempAvailableList[ count ]

			# count += 1

			# for i in self.reducePipes:
			# 	poll = i.parentConnection.poll()
			# 	if poll == True:
			# 		message = i.parentConnection.recv()
			# 		if message == 1:
			# 			# print "reduce %i got a one"  % count
			# 			self.availableListTwo[ count ] += 1
			# 		else:
			# 			# print "reduce %i got a zero"  % count
			# 			self.availableListTwo[ count ] -= 1

			# print self.availableListTwo
			
			if self.availableListOne != self.availableListTwo:
				self.availableListOne = [ i for i in self.availableListTwo ]
				self.schedularPipe.send( self.availableListOne )
				# print "sending %s" % self.availableListOne

			time.sleep( 0.025 )

    def terminate( self ):
		for p in self.mapWorkers:
			p.terminate()

		for p in self.reduceWorkers:
			p.terminate()

		for p in self.mapWorkers:
			p.join()

		for p in self.reduceWorkers:
			p.join()

		self.handler.terminate()
		self.handler.join()
