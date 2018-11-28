#!/usr/bin/python

from mininet.node import *
from MapReduceScheduler import Statistics
from multiprocessing import Process
from threading import Thread
import time
import sys

class Job( Process ):
    def __init__( self, schedularPipe ):
        self.schedularPipe = schedularPipe

    @staticmethod
    def getTime():
        return time.strftime( "%H:%M:%S", time.localtime() )

    @staticmethod
    def runReducer( port, host, ip ):
    #    print "red start"
        command = "iperf3 -s -1 -p %s" % port
    #    print ip + " " + command
        host.cmd( command )
    #    time.sleep( 3 )
    #    print "reduce after sleep"

    @staticmethod
    def runMapper( host, destinationIP, port, bytesToSend ):
    #     print "map start"
         command = "iperf3 -c %s -n %s -p %s" % ( destinationOP, bytesToSend, port )
    #     print ip + " " + command
         host.cmd( command )
    #     time.sleep( 3 )
    #     print "map after sleep"

    @staticmethod
    def setIperfPair( hostOne, hostTwo, numberOfBytesToSend ):
        threads = []
        hostOneIP = hostOne.getIP()
        hostTwoIP = hostTwo.getIP()
        numberOfBytesToSend = numberOfBytesToSend / 2

        threads.append( Thread( target=self.runReducer( 5001, hostOne, hostOneIP ) ) )
        threads.append( Thread( target=self.runMapper( hostTwo, hostOneIP, 5001, numberOfBytesToSend ) ) )

        threads.append( Thread( target=self.runReducer( 5002, hostTwo, hostTwoIP ) ) )
        threads.append( Thread( target=self.runMapper( hostOne, hostTwoIP, 5001, numberOfBytesToSend ) ) )

        threads.append( Thread( target=self.runReducer( 5001, hostTwo, hostTwoIP ) ) )
        threads.append( Thread( target=self.runMapper( hostOne, hostTwoIP, 5001, numberOfBytesToSend ) ) )

        threads.append( Thread( target=self.runReducer( 5002, hostOne, hostTwoIP ) ) )
        threads.append( Thread( target=self.runMapper( hostTwo, hostOneIP, 5001, numberOfBytesToSend ) ) )

        return threads

    def run( self ):
        threadList = []
        jobStatistic = None
        while True:
            receiveMessage = self.schedularPipe.recv()
            jobStatistic = receiveMessage
            for pair in xrange( 0, len( jobStatistic.hosts ), 2 ):
                threadList.append( self.setIperfPair( jobStatistic.hosts( pair ), jobStatistic.hosts( pair + 1 ), jobStatistic.bytesToSend ) )

            jobStatistic.startTime = self.getTime()

            for thread in threads:
                thread.start()
                time.sleep( 0.5 )

            for thread in threads:
                thread.join()

            jobStatistic.startTime = self.getTime()
            connection.send( jobStatistic )
