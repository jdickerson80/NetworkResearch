#!/usr/bin/python

import threading
from MapReduceHost import *
import time
import sys

class MapReduceManager( object ):
    def __init__( self, hostList, availableHostCommunicator ):
         self.availableHostCommunicator = availableHostCommunicator
         numberOfHosts = len( hostList )
	 self.hostMapReduceList = []
         self.availableList = []
         self.pipeList = [ PerProcessPipes() for i in range( numberOfHosts ) ]
	 self.hostHandlerRunning = True
	 self.taskHandlerRunning = True
	 counter = 0

	 for i in self.pipeList:
	     i.parentConnection, i.childConnection = Pipe()

	 for i in hostList:
             self.hostMapReduceList.append( HostMapReduce( host = i, schedularPipe = self.pipeList[ counter ].childConnection ) )
	     counter = counter + 1

	 self.hostHandler = threading.Thread( target=self.handleAvailableHostCommunication )
	 self.hostHandler.start()

	 self.taskHandler = threading.Thread( target=self.handleTaskCommunication )
	 self.taskHandler.start()

    def terminate( self ):
	self.hostHandlerRunning = False
	self.taskHandlerRunning = False
	self.hostHandler.join()
	self.taskHandler.join()
	for i in self.hostMapReduceList:
	    i.terminate()

    def availableHosts( self ):
	return self.availableList

    def handleAvailableHostCommunication( self ):
        self.availableHostCommunicator.send( "ready" )
        tempAvailableList = []
        self.availableList = []
	while self.hostHandlerRunning == True:
#	    print "available"
            tempAvailableList = []
            for i in self.pipeList:
		poll = i.parentConnection.poll()
                if poll == True:
                    message = i.parentConnection.recv()
                    tempAvailableList.append( message )

            if tempAvailableList != self.availableList:
		self.availableList = tempAvailableList
		print self.availableList
		self.availableHostCommunicator.send( self.availableList )

	    time.sleep( 0.125 )


    def handleTaskCommunication( self ):
        messageLength = 0
	while self.taskHandlerRunning == True:
#	    print "task"
	    poll = self.availableHostCommunicator.poll()
            if poll == True:
                message = self.availableHostCommunicator.recv()

                messageLength = len( message )
                if ( messageLength == 5 ):
                    self.hostMapReduceList[ message[ 0 ] ].addMapJob( message[ 1 ], message[2:5] )
                if ( messageLength == 3 ):
                    self.hostMapReduceList[ message[ 0 ] ].addReduceJob( message[ 1 ], message[ 2 ] )

	    time.sleep( 0.125 )
