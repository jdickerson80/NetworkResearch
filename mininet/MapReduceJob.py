#!/usr/bin/python

from mininet.node import *
from MapReduceScheduler import *
from MapReduceHost import *
from multiprocessing import Process
#import threading
import time
import sys

class MapReduceJob( Process ):
    def __init__( self, schedulerPipe, hostMapReduceList, hostPipeList ):
	super( MapReduceJob, self ).__init__()
	self.schedulerPipe = schedulerPipe
        self.hostMapReduceList = hostMapReduceList
        self.hostPipeList = hostPipeList

    @staticmethod
    def getTime():
        return time.strftime( "%H:%M:%S", time.localtime() )

    def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
        hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
        hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
        numberOfBytesToSend = numberOfBytesToSend / 2

        self.hostMapReduceList[ hostOne ].addReducer( 0 )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostTwo ].addMapper( 0, numberOfBytesToSend, hostOneIP )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostOne ].addReducer( 1 )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostTwo ].addMapper( 1, numberOfBytesToSend, hostOneIP )
        time.sleep( 0.5 )


        self.hostMapReduceList[ hostTwo ].addReducer( 0 )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostOne ].addMapper( 0, numberOfBytesToSend, hostTwoIP )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostTwo ].addReducer( 1 )
        time.sleep( 0.5 )
        self.hostMapReduceList[ hostOne ].addMapper( 1, numberOfBytesToSend, hostTwoIP )
        time.sleep( 0.5 )

    def run( self ):
	jobStatistic = None

        while True:
            jobPipes = []
	    receiveMessage = self.schedulerPipe.recv()
            jobStatistic = receiveMessage
            jobStatistic.startTime = self.getTime()
            for pair in xrange( 0, len( jobStatistic.hosts ), 2 ):
                self.setIperfPair( jobStatistic.hosts[ pair ], jobStatistic.hosts[ pair + 1 ], jobStatistic.bytesToSend )
                jobPipes.append( self.hostPipeList[ pair ] )
                jobPipes.append( self.hostPipeList[ pair + 1 ] )

            while True:
                count = 0
                for i in jobPipes:
                    poll = i.parentConnection.poll()
                    if poll == True:
                        message = i.parentConnection.recv()
                        if message == [ 2, 2 ]:
                            del jobPipes[ count ]
                    count += 1
                if len( jobPipes ) == 0:
                    break
                time.sleep( 0.025 )

	    jobStatistic.endTime = self.getTime()
	    self.schedulerPipe.send( jobStatistic )
