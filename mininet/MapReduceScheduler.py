#!/usr/bin/python

from enum import IntEnum
import threading
from MapReduceManager import MapReduceManager
from Queue import Queue
import time
import sys
import time

class MapReduceScheduler( object ):
    class Statistics( object ):
        def __init__( self ):
            self.job = "Empty"
            self.startTime = 0
            self.endTime = 0
            self.hosts = []

        def __str__( self ):
            hosts = '[%s]' % ', '.join( map( str, self.hosts ) )
            return "%s, %s, %s, %s" % ( self.job, self.startTime, self.endTime, hosts )

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
        self.jobStats = []
        self.logger = threading.Thread( target=self.handleJobLog )

	print "Queue size: %i" % self.workQueue.qsize()
	self.manager = MapReduceManager( hostList, self.managerCallback )
	self.hasListBeenUpdated = False


    def startLoggerThread( self ):
        self.loggerRunning = True
        self.logger.start()

    def terminate( self ):
        self.manager.terminate()
        if self.loggerRunning == True:
            self.loggerRunning = False
            self.logger.join()

    def handleJobLog( self ):
        beginningJobCounter = 0
        hostList = []
        time.sleep( 1 )
        print "JobLogRunning"

        while self.loggerRunning == True:
#            if len( self.jobStats ) == beginningJobCounter + 1:
#                print "BROKE"
#                break
            while self.hasListBeenUpdated == False:
#                print "sleeeppy"
                time.sleep( 0.025 )
            for i in xrange( beginningJobCounter, len( self.jobStats ) ):
                if self.jobStats[ i ].endTime == 0:
                    hostList = self.jobStats[ i ].hosts
                    listComplete = True
                    for host in hostList:
                        if self.availableList[ host - 1 ][ 0 ] != 2 and self.availableList[ host - 1 ][ 1 ]:
                            listComplete = False
                            break
                    if listComplete == True:
                        self.jobStats[ i ].endTime = self.getTime()
                        beginningJobCounter = i


            time.sleep( 0.025 )

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

            self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ), jobComponents[ 0 ] ], block=False )


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


    def dualPairs( self, numberOfHosts, requiredHosts, bytesToTransmit, counter ):
	currentAvailableHostList = self.availableList
#	print "test %s" % currentAvailableHostList
	for j in xrange( numberOfHosts ):
	    if currentAvailableHostList[ j ][ 0 ] == 2:
		mapperHost = j
		for h in xrange( numberOfHosts - 1, 0, -1 ):
		    if currentAvailableHostList[ h ][ 1 ] == 2:
			if h == j:
			    print "same host"
			    break
			elif j > h:
			    print "wrap around"
			    break
			reducerHost = h
			requiredHosts = requiredHosts - 1
                        self.jobStats[ counter ].hosts.append( reducerHost + 1 )
                        self.jobStats[ counter ].hosts.append( mapperHost + 1 )
			self.manager.setIperfPair( reducerHost, mapperHost, bytesToTransmit )
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
        counter = 0
        self.startLoggerThread()

	for i in xrange( self.workQueue.qsize() ):
	    currentAvailableHostList = self.availableList
	    currentJob = self.workQueue.get_nowait()
	    startTime = self.getTime()
	    requiredHosts = currentJob[ 0 ]
	    tempHosts = requiredHosts
	    totalRequiredHosts = currentJob[ 0 ]
	    forward = True
#	    print currentJob
            self.jobStats.append( self.Statistics() )
            self.jobStats[ counter ].job = currentJob[ 2 ]
            self.jobStats[ counter ].startTime = self.getTime()
#            print self.jobStats[ counter ]
	    while requiredHosts > 0:
                tempHosts = self.dualPairs( numberOfHosts, requiredHosts, currentJob[ 1 ] / totalRequiredHosts, counter )
#		if forward:
#		    tempHosts = self.traverseOuterPairs( numberOfHosts, requiredHosts, currentJob[ 1 ] / totalRequiredHosts  )
#		    forward = False
#		else:
#		    tempHosts = self.traverseInnerPairs( numberOfHosts, requiredHosts, currentJob[ 1 ] / totalRequiredHosts  )
#		    forward = True
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
            counter = counter + 1

        with open('jobLog%s.txt' % self.getTime(), 'w') as f:
            for item in self.jobStats:
                f.write("%s\n" % item)

