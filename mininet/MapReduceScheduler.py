#!/usr/bin/python

from enum import IntEnum
from MapReduceManager import MapReduceManager
from Queue import Queue
import time
import sys

class MapReduceScheduler( object ):
    class BytesPerHostEnum( IntEnum ):
        BytesPerHost = 64000000

    def __init__( self, hostList, workFile ):
        with open( workFile ) as f:
            data = f.readlines()

	self.workQueue = Queue()
        self.__preprocessJobList( data )
        self.__processJobs( data )

#	for i in xrange( self.workQueue.qsize() ):
#	    print self.workQueue.get_nowait()

        print "Size %i" % self.workQueue.qsize()
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

            self.workQueue.put( [ hostsNeeded, int ( jobComponents[ 2 ] ) ], block=False )









"""
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

                    remainder = inputBytes % self.BytesPerHostEnum.BytesPerHost
                    hostsNeeded = inputBytes / self.BytesPerHostEnum.BytesPerHost
                    if remainder != 0 or hostsNeeded == 0:
                        hostsNeeded = hostsNeeded + 1

                    perHostTransmitBytes = float( jobComponents[ 2 ] ) / float( hostsNeeded )
                    self.workQueue.put( [ hostsNeeded, perHostTransmitBytes ], block=False )
   """
