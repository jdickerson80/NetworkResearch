#!/usr/bin/python

from FileConstants import *
from JobStatistic import *
import pickle
import json
import os

class JSONParser( object ):

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

	# @staticmethod
	# def deleteLinesInFile( file, lastLineToDelete ):
	# 	infile = open( file,'r' ).readlines()
	# 	with open( file,'w' ) as outfile:
	# 		for index,line in enumerate( infile ):
	# 			if index >= lastLineToDelete:
	# 				outfile.write(line)	

	@staticmethod
	def deleteLinesInFile( file, lastLineToDelete ):
		linesToWrite = []
		foundEnd = False
		fileObject = open( file,'r' )
		infile = fileObject.readlines()

		fileObject.close()
		count = 0
		for line in infile:
			if foundEnd == False and count > 0:
				if line[ 0 ] == '{':
					foundEnd = True
					linesToWrite.append( line )
			elif foundEnd == True:
				linesToWrite.append( line )
			count += 1

		with open( file,'w' ) as outfile:
			for i in linesToWrite:
				outfile.write( i )	

	@staticmethod
	def logIperfResults( jobDirectory, job ):
		directoryJobs = os.listdir( jobDirectory )
		
		for j in job:
			jobFolder = JSONParser.findJobDirectory( directoryJobs, j )
			sourceDirectory = os.path.join( jobDirectory, jobFolder ) 
			sourceDirectory += "/"
			fileList = os.listdir( sourceDirectory )
			
			for jobFile in fileList:
				try:
					with open( os.path.join( sourceDirectory, jobFile ), "r" ) as file:
						file.seek( 0 )
						char = file.read( 1 )
						if not char:
							print "file is empty" #first character is the empty string..
							continue
						else:
							file.seek( 0 ) #first character wasn't empty, return to start of file.

						try:
							jsonObject = json.load( file )
						except ValueError as e:
							# print "IperfResults %s %s" % ( jobFile, e )
							stringError = str( e )
							fileName = file.name

							start = stringError.find( "line " )
							start += 5
							end = stringError.find( "column " )
							end -= 1
 
							seekPosition = int( stringError[ start : end ] )
							file.close()
							JSONParser.deleteLinesInFile( fileName, seekPosition - 1 )
							print "file %s error %s %s " % ( jobFile, e, seekPosition )
							file = open( fileName, "r" )
							try:
								jsonObject = json.load( file )
								# print jsonObject
							except ValueError as e:
								print "gave up on file %s error %s" % ( jobFile, e )
								continue


						if jsonObject == None:
							print "object none"
							continue

						if JSONParser.fillIperfResults( j.receiveResults, jsonObject, 'sum_received' ) == False: 
							print "%s receive HAS ERROR" % j.job
						
						# SOMETHING WRONG WITH METHOD!!!!!!
						if JSONParser.fillIperfResults( j.sentResults, jsonObject, 'sum_sent' ) == False:
							print "%s sent HAS ERROR" % j.job

				except IOError as error:
					print "IOERROR %s" % error


	@staticmethod
	def logPerJobResults( jobDirectory ):
		directoryJobs = os.listdir( jobDirectory )
		results = []

		for job in directoryJobs:
			sourceDirectory = os.path.join( jobDirectory, job ) 
			if not os.path.isdir( sourceDirectory ):
				continue
			
			fileList = os.listdir( sourceDirectory )
			totalBandwidth = 0
			totalCompletionTime = 0
			perJobDataTransfer = 0
			totalHost = len( fileList )
			for jobFile in fileList:
				with open( os.path.join( sourceDirectory, jobFile ), "r" ) as file:
					try:
						jsonObject = json.load( file )
						try:
							totalBandwidth += jsonObject[ 'end' ][ 'sum_sent' ][ 'bits_per_second' ]	
							totalCompletionTime += jsonObject[ 'end' ][ 'sum_sent' ][ 'seconds' ]	
							perJobDataTransfer = jsonObject[ 'start' ][ 'test_start' ][ 'bytes' ]	
						except KeyError as k:
							print "logPerJobResults %s" % k
						
					except ValueError as e:
						print "logPerJobResults %s" % e
						continue


			results.append( [ job, totalHost, perJobDataTransfer, totalCompletionTime / totalHost, totalBandwidth / totalHost ] )

		with open('%sResults.csv' % jobDirectory, 'w' ) as f:
			f.write( "Job,Host,PerJobDataTransfer,Avg Completion Time,Avg Bandwidth (bits/sec),Avg Bandwidth (bytes/sec),CalculatedRate (bits/sec)\n")
			for r in results:
				try:
					f.write( "%s,%s,%s,%s,%s,%f,%f\n" % ( r[0], r[1], r[2], r[3], r[4], float( r[4] / 8.0 ), float( ( r[2] / r[3] ) * 8.0 ) ) )
				except ZeroDivisionError as z:
					print "%s" % z
					f.write( "%s,%s,%s,%s,%s,%f,%f\n" % ( r[0], r[1], r[2], r[3], r[4], float( r[4] / 8.0 ), 0 ) ) 

	
	@staticmethod
	def fillIperfResults( results, jsonObject, whatResults ):
		asdf = jsonObject[ 'start' ][ 'connected' ]
		if not asdf:
			return False
		byte = jsonObject[ 'end' ][ whatResults ][ 'bytes' ]
		duration = jsonObject[ 'end' ][ whatResults ][ 'seconds' ]

		if whatResults == 'sum_received':
			retransmit = None
		else:
			retransmit = jsonObject[ 'end' ][ 'sum_sent' ][ 'retransmits' ]	
		
		transmissionRate = jsonObject[ 'end' ][ whatResults ][ 'bits_per_second' ]
		results.append( IperfResults( byte, duration, retransmit, transmissionRate ) )
		return True

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