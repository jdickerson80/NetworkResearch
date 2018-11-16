#!/usr/bin/python

import threading
from MapReduceHost import *
import time
import sys

class MapReduceManager( object ):
    def __init__( self, hostList, callbackFunction ):
	 self.callbackFunction = callbackFunction
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

#	 self.taskHandler = threading.Thread( target=self.handleTaskCommunication )
#	 self.taskHandler.start()

    def terminate( self ):
	self.hostHandlerRunning = False
	self.taskHandlerRunning = False
	self.hostHandler.join()
#	self.taskHandler.join()
	for i in self.hostMapReduceList:
	    i.terminate()

    def availableHosts( self ):
	return self.availableList

#    def availableHostsCount( self ):


    def handleAvailableHostCommunication( self ):
        tempAvailableList = []
        self.availableList = []
	while self.hostHandlerRunning == True:
            tempAvailableList = []
            for i in self.pipeList:
		poll = i.parentConnection.poll()
                if poll == True:
                    message = i.parentConnection.recv()
                    tempAvailableList.append( message )

            if tempAvailableList != self.availableList:
		self.availableList = tempAvailableList
		self.callbackFunction( self.availableList )
#		print self.availableList

	    time.sleep( 0.125 )

    def handleTaskAssignment( self, message ):
	messageLength = len( message )
	if ( messageLength == 5 ):
	    self.hostMapReduceList[ message[ 0 ] ].addMapJob( message[ 1 ], message[2:5] )
	elif ( messageLength == 3 ):
	    self.hostMapReduceList[ message[ 0 ] ].addReduceJob( message[ 1 ] )

#	    time.sleep( 0.125 )
