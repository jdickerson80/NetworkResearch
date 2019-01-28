#!/usr/bin/python

from enum import IntEnum

class HostStates( IntEnum ):
	Busy 		= 0
	Error 		= -1
	Ready		= 1
	Terminate	= 2

	def __str__( self ):
		return "%i" % ( int( self ) )
