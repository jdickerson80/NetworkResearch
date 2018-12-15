#!/usr/bin/env python

import os
import re
import shutil

class FileConstants():
	hostBandwidthLogFile	= "BandwidthUsage.log"
	hostJobLogFile			= "FlowRates.csv"
	hostBaseDirectory 		= "/tmp/"

	@staticmethod
	def removeFiles( directory, pattern ):
		for f in os.listdir( directory ):
			if re.search( pattern, f ):
				while True:         
					try:
						os.remove( os.path.join( directory, f ) )
						break
					except os.error as error:
						print "remove files %s" % error       
					time.sleep( 0.025 )


	@staticmethod
	def copyFiles( sourceDirectory, destinationDirectory, pattern ):
		for f in os.listdir( sourceDirectory ):
			if re.search( pattern, f ):
				while True:         
					try:
						source = os.path.join( sourceDirectory, f )
						destination = os.path.join( destinationDirectory, f ) 
						shutil.copy2( source, destination )
						break
					except IOError as error:
						print "remove files %s" % error       
					time.sleep( 0.025 )