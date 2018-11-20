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

	print "Queue size: %i" % self.workQueue.qsize()
	self.manager = MapReduceManager( hostList, self.managerCallback )
	self.hasListBeenUpdated = False

    def terminate( self ):
	self.manager.terminate()

    @staticmethod
    def getTime():
	return time.strftime( "%H:%M:%S", time.localtime() )

    @staticmethod
    def __preprocessJobList( workFile ):
	for job in workFile:
	    job = job.replace( "\n", "" )
	    job = job.replace( "job ", "" )
	    job = job.replace( " ", "" )

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
		continue

	    self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ) ], block=False )


    def managerCallback( self, availableList ):
	self.availableList = availableList
	self.hasListBeenUpdated = True

    def traverseOuterPairs( self, numberOfHosts, requiredHosts, bytesToTransmit ):
	currentAvailableHostList = self.availableList
	print "outer %s" % currentAvailableHostList
	for j in xrange( numberOfHosts ):
	    if currentAvailableHostList[ j ][ 0 ] != 0:
		mapperHost = j
		mapperHostNumber = currentAvailableHostList[ j ][ 0 ] - 1
		for h in xrange( numberOfHosts - 1, 0, -1 ):
		    if currentAvailableHostList[ h ][ 1 ] != 0:
			if h == j:
			    print "same host"
			    break
			elif j > h:
			    print "wrap around"
			    break
			reducerHost = h
			reducerHostNumber = currentAvailableHostList[ h ][ 1 ] - 1
			requiredHosts = requiredHosts - 1
			self.manager.setIperf( reducerHost, reducerHostNumber, mapperHost, mapperHostNumber, bytesToTransmit )
			break
		    else:
			if h == 0:
			    j = 0
		break
	return requiredHosts

    def traverseInnerPairs( self, numberOfHosts, requiredHosts, bytesToTransmit ):
	currentAvailableHostList = self.availableList
	print "inner %s" % currentAvailableHostList
	for j in xrange( numberOfHosts ):
	    if currentAvailableHostList[ j ][ 1 ] != 0:
		reducerHost = j
		reducerHostNumber = currentAvailableHostList[ j ][ 1 ] - 1
		for h in xrange( numberOfHosts - 1, 0, -1 ):
		    if currentAvailableHostList[ h ][ 0 ] != 0:
			if h == j:
			    print "same host"
			    break
			elif j > h:
			    print "wrap around"
			    break
			mapperHost = h
			mapperHostNumber = currentAvailableHostList[ h ][ 0 ] - 1
			requiredHosts = requiredHosts - 1
			self.manager.setIperf( reducerHost, reducerHostNumber, mapperHost, mapperHostNumber, bytesToTransmit )
			break
		    else:
			if h == 0:
			    j = 0
		break
	return requiredHosts

    def runTest( self ):
	if self.workQueue.empty() == True:
	    raise LookupError( "Running test on an empty queue" )
	currentJob = []
	numberOfHosts = self.numberOfHosts

	for i in xrange( self.workQueue.qsize() ):
	    currentAvailableHostList = self.availableList
	    currentJob = self.workQueue.get_nowait()
	    startTime = self.getTime()
	    requiredHosts = currentJob[ 0 ]
	    tempHosts = requiredHosts
	    totalRequiredHosts = currentJob[ 0 ]
	    forward = True
	    print currentJob

	    while requiredHosts > 0:
		if forward:
		    tempHosts = self.traverseOuterPairs( numberOfHosts, requiredHosts, currentJob[ 1 ] / totalRequiredHosts  )
		    forward = False
		else:
		    tempHosts = self.traverseInnerPairs( numberOfHosts, requiredHosts, currentJob[ 1 ] / totalRequiredHosts  )
		    forward = True
		self.hasListBeenUpdated = False
		if ( requiredHosts == tempHosts ):
		    print "WAITING"
		    while ( True ):
			if self.hasListBeenUpdated == True:
			    break
			time.sleep( 0.125 )
		else:
		    requiredHosts = tempHosts
		time.sleep( 0.125 )
	    time.sleep( 0.125 )

