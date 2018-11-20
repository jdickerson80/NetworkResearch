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
#		print "manager has something new"

	    time.sleep( 0.025 )

#    def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
#	hostOneDestIP = self.hostMapReduceList[ hostOne ].getIP()
#	hostTwoDestIP = self.hostMapReduceList[ hostTwo ].getIP()
#	numberOfBytesToSend = numberOfBytesToSend / 2
#	self.hostMapReduceList[ hostOne ].addMapJobPair( 0, numberOfBytesToSend, hostTwoDestIP )
#	self.hostMapReduceList[ hostOne ].addMapJobPair( 1, numberOfBytesToSend, hostTwoDestIP )
#	self.hostMapReduceList[ hostTwo ].addMapJobPair( 0, numberOfBytesToSend, hostOneDestIP )
#	self.hostMapReduceList[ hostTwo ].addMapJobPair( 1, numberOfBytesToSend, hostOneDestIP )

    def setIperf( self, hostOne, hostOneMapper, hostOneReducer, hostTwo, hostTwoMapper, hostTwoReducer, numberOfBytesToSend ):
#	print [ hostOne, hostOneMapper, hostOneReducer, hostTwo, hostTwoMapper, hostTwoReducer, numberOfBytesToSend ]
	hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
	hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
	numberOfBytesToSend = numberOfBytesToSend / 2
	self.hostMapReduceList[ hostOne ].addReducePair( hostOneReducer )
#	self.hostMapReduceList[ hostTwo ].addReducePair( hostTwoReducer )

	time.sleep( 5 )

#	self.hostMapReduceList[ hostOne ].addMapPair( hostOneMapper, numberOfBytesToSend, hostTwoIP )
	self.hostMapReduceList[ hostTwo ].addMapPair( hostTwoMapper, numberOfBytesToSend, hostOneIP )


    def setIperf1( self, hostOne, hostOneMapper, hostTwo, hostTwoReducer, numberOfBytesToSend ):
#	print [ hostOne, hostOneMapper, hostTwo, hostTwoReducer, numberOfBytesToSend ]
	hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
	hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
	numberOfBytesToSend = numberOfBytesToSend / 2
	self.hostMapReduceList[ hostOne ].addReducePair( hostOneMapper )

	time.sleep( 1.5 )

	self.hostMapReduceList[ hostTwo ].addMapPair( hostTwoReducer, numberOfBytesToSend, hostOneIP )

    def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
	hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
	hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
	numberOfBytesToSend = numberOfBytesToSend / 2
	self.hostMapReduceList[ hostOne ].addReducePair( 0 )
	self.hostMapReduceList[ hostOne ].addReducePair( 1 )

	self.hostMapReduceList[ hostTwo ].addReducePair( 0 )
	self.hostMapReduceList[ hostTwo ].addReducePair( 1 )

	time.sleep( 1 )

	self.hostMapReduceList[ hostOne ].addMapPair( 0, numberOfBytesToSend, hostTwoIP )
	self.hostMapReduceList[ hostOne ].addMapPair( 1, numberOfBytesToSend, hostTwoIP )

	self.hostMapReduceList[ hostTwo ].addMapPair( 0, numberOfBytesToSend, hostOneIP )
	self.hostMapReduceList[ hostTwo ].addMapPair( 1, numberOfBytesToSend, hostOneIP )

    def handleTaskAssignment( self, message ):
	messageLength = len( message )
	if ( messageLength == 5 ):
	    self.hostMapReduceList[ message[ 0 ] ].addMapJob( message[ 1 ], message[2:5] )
	elif ( messageLength == 3 ):
	    self.hostMapReduceList[ message[ 0 ] ].addReduceJob( message[ 1 ] )

#	    time.sleep( 0.125 )
