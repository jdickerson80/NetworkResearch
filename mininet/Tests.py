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
	host = 1
	while host <= numberOfHosts:
	    print host
	    host = host + 1




