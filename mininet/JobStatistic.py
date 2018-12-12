#!/usr/bin/env python

class JobStatistic( object ):
	def __init__( self ):
		self.job = "Empty"
		self.startTime = 0
		self.endTime = 0
		self.hosts = []
		self.bytesToSend = None
		self.reduceJob = 0

	def __str__( self ):
		hosts = '[%s]' % ' '.join( map( str, self.hosts ) )
		return "%s, %s, %s, %s, %i, %i" % ( self.job, self.startTime, self.endTime, hosts, self.bytesToSend, self.reduceJob )
