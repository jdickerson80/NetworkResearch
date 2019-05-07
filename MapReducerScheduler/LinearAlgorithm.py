#!/usr/bin/python

class Range( object ):
	def __init__( self, low, high ):
		self.low = float( low )
		self.high = float( high )
		self.range = self.high - self.low

class LinearAlgorithm( object ):
	def __init__( self, xRange, yRange ):
		self.numerator = yRange.range
		self.denominator = xRange.range
		self.yintercept = -xRange.low * self.numerator / self.denominator + yRange.low
		print yRange.range, xRange.range, self.yintercept

	def __str__( self ):
		return "numerator %f denominator %f y intercept %f" % ( self.numerator, self.denominator, self.yintercept )

	def yFromX( self, x ):
		return int( x * self.numerator / self.denominator + self.yintercept )


if __name__ == '__main__':
	line = LinearAlgorithm( Range( 4000, 20000 ), Range( 0, 1000 ) )
	print line

	for i in xrange( 4000, 5000 ):
		print "%i %f" % ( i, line.yFromX( i ) )