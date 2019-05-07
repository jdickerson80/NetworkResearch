#!/usr/bin/python

from mininet.node import Host
import threading 
import time

class ReducerManager( threading.Thread ):

	_iperfCommand = "iperf3 -s -p %s -f M 2>&1 > /dev/null"
	def __init__( self, hostList ):
		super( ReducerManager, self ).__init__()
		self.hostList 			= hostList	
		self.handleReducersThreadRunning = True
		self.reducerList		= []
		self.killList			= []

	def terminate( self ):
		self.handleReducersThreadRunning = False
		for i in self.reducerList:
			i[ 0 ].kill()
			i[ 0 ].wait()
			i[ 1 ].kill()
			i[ 1 ].wait()

	def startAllReducers( self ):
		for i in xrange( len( self.hostList ) - 1 ):
			one = self.hostList[ i ].pexecNoWait( self._iperfCommand % 5001 )
			two = self.hostList[ i ].pexecNoWait( self._iperfCommand % 5002 )
			self.reducerList.append( [ one, two ] )	

	def startIperfServer( self ):
		pOpen = self.host.pexecNoWait( self._iperfCommand % self.port )
		return pOpen
		
	def run( self ):
		super( ReducerManager, self ).run()
		
		self.startAllReducers()
		# print self.reducerList
		# infinite loop
		while self.handleReducersThreadRunning == True:
			# clear the count variable
			count = 0
			# loop through the job pipes
			for i in self.reducerList:
				# get the poll value
				poll = i[ 0 ].poll()
				# if there is data
				if poll != None:
					self.reducerList[ count ][ 0 ] = self.hostList[ count ].pexecNoWait( self._iperfCommand % 5001 )

				poll = i[ 1 ].poll()
				# if there is data
				if poll != None:
					self.reducerList[ count ][ 1 ] = self.hostList[ count ].pexecNoWait( self._iperfCommand % 5002 )
					
				count += 1

			count = 0
			# print self.killList
			for i in self.killList:
				self.reducerList[ i[ 0 ] ][ i[ 1 ] ].kill()
				# self.reducerList[ i[ 0 ] ][ i[ 1 ] ].communicate( input=None )
				self.reducerList[ i[ 0 ] ][ i[ 1 ] ].wait()
				time.sleep( 0.25 )
				if i[ 1 ] % 2 == 0:
					port = 5002
				else:
					port = 5001

				self.reducerList[ i[ 0 ] ][ i[ 1 ] ] = self.hostList[ i[ 0 ] ].pexecNoWait( self._iperfCommand % port )
				# print self.reducerList[ i[ 0 ] ][ i[ 1 ] ]
				# print "killing and restarting %s %s" % ( self.hostList[ i[ 0 ] ], i[ 1 ] )
				time.sleep( 0.0025 )
				del( self.killList[ count ] )
				count += 1

			time.sleep( 0.025 )

	def killAndRestartReducer( self, host, index ):
		self.killList.append( [ host, index ] )
		# print self.killList
