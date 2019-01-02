#!/usr/bin/python

from mininet.node import Host
import multiprocessing 
import time
from HostStates import *

class Reducer( multiprocessing.Process ):
	def __init__( self, connection, host, port ):
		multiprocessing.Process.__init__( self )
		self.connection = connection
		self.host 		= host
		self.port 		= port

	def startIperfServer( self ):
		pOpen = self.host.pexecNoWait( "iperf3 -s -p %s 2>&1 > /dev/null" % self.port )
		return pOpen
		
	def run( self ):
		
		pOpenObject = self.startIperfServer()

		# send ready
		self.connection.send( HostStates.Ready )

		# infinite loop
		while True:
			returnCode = pOpenObject.poll()

			if returnCode != None:
				out, err = pOpenObject.communicate()
				print "RESTARTED REDUCER %s code %s out %s err %s" % ( self.host, returnCode, out, err )
				pOpenObject = self.startIperfServer()

			poll = self.connection.poll()

			# if there is data to receive
			if poll == True:
				# wait for a message
				receiveMessage = self.connection.recv()

				if receiveMessage == HostStates.Terminate:
					# print "reducer got a terminate message"
					pOpenObject.kill()
					pOpenObject.wait()
					return
				# send busy command
				self.connection.send( receiveMessage )
			time.sleep( 0.125 )

			
def startIperfServer( host, port ):
	pOpen = host.pexecNoWait( "iperf3 -s -p %s" % port )
	# pOpen = self.host.pexecNoWait( "iperf3 -s -p %s 2>&1 > /dev/null" % self.port )
	return pOpen

def runReducer( connection, host, port ):
	pOpenObject = startIperfServer( host, port )

	# send ready
	connection.send( HostStates.Ready )

	# infinite loop
	while True:
		returnCode = pOpenObject.poll()

		if returnCode != None:
			out, err = pOpenObject.communicate()
			print "RESTARTED REDUCER %s code %s out %s err %s" % ( host, returnCode, out, err )
			pOpenObject = startIperfServer(host, port )

		poll = connection.poll()

		# if there is data to receive
		if poll == True:
			# wait for a message
			receiveMessage = connection.recv()

			if receiveMessage == HostStates.Terminate:
				# print "reducer got a terminate message"
				pOpenObject.kill()
				pOpenObject.wait()
				return
			# send busy command
			connection.send( receiveMessage )
		time.sleep( 0.125 )
