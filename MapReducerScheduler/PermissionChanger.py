#!/usr/bin/python

import argparse
import os
import sys
import time
import re

def fixOutputFilePermissions( outputDirectory ):
	os.system( "chmod 777 -R %s" % outputDirectory )
		
	for f in os.listdir( outputDirectory ):
		# if re.search( "*.csv", f ) or re.search( "*.dat", f ):
		if re.search( ".csv", f ):
			os.system( "chmod 666 %s/%s" % ( outputDirectory, f ) )

def parseCommandLineArgument():
	parser = argparse.ArgumentParser( description="Permission Changer" )

	parser.add_argument('--directory', '-d',
						action="store",
						help="Directory",
						default=None)

	args = parser.parse_args()
	return args

if __name__ == '__main__':
	if os.getuid() != 0:
		print "You are NOT root"
		sys.exit()
	arguments = parseCommandLineArgument()

	fixOutputFilePermissions( arguments.directory )
