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
	return time.strftime( "%H:%M:%S", time.localtime() )

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

#	    print hostsNeeded, jobComponents[ 2 ]
	    self.workQueue.put( [ int ( hostsNeeded ), int ( jobComponents[ 2 ] ) ], block=False )


    def managerCallback( self, availableList ):
	self.availableList = availableList
	self.hasListBeenUpdated = True
#	print "CALLBACK %i" % ( len( self.availableList ) - 1 )


    def runTest( self ):
	if self.workQueue.empty() == True:
	    raise LookupError( "Running test on an empty queue" )
	beginningHost = 0
	endHost = self.numberOfHosts - 1
	currentJob = []
	jobHosts = []
#	jobStartTime = []
	hostsLeft = self.numberOfHosts
	self.jobList = []
	jobCount = 0
	numberOfHosts = self.numberOfHosts

	for i in xrange( self.workQueue.qsize() ):
	    currentAvailableHostList = self.availableList
	    currentJob = self.workQueue.get_nowait()
	    startTime = self.getTime()
	    requiredHosts = currentJob[ 0 ]
	    totalRequiredHosts = currentJob[ 0 ]
	    hostsUsed = 0
	    availableHosts = 0
	    print currentJob
	    while requiredHosts > 0:
		currentAvailableHostList = self.availableList
		print currentAvailableHostList
		for j in xrange( numberOfHosts ):
		    if currentAvailableHostList[ j ] != 0:
			beginningHost = j
			beginningHostNumber = currentAvailableHostList[ j ] - 1
#			print "j is %i" % j
			for h in xrange( self.numberOfHosts - 1, 0, -1 ):
#			    print "h is %i" % h
			    if currentAvailableHostList[ h ] != 0:
				if h == j:
#				    print "same host"
				    break
				endHost = h
				endHostNumber = currentAvailableHostList[ h ] - 1
				requiredHosts = requiredHosts - 1
#				def setIperf( self, hostOne, hostOneMapper, hostOneReducer, hostTwo, hostTwoMapper, hostTwoReducer, numberOfBytesToSend ):
				self.manager.setIperf1( beginningHost, beginningHostNumber, endHost, endHostNumber, currentJob[ 1 ] / totalRequiredHosts )
#				print "beg %i end %i left %i" % ( beginningHost, endHost, requiredHosts )
				break
			    else:
				if h == 0:
				    j = 0
#				time.sleep( 1 )
			break

		time.sleep( 0.125 )
	    time.sleep( 0.125 )





#	    def runTest( self ):
#		if self.workQueue.empty() == True:
#		    raise LookupError( "Running test on an empty queue" )
#		beginningHost = 0
#		endHost = self.numberOfHosts - 1
#		currentJob = []
#		jobHosts = []
#	#	jobStartTime = []
#		hostsLeft = self.numberOfHosts
#		self.jobList = []
#		jobCount = 0
#		numberOfHosts = self.numberOfHosts

#		for i in xrange( self.workQueue.qsize() ):
#		    currentAvailableHostList = self.availableList
#		    currentJob = self.workQueue.get_nowait()
#		    startTime = self.getTime()
#		    requiredHosts = currentJob[ 0 ]
#		    totalRequiredHosts = currentJob[ 0 ]
#		    hostsUsed = 0
#		    availableHosts = 0
#		    print currentJob
#		    while requiredHosts > 0:
#			currentAvailableHostList = self.availableList
#	#		print currentAvailableHostList
#			for j in xrange( numberOfHosts ):
#			    if currentAvailableHostList[ j ] != 0:
#				beginningHost = j
#	#			print "j is %i" % j
#				for h in xrange( self.numberOfHosts - 1, 0, -1 ):
#	#			    print "h is %i" % h
#				    if currentAvailableHostList[ h ] != 0:
#					if h == j:
#	#				    print "same host"
#					    break
#					endHost = h
#					requiredHosts = requiredHosts - 1
#					self.manager.setIperfPair( beginningHost, endHost, currentJob[ 1 ] / totalRequiredHosts )
#	#				print "beg %i end %i left %i" % ( beginningHost, endHost, requiredHosts )
#					break
#				    else:
#					if h == 0:
#					    j = 0
#	#				time.sleep( 1 )
#				break

#			time.sleep( 0.125 )
#		    time.sleep( 0.125 )
#		# send the hosts pair and the number of bytes to be sent
#		self.manager.setIperfPair( beginningHost, endHost, currentJob[ 1 ] / requiredHosts )
#		beginningHost = beginningHost + 1
#		endHost = endHost - 1
#		hostsLeft = hostsLeft - 2
#		print self.availableList



#		hostsLeft = len( self.availableList ) - 1
##		pass
##		print j
##		if hostsLeft < requiredHosts:
##		    time.sleep( 0.125 )
##		# send the hosts pair and the number of bytes to be sent
#		self.manager.setIperfPair( beginningHost, endHost, currentJob[ 1 ] / requiredHosts )
#		beginningHost = beginningHost + 1
#		endHost = endHost - 1
##		hostsLeft = hostsLeft - 2
#		print "beg %i end %i left %i" % ( beginningHost, endHost, hostsLeft )
#		print self.availableList
#		time.sleep( 1 )
