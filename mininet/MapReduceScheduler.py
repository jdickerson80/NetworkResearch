#!/usr/bin/python

from mininet.node import *

from multiprocessing import *
#from multiprocessing.connection import wait
from enum import IntEnum
import time
import sys

class MapReduceScheduler( object ):

    @staticmethod
    def runReducer( port, host ):
	print "red start"
	command = "iperf3 -s -1 -p %s" % port
	print command
 #       host.cmd( command )
	time.sleep( 3 )

    @staticmethod
    def runMapper( list, host ):
	 print "map start"
	 command = "iperf3 -c %s -n %s -p %s" % ( list[0], list[1], list[2] )
	 print command
  #       host.cmd( command )
	 time.sleep( 3 )


    class PerProcessPipes():
	def __init__( self ):
	    self.parentConnection = None
            self.childConnection  = None

    class HostMapReduce( object ):
	class MapReduceClassIndex( IntEnum ):
	    MapperOne = 0
	    MapperTwo = 1
	    ReducerOne = 0
	    ReducerTwo = 1
	    MAX = 2

	def __init__( self, host, schedularPipe ):
	    self.host = host
	    self.schedularPipe = schedularPipe
            self.mapPipes    = [ MapReduceScheduler.PerProcessPipes() for i in range( self.MapReduceClassIndex.MAX ) ]
            self.reducePipes = [ MapReduceScheduler.PerProcessPipes() for i in range( self.MapReduceClassIndex.MAX ) ]
            self.mapWorkers  = []
	    self.reduceWorkers = []


	    for i in self.mapPipes:
		i.parentConnection, i.childConnection = Pipe()

	    for i in self.reducePipes:
		i.parentConnection, i.childConnection = Pipe()

	    for i in range( self.MapReduceClassIndex.MAX ):
                self.mapWorkers.append( Process(target=self.runMapper, args=(self.mapPipes[ i ].childConnection, self.host, ) ) )

	    for i in range( self.MapReduceClassIndex.MAX ):
                self.reduceWorkers.append( Process(target=self.runReducer, args=(self.reducePipes[ i ].childConnection, self.host, ) ) )

	    for p in self.mapWorkers:
		p.start()

	    for p in self.reduceWorkers:
		p.start()

            self.handler = Process( target=self.handleHostProcesses )
            self.handler.start()

        @staticmethod
        def runReducer( connection, host ):
            connection.send( "ready" )
	    while True:
#		receiveMessage = connection.recv()
#		runReducer( receiveMessage, host )
                connection.send( "ready" )
                time.sleep( 0.25 )

        @staticmethod
        def runMapper( connection, host ):
            connection.send( "ready" )
	    while True:
#		receiveMessage = connection.recv()
#		runReducer( receiveMessage, host )
                connection.send( "ready" )
                time.sleep( 0.25 )

        def handleHostProcesses( self ):
            while True:
                self.availableList = []
                for i in self.mapPipes:
                    poll = i.parentConnection.poll()

                    if poll == True:
                        message = i.parentConnection.recv()

                        if message == "ready":

                    print poll

                for i in self.reducePipes:
                    poll = i.parentConnection.poll()
                    print poll

                print "--------------------"
                time.sleep( 0.5 )

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

    def __init__( self, hostList, communicator ):
         print (sys.version)
         self.communicator = communicator
	 self.numberOfHosts = len( hostList )
	 self.hostMapReduceList = []
         self.availableList = []
	 self.pipeList = [ self.PerProcessPipes() for i in range( self.numberOfHosts ) ]
	 counter = 0

	 for i in self.pipeList:
	     i.parentConnection, i.childConnection = Pipe()

	 for i in hostList:
	     self.hostMapReduceList.append( self.HostMapReduce( host = i, schedularPipe = self.pipeList[ counter ].childConnection ) )
	     counter = counter + 1

#         self.handler = Process( target=self.handleHostMapReducers )
#         self.handler.start()

    def terminate( self ):
	for i in self.hostMapReduceList:
	    i.terminate()

        self.handler.terminate()
        self.handler.join()

    def handleHostMapReducers( self ):
        self.communicator.send( "ready" )
        while True:
            self.availableList = []
            for i in self.pipeList:
                poll = i.childConnection.poll( 0.25 )
                print poll

            print "NEXT"
            time.sleep( 0.25 )
#	    print count
"""class MapReduce( object ):
    queue = None

    class MapReduceClassIndex( IntEnum ):
	MapperOne = 0
	MapperTwo = 1
	ReducerOne = 0
	ReducerTwo = 1
	MAX = 2

    def __init__(self):
	self.mapWorkers = [ Process(target=self.runMapper) for i in range( self.MapReduceClassIndex.MAX ) ]
	self.reduceWorkers = [ Process(target=self.runReducer) for i in range( self.MapReduceClassIndex.MAX ) ]
	self.mapQueue = JoinableQueue()
	self.reduceQueue = JoinableQueue()
	self.host = None
	for p in self.mapWorkers:
	    p.start()

	for p in self.reduceWorkers:
	    p.start()

    def runReducer( self ):
	while True:
	    print "red start"
	    item = self.reduceQueue.get()
	    if item is not None:
		command = "iperf3 -s -1 -p %s" % item
		print command
		self.reduceQueue.task_done()
		print "reduce done"
	    else:
		print "reduce sleep"
		time.sleep( 0.25 )

    def runMapper( self ):
	while True:
	    item = self.mapQueue.get()
	    if item is not None:
		command = "iperf3 -c %s -n %s -p %s" % ( item[0], item[1], item[2])
		print command
		self.mapQueue.task_done()
		print "map done"
	    else:
		print "Map sleep"
		time.sleep( 0.25 )

    def setHost( self, host ):
	self.host = host

    def addMapJob( self, job ):
	self.mapQueue.put( job )

    def getMapJobs( self ):
	return self.mapQueue.qsize()

    def addReduceJob( self, job ):
	self.reduceQueue.put( job )

    def getReduceJobs( self ):
	return self.reduceQueue.qsize()

    def terminate( self ):
	self.mapQueue.join()
	self.reduceQueue.join()
	for p in self.mapWorkers:
	    p.terminate()

	for p in self.reduceWorkers:
	    p.terminate()

if __name__ == '__main__':
    p = MapReduce()
    p.addMapJob( [ "10.0.0.1", 1000, 5001 ] )
    p.addReduceJob( 5001 )
    time.sleep( 8 )

    p.addMapJob( [ "10.0.0.11", 11000, 5004 ] )
    p.addReduceJob( 5008 )

    p.terminate()
"""


"""
    class Iperf3Base( object ):
	def __init__( self ):
	    self.host = None

	def setHost( self, host ):
	    self.host = host
	    print host

	def run( self, **params ):
	    raise NotImplementedError()
	    pass

    class Iperf3Server( Iperf3Base ):
	def __init__( self ):
	    super().__init__( self )

	def run( self, **params ):
	    print "ASdfasdf"
	    if self.host == None:
		raise Exception( "iperf3Server execute called with no host" )
	    command = "iperf3 -s  -1 -p %s" % ( params.get( 'port' ) )
	    print command
	    time.sleep( 5 )
    #	return self.host.cmd( command )

    class Iperf3Client( Iperf3Base ):
	def __init__( self ):
	    super().__init__( self )

    #    def run( self, destinationIP, bytesToSend, port = 5001 ):
	def run( self, **params  ):
	    print "ASDFASDF"
	    if self.host == None:
		raise Exception( "iperf3Client execute called with no host" )
	    command = "iperf3 -c %s -n %s -p %s" % ( destinationIP, bytesToSend, port )
	    print command
	    time.sleep( 5 )
    #	return self.host.cmd( command )

    class Mapper( Iperf3Client ):
	"A mapper is simply an Iperf3Client"
	pass

    class Reducer( Iperf3Server ):
	"A reducer is simply an Iperf3Server"
	pass

    """
