#!/usr/bin/python

from MapReduceManager import MapReduceManager
from multiprocessing import Pipe
import time
import sys

class MapReduceScheduler( object ):
    def __init__( self, hostList ):
	self.thisChannel, self.mapReduceManagerChannel = Pipe()
	self.manager = MapReduceManager( hostList, self.mapReduceManagerChannel )

    def terminate( self ):
	self.manager.terminate()

    @staticmethod
    def preprocessJobList( workFile ):
	for job in workFile:
	    job = job.replace( "\n", "" )
	    job = job.replace( "job ", "" )
	    job = job.replace( " ", "" )

    def runJobs( self, workFile ):
	self.workFile = self.preprocessJobList( workFile )
	for job in workFile:
	    jobComponents = job.split( "," )
	    if int( jobComponents[ 1 ] ) == 0:
		continue

	    remainder = int( jobComponents[ 1 ] ) % 64000000
	    hostsNeeded = ( int( jobComponents[ 1 ] ) / 64000000 )
	    if remainder != 0:
		hostsNeeded = hostsNeeded + 1

	    for i in xrange( 0, 10 ):
		availableHosts = self.manager.availableHosts()
		print availableHosts
		time.sleep( 0.125 )
#	    while True:
#		print "polling"
#		poll = self.thisChannel.poll( .50 )
#		if poll == True:
#		    message = self.thisChannel.recv()
#		    break
#	    print message
#	    numberOfHostsNeeded = jobComponents[ 1 ] % 64
#	    print numberOfHostsNeeded
