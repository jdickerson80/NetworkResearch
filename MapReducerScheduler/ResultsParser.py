#!/usr/bin/python

import argparse
import os
import sys

from IperfResultsParser import *

def parseCommandLineArgument():
	parser = argparse.ArgumentParser( description="Results Parser" )

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

#	job = IperfResultsParser.getJobStatFromFile( arguments.directory )
	IperfResultsParser.logPerJobResults( arguments.directory )
