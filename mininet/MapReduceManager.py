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

    def terminate( self ):
	self.hostHandlerRunning = False
	self.taskHandlerRunning = False
	self.hostHandler.join()
	for i in self.hostMapReduceList:
	    i.terminate()

    def availableHosts( self ):
	return self.availableList

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

	    time.sleep( 0.025 )

    def setIperf( self, hostOne, hostOneMapper, hostTwo, hostTwoReducer, numberOfBytesToSend ):
	hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
	hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
	numberOfBytesToSend = numberOfBytesToSend / 2
	self.hostMapReduceList[ hostOne ].addReducer( hostOneMapper )

	time.sleep( 1.5 )

	self.hostMapReduceList[ hostTwo ].addMapper( hostTwoReducer, numberOfBytesToSend, hostOneIP )

    def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
	hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
	hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
	numberOfBytesToSend = numberOfBytesToSend / 2
	self.hostMapReduceList[ hostOne ].addReducer( 0 )
	self.hostMapReduceList[ hostOne ].addReducer( 1 )

	self.hostMapReduceList[ hostTwo ].addReducer( 0 )
	self.hostMapReduceList[ hostTwo ].addReducer( 1 )

	time.sleep( 1 )

	self.hostMapReduceList[ hostOne ].addMapper( 0, numberOfBytesToSend, hostTwoIP )
	self.hostMapReduceList[ hostOne ].addMapper( 1, numberOfBytesToSend, hostTwoIP )

	self.hostMapReduceList[ hostTwo ].addMapper( 0, numberOfBytesToSend, hostOneIP )
	self.hostMapReduceList[ hostTwo ].addMapper( 1, numberOfBytesToSend, hostOneIP )
