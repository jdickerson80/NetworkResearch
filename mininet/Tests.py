#!/usr/bin/env python

from mininet.net import *
from mininet.node import *

import multiprocessing
import os
import signal
import time

class TestBaseClass( object ):
    def __init__( self, loggingDirectory, testDuration, network ):
		self.loggingDirectory = loggingDirectory
		self.testDuration = testDuration
		self.network = network

    def runTest( self ):
        pass

class Efficiency( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

class LongFlowHandling( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

class RandomFlowHandling( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

class ShortFlowHandling( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

class WCBandwidthUtilization( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

class WCLogic( TestBaseClass ):
    def __init__( self, loggingDirectory, testDuration, network ):
        TestBaseClass.__init__( self, loggingDirectory, testDuration, network )

    def runTest( self ):
	numberOfHosts = len( self.network.hosts )
	bandwidthValue = 100

	for host in xrange( numberOfHosts ):
	    for hostToRun in xrange( host ):
		currentHost = self.network.getNodeByName( "h%s" % str( hostToRun + 1 ) )
		print currentHost.IP(), currentHost.name
		command = "./TestHandler -b %s -t 5 -l ""/home/jd/Desktop/test%s.log"" -r ""%s"" -z ""ClientServer"" -q" % currentHost.IP()
#		print command
#		commandReturns = currentHost.cmd( command )

	    bandwidthValue = bandwidthValue + 100
	    print bandwidthValue



