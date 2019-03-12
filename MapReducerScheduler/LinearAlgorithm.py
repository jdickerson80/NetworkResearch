#!/usr/bin/python

class LinearAlgorithm( object ):
	def __init__( self, startingX, endingX, startingY, endingY ):
		self.startingX = float( startingX )
		self.endingX = float( endingX )
		self.startingY = float( startingY )
		self.endingY = float( endingY )
		self.slope = ( self.endingX - self.endingY ) / ( self.startingX - self.startingY )
		self.yintercept = self.endingX - ( self.slope * self.startingX )

	def __str__( self ):
		return "slope %f yintercept %s start x %s ending x %s start y %s ending y %s" % ( self.slope, self.yintercept, self.startingX, self.endingX, self.startingY, self.endingY )

	def yFromX( self, x ):
		return ( x * self.slope ) + self.yintercept


if __name__ == '__main__':
	line = LinearAlgorithm( 4000, 20000, 0, 1000 )
	print line

	for i in xrange( 4000, 4500 ):
		print "%s %s" % ( i, line.yFromX( i ) )