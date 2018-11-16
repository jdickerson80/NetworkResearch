#!/usr/bin/python

from enum import IntEnum
from MapReduceManager import MapReduceManager
from Queue import Queue
import time
import sys
import time

class MapReduceScheduler( object ):
    class BytesPerHostEnum( IntEnum ):
        BytesPerHost = 64000000

    def __init__( self, hostList, workFile ):
        with open( workFile ) as f:
            data = f.readlines()

	self.workQueue = Queue()
	self.numberOfHosts = int ( len( hostList ) )
        self.__preprocessJobList( data )
        self.__processJobs( data )
	self.availableList = []
	self.hasListBeenUpdated = False
	self.jobList = []

#	for i in xrange( self.workQueue.qsize() ):
#	    print self.workQueue.get_nowait()

	print "Size %i" % self.workQueue.qsize()
	print self.getTime()
	self.manager = MapReduceManager( hostList, self.managerCallback )

    def terminate( self ):
	self.manager.terminate()

    @staticmethod
    def getTime():
	return time.strftime("%H:%M:%S", time.localtime())

    @staticmethod
    def __preprocessJobList( workFile ):
	for job in workFile:
	    job = job.replace( "\n", "" )
	    job = job.replace( "job ", "" )
	    job = job.replace( " ", "" )
#	    print job

    def __processJobs( self, workFile ):
	inputBytes = 0
	hostNeeded = 0
	for job in workFile:
	    jobComponents = job.split( "\t" )

	    if jobComponents[ 0 ] == '\n':
		continue

	    if int( jobComponents[ 2 ] ) == 0:
		continue

	    inputBytes = int( jobComponents[ 1 ] )

            remainder = inputBytes % self.BytesPerHostEnum.BytesPerHost
            hostsNeeded = inputBytes / self.BytesPerHostEnum.BytesPerHost

	    if remainder != 0 or hostsNeeded == 0:
		hostsNeeded = hostsNeeded + 1

	    if ( int ( hostsNeeded ) > int ( self.numberOfHosts ) ):
#		print "TOO MANY HOSTS!!"
		continue

#	    print hostsNeeded, self.numberOfHosts
	    self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ) ], block=False )


    def managerCallback( self, availableList ):
	self.availableList = availableList
	self.hasListBeenUpdated = True


    def runTest( self ):
	if self.workQueue.empty() == True:
	    raise LookupError( "Running test on an empty queue" )
	currentAvailableHostList = self.availableList
	beginningHost = 0
	endHost = self.numberOfHosts
	currentJob = []
	jobHosts = []
#	jobStartTime = []
	hostsLeft = self.numberOfHosts

	for i in xrange( 0, self.workQueue.qsize() ):
	    currentJob = self.workQueue.get_nowait()
	    for i in xrange( currentJob[ 0 ] ):
		pass





