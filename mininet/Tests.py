#!/usr/bin/env python

from mininet.net import *
from mininet.node import *

import multiprocessing
import os
import signal
import time

def iperfServerWrapper( host, port ):
    serverCommand = 'iperf3 -s -i 1 -p %s -1 >> /tmp/%s/%sServer.log &' % ( port, host.name, host.name )
    print serverCommand
    host.cmd( serverCommand )

def iperfClientWrapper( host, port, time, logPath ):
    clientCommand = 'iperf3 -c %s -i 1 -p %s -t %s >> /tmp/%s/%sClient.log &' % ( host.IP(), port, time, logPath, logPath )
    print clientCommand
    host.cmd( clientCommand )

class TestBaseClass( object ):
     def __init__( self, hostOne, hostTwo, loggingDirectory, testDuration, network ):
	 self.hostOne = hostOne
	 self.hostTwo = hostTwo
	 self.loggingDirectory = loggingDirectory
	 self.testDuration = testDuration
	 self.network = network

    def runTest( self ):
	pass

class iperfTester( TestBaseClass ):

    def __init__( self, hostOne, hostTwo, loggingDirectory, testDuration, network ):
	TestBaseClass.__init__( self, hostOne, hostTwo, loggingDirectory, testDuration, network ):

    def runTest( self ):
	jobs = []
	port = 5001
	for i in range( numberOfLinks ):
	    p = multiprocessing.Process( target=iperfPair, args=( hostOne, hostTwo, testDuration, net, port ) )
	    jobs.append( p )
	    p.start()
	    port += 2

	for j in jobs:
	    j.join()
	    print 'Tester %s.exitcode = %s' % (j.name, j.exitcode)

    def iperfPair( self, port ):
	iperfServerWrapper( self.hostOne, int( port ) )
	iperfServerWrapper( self.hostTwo, int( port ) + 1 )

	time.sleep( 0.25 )

	iperfClientWrapper( self.hostTwo, int( port ) + 1, self.testDuration, self.hostOne.name )
	iperfClientWrapper( self.hostOne, int( port ), self.testDuration, self.hostTwo.name )
