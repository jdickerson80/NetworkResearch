#!/usr/bin/python

from enum import IntEnum
from MapReduceManager import MapReduceManager
from Queue import Queue
import time
import sys

class MapReduceScheduler( object ):
    class MegabytesPerHostEnum( IntEnum ):
	MegabytesPerHost = 64000000

    def __init__( self, hostList, workFile ):
	self.workQueue = Queue()
	self.__preprocessJobList( workFile )
	self.__processJobs( workFile )
	for i in xrange( self.workQueue.qsize() ):
	    print self.workQueue.get_nowait()
	print self.workQueue.qsize()
	self.manager = MapReduceManager( hostList, self.managerCallback )

    def terminate( self ):
	self.manager.terminate()

    @staticmethod
    def __preprocessJobList( workFile ):
	for job in workFile:
	    job = job.replace( "\n", "" )
	    job = job.replace( "job ", "" )
	    job = job.replace( " ", "" )
#	    print job

    def managerCallback( self, availableList ):
	pass

    def __processJobs( self, workFile ):
	inputBytes = 0
	perHostTransmitBytes = 0
	hostNeeded = 0
	for job in workFile:
	    jobComponents = job.split( "\t" )
	    if jobComponents[ 0 ] == '\n':
		continue
	    if int( jobComponents[ 2 ] ) == 0:
		continue

	    inputBytes = int( jobComponents[ 1 ] )

	    remainder = inputBytes % self.MegabytesPerHostEnum.MegabytesPerHost
	    hostsNeeded = inputBytes / self.MegabytesPerHostEnum.MegabytesPerHost
	    if remainder != 0 or hostsNeeded == 0:
		hostsNeeded = hostsNeeded + 1

#	    if hostsNeeded == 0:
#		print hostsNeeded
	    perHostTransmitBytes = int( jobComponents[ 2 ] ) / hostsNeeded
#	    print [ hostsNeeded, inputBytes, perHostTransmitBytes ]
#	    print hostNeeded
	    self.workQueue.put( [ hostsNeeded, perHostTransmitBytes ], block=False )

