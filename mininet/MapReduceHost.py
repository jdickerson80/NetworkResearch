#!/usr/bin/python

from mininet.node import *
from PerProcessPipes import *
from multiprocessing import *
from enum import IntEnum
import time
import sys

def runReducer( port, host, ip ):
#    print "red start"
    command = "iperf3 -s -1 -p %s" % port
#    print ip + " " + command
    host.cmd( command )
#    time.sleep( 3 )
#    print "reduce after sleep"

def runMapper( list, host, ip ):
#     print "map start"
     command = "iperf3 -c %s -n %s -p %s" % ( list[0], list[1], list[2] )
#     print ip + " " + command
     host.cmd( command )
#     time.sleep( 3 )
#     print "map after sleep"

#class PerProcessPipes():
#    def __init__( self ):
#	self.parentConnection = None
#	self.childConnection  = None

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

	self.availableList = []
	portNumber = 5001
	self.ipAddress = host.IP()

	self.availableList.append( 0 )
	self.availableList.append( 0 )

	for i in self.mapPipes:
	    i.parentConnection, i.childConnection = Pipe()

	for i in self.reducePipes:
	    i.parentConnection, i.childConnection = Pipe()

	for i in range( self.MapReduceClassIndex.NumberOfMappers ):
	    self.mapWorkers.append( Process(target=self.runMapper, args=(self.mapPipes[ i ].childConnection, self.host, self.ipAddress, ) ) )

	for i in range( self.MapReduceClassIndex.NumberOfReducers ):
	    self.reduceWorkers.append( Process(target=self.runReducer, args=(self.reducePipes[ i ].childConnection, self.host, portNumber, self.ipAddress, ) ) )
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
	    receiveMessage = connection.recv()
	    connection.send( 0 )
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
	self.reducePipes[ whatReducer ].parentConnection.send( "go" )

    def handleHostProcesses( self ):
	self.schedularPipe.send( self.availableList )
        tempAvailableList = self.availableList
	while True:
	    count = 0
	    for i in self.mapPipes:
		poll = i.parentConnection.poll()

		if poll == True:
		    message = i.parentConnection.recv()
#                    print message
		    if message == 1:
                        tempAvailableList[ count ] += 1
		    else:
                        tempAvailableList[ count ] -= 1
	    count = count + 1

	    for i in self.reducePipes:
		poll = i.parentConnection.poll()
		if poll == True:
		    message = i.parentConnection.recv()
#                    print message
		    if message == 1:
                        tempAvailableList[ count ] += 1
		    else:
                        tempAvailableList[ count ] -= 1
#            print tempAvailableList
#            if tempAvailableList[ 0 ] != self.availableList[ 0 ] and tempAvailableList[ 1 ] != self.availableList[ 1 ]:
#                self.availableList = tempAvailableList
#                print "ASDFSADF" + self.availableList
                self.schedularPipe.send( tempAvailableList )
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
