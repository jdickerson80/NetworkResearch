#!/usr/bin/python

from mininet.node import *
from MapReduceScheduler import *
from multiprocessing import Process
import threading
import time
import sys

class ReducerThread( threading.Thread ):
    def __init__( self, host, port ):
	threading.Thread.__init__( self )
	self.host = host
	self.port = port
	self.myIP = self.host.IP()

    def run( self ):
    #    print "red start"
	command = "iperf3 -s -1 -p %s" % self.port
	shouldRun = True
	print self.myIP + " " + command
	while shouldRun == True:
	    try:
		self.host.cmd( command )
		print "reducer BROKE"
		shouldRun = False
	    except AssertionError as error:
#		print "reduce continue"
		time.sleep( 0.125 )

    #    time.sleep( 3 )
    #    print "reduce after sleep"

class MapperThread( threading.Thread ):
    def __init__( self, host, port, destinationIP, bytesToSend ):
	threading.Thread.__init__( self )
	self.host = host
	self.port = port
	self.destinationIP = destinationIP
	self.bytesToSend = bytesToSend
	self.myIP = self.host.IP()

    def run( self ):
#	 print "map start"
	 shouldRun = True
	 command = "iperf3 -c %s -n %s -p %s" % ( self.destinationIP, self.bytesToSend, self.port )
	 print self.myIP + " " + command
	 while shouldRun == True:
	     try:
		 print "mapper BROKE"
		 self.host.cmd( command )
		 shouldRun = False
	     except AssertionError as error:
#		 print "map continue"
		 time.sleep( 0.125 )

    #     time.sleep( 3 )
    #     print "map after sleep"


class MapReduceJob( Process ):
    def __init__( self, schedulerPipe, hostList ):
	super( MapReduceJob, self ).__init__()
	self.schedulerPipe = schedulerPipe
	self.hostList = hostList
	self.threads = []

    @staticmethod
    def getTime():
        return time.strftime( "%H:%M:%S", time.localtime() )


    def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
#	threads = []
	hostOneIP = hostOne.IP()
	hostTwoIP = hostTwo.IP()
        numberOfBytesToSend = numberOfBytesToSend / 2

	self.threads.append( ReducerThread( hostOne, 5001 ) )
	self.threads.append( ReducerThread( hostOne, 5002 ) )
	self.threads.append( ReducerThread( hostTwo, 5001 ) )
	self.threads.append( ReducerThread( hostTwo, 5002 ) )

	self.threads.append( MapperThread(  hostOne, 5001, hostTwoIP, numberOfBytesToSend ) )
	self.threads.append( MapperThread(  hostOne, 5002, hostTwoIP, numberOfBytesToSend ) )
	self.threads.append( MapperThread(  hostTwo, 5001, hostOneIP, numberOfBytesToSend ) )
	self.threads.append( MapperThread(  hostTwo, 5002, hostOneIP, numberOfBytesToSend ) )
#	print "DONE!!!"
#	print self.threads

    def run( self ):
	jobStatistic = None

        while True:
	    self.threads = []
	    receiveMessage = self.schedulerPipe.recv()
            jobStatistic = receiveMessage
	    print jobStatistic
#	    print len( jobStatistic.hosts )
            for pair in xrange( 0, len( jobStatistic.hosts ), 2 ):
#		print jobStatistic.hosts[ pair ], jobStatistic.hosts[ pair + 1 ]
		self.setIperfPair( self.hostList[ jobStatistic.hosts[ pair ] ], self.hostList[ jobStatistic.hosts[ pair + 1 ] ], jobStatistic.bytesToSend )

            jobStatistic.startTime = self.getTime()

	    for thread in self.threads:
		thread.start()
#		while thread.is_alive() == False:
#		    print "WAITING!!!!!"
		time.sleep( 1 )

	    for thread in self.threads:
                thread.join()

	    jobStatistic.endTime = self.getTime()
	    self.schedulerPipe.send( jobStatistic )
