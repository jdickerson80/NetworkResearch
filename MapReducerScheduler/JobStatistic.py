#!/usr/bin/env python

class JobHost( object ):
	def __init__( self, hostName, hostIndex ):
		self.hostName = hostName
		self.hostIndex = hostIndex

	def __str__( self ):
		# return "%s:%s, " % ( str( self.hostName ), str( self.hostIndex ) )
		return "[%s:%s]" % ( str( self.hostName ), str( self.hostIndex ) )

class IperfResults( object ):
	def __init__( self ):
		self.bytes 			  = 0
		self.duration		  = 0
		self.retransmit		  = 0
		self.transmissionRate = 0

	def __init__( self, byte, duration, retransmit, transmissionRate ):
		self.bytes 			  = byte
		self.duration		  = duration
		self.retransmit		  = retransmit
		self.transmissionRate = transmissionRate

	def __str__( self ):
		return "[%s %s %s %s]" % ( str( self.bytes ), str( self.duration ), str( self.retransmit ), str( self.transmissionRate ) )

class JobStatistic( object ):
	def __init__( self ):
		self.bytesToSend	= None
		self.endTime 		= 0
		self.failedHostList = []
		self.mapHostList	= []
		self.reduceHostList	= []
		self.receiveResults	= []
		self.sentResults	= []
		self.job 			= "Empty"
		self.numberOfHosts 	= 0
		self.reduceJob 		= 0
		self.startTime 		= 0
		self.errorCounts	= 0

	def __str__( self ):
		mapHosts 		= '[%s]' % ' '.join( map( str, self.mapHostList ) )
		reduceHosts 	= '[%s]' % ' '.join( map( str, self.reduceHostList ) )
		receiveResults 	= '[%s]' % ' '.join( map( str, self.receiveResults ) )
		sentResults 	= '[%s]' % ' '.join( map( str, self.sentResults ) )
		failedHostList 	= '[%s]' % ' '.join( map( str, self.failedHostList ) )

		# return "%s, %s, %s\n%s\n%s\n\n" % ( self.job, self.errorCounts, failedHostList, mapHosts, reduceHosts )
		return "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s" % ( self.job, self.numberOfHosts, self.startTime, self.endTime, mapHosts, reduceHosts, self.errorCounts, failedHostList, self.bytesToSend, receiveResults, sentResults )

