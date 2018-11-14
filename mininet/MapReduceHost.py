#!/usr/bin/python

from mininet.node import *

from multiprocessing import *
from enum import IntEnum
import time
import sys

def runReducer( port, host ):
#    print "red start"
    command = "iperf3 -s -1 -p %s" % port
    print command
    host.cmd( command )
#    time.sleep( 3 )

def runMapper( list, host ):
#     print "map start"
     command = "iperf3 -c %s -n %s -p %s" % ( list[0], list[1], list[2] )
     print command
     host.cmd( command )
#     time.sleep( 3 )

class PerProcessPipes():
    def __init__( self ):
	self.parentConnection = None
	self.childConnection  = None

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

	for i in self.mapPipes:
	    i.parentConnection, i.childConnection = Pipe()

	for i in self.reducePipes:
	    i.parentConnection, i.childConnection = Pipe()

	for i in range( self.MapReduceClassIndex.NumberOfMappers ):
	    self.mapWorkers.append( Process(target=self.runMapper, args=(self.mapPipes[ i ].childConnection, self.host, ) ) )
	    self.availableList.append( False )

	for i in range( self.MapReduceClassIndex.NumberOfReducers ):
	    self.reduceWorkers.append( Process(target=self.runReducer, args=(self.reducePipes[ i ].childConnection, self.host, ) ) )
	    self.availableList.append( False )

	for p in self.mapWorkers:
	    p.start()

	for p in self.reduceWorkers:
	    p.start()

	self.handler = Process( target=self.handleHostProcesses )
	self.handler.start()

    @staticmethod
    def runReducer( connection, host ):
	time.sleep( 0.5 )
	connection.send( "ready" )
	while True:
	    receiveMessage = connection.recv()
	    connection.send( "starting" )
	    runReducer( receiveMessage, host )
	    connection.send( "ready" )
#            time.sleep( 0.125 )

    @staticmethod
    def runMapper( connection, host ):
	time.sleep( 0.5 )
	connection.send( "ready" )
	while True:
	    receiveMessage = connection.recv()
	    connection.send( "starting" )
	    runMapper( receiveMessage, host )
	    connection.send( "ready" )
#            time.sleep( 0.125 )

#    def isMapperAvailable( self, whatMapper ):
#        poll = self.mapPipes[ whatMapper ].parentConnection.poll()

    def addMapJob( self, whatMapper, message ):
	self.mapPipes[ whatMapper ].parentConnection.send( message )

    def addReduceJob( self, whatReduce, message ):
	self.reducePipes[ whatReduce ].parentConnection.send( message )

    def handleHostProcesses( self ):
	self.schedularPipe.send( self.availableList )
	while True:
	    tempAvailableList = self.availableList
	    count = 0
	    for i in self.mapPipes:
		poll = i.parentConnection.poll()

		if poll == True:
		    message = i.parentConnection.recv()

		    if message == "ready":
			self.availableList[ count ] = True
		    else:
			self.availableList[ count ] = False
		count = count + 1

	    for i in self.reducePipes:
		poll = i.parentConnection.poll()
		if poll == True:
		    message = i.parentConnection.recv()

		    if message == "ready":
			self.availableList[ count ] = True
		    else:
			self.availableList[ count ] = False
		count = count + 1

	    self.schedularPipe.send( self.availableList )

	    time.sleep( 0.125 )

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

#	    print tempAvailableList
#	    if tempAvailableList != self.availableList:
#		self.schedularPipe.send( self.availableList )
#		print "SENDING"
#	    shouldSend = False
#	    for i in xrange( 0, 4 ):
##		print i
#		print "t %i s %i" % (tempAvailableList[ i ], self.availableList[ i ] )
#		if tempAvailableList[ i ] != self.availableList[ i ]:
#		    self.availableList[ i ] = tempAvailableList[ i ]
#		    shouldSend = True
#		    print "DIFFERENT!!!!!!!!!!!!!!!!!!!!!!!!"
#		    break

#	    print tempAvailableList
#	    print self.availableList

#	    if shouldSend == True:
#		self.schedularPipe.send( self.availableList )
#	    if tempAvailableList != self.availableList:
#		self.availableList = tempAvailableList
#		print self.availableList
#		self.schedularPipe.send( self.availableList )
#	    print self.availableList
