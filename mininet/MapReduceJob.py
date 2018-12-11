#!/usr/bin/python

from mininet.node import *
from MapReduceScheduler import *
from MapReduceHost import *
from multiprocessing import Process
import time
import sys

class MapReduceJob( Process ):
	def __init__( self, schedulerPipe, hostMapReduceList, hostPipeList ):
		super( MapReduceJob, self ).__init__()
		self.schedulerPipe = schedulerPipe
		self.hostMapReduceList = hostMapReduceList
		self.hostPipeList = hostPipeList

	@staticmethod
	def getTime():
		return time.strftime( "%H:%M:%S", time.localtime() )

	def setIperfPair( self, hostOne, hostTwo, numberOfBytesToSend ):
		hostOneIP = self.hostMapReduceList[ hostOne ].getIP()
		hostTwoIP = self.hostMapReduceList[ hostTwo ].getIP()
		numberOfBytesToSend = numberOfBytesToSend / 2

		while True:
			poll = self.hostPipeList[ hostOne ].parentConnection.poll()

			if poll == True:
				message = self.hostPipeList[ hostOne ].parentConnection.recv()
				if message == [ 2, 2 ]:
					# print "host one got a 2,2"
					break
				# print "%i %s" % ( hostOne, message )
			else:
			# 	# print "Broke %i" % hostOne
				# print "host one got false"
				break

		while True:
			poll = self.hostPipeList[ hostTwo ].parentConnection.poll()

			if poll == True:
				message = self.hostPipeList[ hostTwo ].parentConnection.recv()
				if message == [ 2, 2 ]:
					# print "host two got a 2,2"
					break
				# print "%i %s" % ( hostTwo, message )
			else:
			# 	# print "Broke %i" % hostTwo
				# print "host one got false"
				break

		# self.hostMapReduceList[ hostOne ].addReducer( 0 )
		# time.sleep( 2 )
		self.hostMapReduceList[ hostTwo ].addMapper( 0, numberOfBytesToSend, hostOneIP )
		# time.sleep( 1 )

		# self.hostMapReduceList[ hostOne ].addReducer( 1 )
		# time.sleep( 2 )
		self.hostMapReduceList[ hostTwo ].addMapper( 1, numberOfBytesToSend, hostOneIP )
		# time.sleep( 1 )


		# self.hostMapReduceList[ hostTwo ].addReducer( 0 )
		# time.sleep( 2 )
		self.hostMapReduceList[ hostOne ].addMapper( 0, numberOfBytesToSend, hostTwoIP )
		# time.sleep( 1 )
		# self.hostMapReduceList[ hostTwo ].addReducer( 1 )
		# time.sleep( 2 )
		self.hostMapReduceList[ hostOne ].addMapper( 1, numberOfBytesToSend, hostTwoIP )
		# time.sleep( 1 )


	def run( self ):
		# jobStatistic = None

		while True:
			jobPipes = []
			receiveMessage = self.schedulerPipe.recv()
			jobStatistic = receiveMessage
			jobStatistic.startTime = self.getTime()

			for pair in xrange( 0, len( jobStatistic.hosts ), 2 ):
				# print pair
				jobPipes.append( self.hostPipeList[ pair ] )
				jobPipes.append( self.hostPipeList[ pair + 1 ] )
				self.setIperfPair( jobStatistic.hosts[ pair ], jobStatistic.hosts[ pair + 1 ], jobStatistic.bytesToSend )

			while True:
				count = 0
				for i in jobPipes:
					poll = i.parentConnection.poll()
					if poll == True:
						message = i.parentConnection.recv()
						# print message
						if message == [ 2, 2 ]:
							# print "got 2,2 %i" % count
							del jobPipes[ count ]
					count += 1

				# print len( jobPipes )
				if len( jobPipes ) == 0:
					# print "BROKE"
					break

				time.sleep( 0.020 )

			jobStatistic.endTime = self.getTime()
			# print jobStatistic
			# print "Run loop"
			self.schedulerPipe.send( jobStatistic )
