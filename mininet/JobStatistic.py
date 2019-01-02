#!/usr/bin/env python

class JobHost( object ):
	def __init__( self, hostName, hostIndex ):
		self.hostName = hostName
		self.hostIndex = hostIndex

	def __str__( self ):
		return "[ Host: %s Index: %s ]" % ( str( self.hostName ), str( self.hostIndex ) )

class JobStatistic( object ):
	def __init__( self ):
		self.bytesToSend	= None
		self.endTime 		= 0
		self.mapHostList	= []
		self.reduceHostList	= []
		self.job 			= "Empty"
		self.numberOfHosts 	= 0
		self.reduceJob 		= 0
		self.startTime 		= 0

	def __str__( self ):
		mapHosts = '[%s]' % ' '.join( map( str, self.mapHostList ) )
		reduceHosts = '[%s]' % ' '.join( map( str, self.reduceHostList ) )
		return "%s, %s, %s, %s, %s, %s, %s, %s" % ( self.job, self.numberOfHosts, self.startTime, self.endTime, mapHosts, reduceHosts, self.bytesToSend, self.reduceJob )
