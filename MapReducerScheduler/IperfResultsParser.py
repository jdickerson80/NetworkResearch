#!/usr/bin/python

from FileConstants import *
from JobStatistic import *
import pickle
import json
import os

class IperfResultsParser( object ):

	@staticmethod	
	def findJobDirectory( jobDirectoryList, job ):
		for j in jobDirectoryList:
			if j == job.job:
				return j
		return None

	@staticmethod
	def storeJobStatInFile( filePath, data ):
		path = filePath + "/PickledJobs.dat"
		with open( path, "wb" ) as file:
			pickle.dump( data, file )

	@staticmethod
	def getJobStatFromFile( filePath ):
		path = filePath + "/PickledJobs.dat"
		with open( path, "r" ) as file:
			job = pickle.load( file )			
		return job

	@staticmethod
	def findJob( jobToFind, jobList ):
		# print jobToFind, jobList
		for job in jobList:
			if job.job == jobToFind:
				return job	

	_bitrateOffsetFromEnd = 20
	@staticmethod
	def logPerJobResults( jobDirectory ):
		directoryJobs = os.listdir( jobDirectory )
		results = []
		jobList = IperfResultsParser.getJobStatFromFile( jobDirectory )
	
		for job in directoryJobs:
			sourceDirectory = os.path.join( jobDirectory, job ) 
			if not os.path.isdir( sourceDirectory ):
				continue
			
			fileList = os.listdir( sourceDirectory )
			totalBandwidth = 0 
			totalCompletionTime = 0
			perJobDataTransfer = 0
			totalHost = len( fileList )
			outputInKila = False
			jobStat = IperfResultsParser.findJob( job, jobList )
			perJobDataTransfer = jobStat.bytesToSend * 8
			iperfBitRate = jobStat.bitrate
			
			for jobFile in fileList:
				with open( os.path.join( sourceDirectory, jobFile ), "r" ) as file:
					try:
						data = file.readlines()
						outputLine = data[ len( data ) - 4 ]
						outputLine = outputLine.split( " " )
		
						temp = float( outputLine[ len( outputLine ) - IperfResultsParser._bitrateOffsetFromEnd  ] )
						totalBandwidth += temp

					except ValueError as e:
						print "logPerJobResults %s" % e
						continue

					except IndexError as e:
						print "logPerJobResults %s" % e
						continue

			results.append( [ job, totalHost, perJobDataTransfer, totalBandwidth / totalHost, iperfBitRate ] )

		filename = jobDirectory[ : len( jobDirectory ) - 1]
		print filename	
		with open('%sResultsOf%s.csv' % ( jobDirectory, filename ), 'w' ) as f:
			f.write( "Job,Host,PerJobDataTransfer (Bits),Avg Bandwidth (Mbits/sec), Iperf Bitrate\n")
			for r in results:
				try:
					f.write( "%s,%s,%s,%s,%s\n" % ( r[0], r[1], r[2], r[3] * 8, r[4] ) )
				except ZeroDivisionError as z:
					print "%s" % z
					f.write( "%s,%s,%s,%s,%s,%f,%f\n" % ( r[0], r[1], r[2], r[3], r[4], float( r[4] / 8.0 ), 0 ) ) 

	@staticmethod
	def logTestResults( jobStats, outputDirectory, hostList ):
		for item in jobStats:
			jobDirectory = "%s%s/" % ( outputDirectory, item.job )
			# print jobDirectory
			if not os.path.exists( jobDirectory ):
				os.mkdir( jobDirectory )

			for host in hostList:
				hostDirectory = "%s%s/" % ( FileConstants.hostBaseDirectory, host )
				JSONParser.copyJobLogs( hostDirectory, jobDirectory, item )

	@staticmethod
	def copyJobLogs( hostDirectory, jobDirectory, job ):
		for f in os.listdir( hostDirectory ):
			end = 0
			start = f.find( job.job )
			if start == -1:
				continue

			start += 3

			end = start
			while ( f[ end ] != 'P' ):
				end += 1

			jobNumber = f[ start : end ]

			if jobNumber == job.job[ 3 : ]:
				while True:         
					try:
						source = os.path.join( hostDirectory, f )
						destination = os.path.join( jobDirectory, f ) 
						shutil.copy2( source, destination )
						break
					except IOError as error:
						print "remove files %s" % error  


					time.sleep( 0.025 )