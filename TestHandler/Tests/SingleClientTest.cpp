#include "SingleClientTest.h"

#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "HelperMethods.h"
#include "PrintHandler.h"

namespace TestHandler {
SingleClientTest::SingleClientTest( const TestData* const testData )
	: TestBaseClass( testData, Tests::SingleClient )
{
	// stream the logging string, create and return the pointer to it
	std::ostringstream logPath;
	std::ostringstream logStatsCommand;
	std::ostringstream removeBandwidthStatsFile;

	logPath << "/tmp/" << Common::HelperMethods::getHostName() << "/";

	removeBandwidthStatsFile << "rm " << logPath.str() << "BandwidthUsage.log";

	logStatsCommand << "cp " << logPath.str() << "BandwidthUsage.log " << _testData->logPath << "\n";

	_logStatsCommand = logStatsCommand.str();
	_removeBandwidthStatsFile = removeBandwidthStatsFile.str();
	PRINT("construct %s\n", _logStatsCommand.c_str() );
}

SingleClientTest::~SingleClientTest()
{

}

bool SingleClientTest::impl_runTest( IPVector* ipVector )
{
	pid_t pid;
	int status;
	_ipVector = ipVector;
//	char commandBuffer[ 512 ];

//	time_t localTime = time(NULL);
//	tm timeStruct = *localtime( &localTime );

//	snprintf( commandBuffer, 512, "%s ""%s%02d_%02d_%04d%02d_%02d_%02d""\n"
//			  , _logStatsCommand.c_str()
//			  , _testData->logPath.c_str()
//			  , timeStruct.tm_mday
//			  , timeStruct.tm_mon+1
//			  , timeStruct.tm_year+1900
//			  , timeStruct.tm_hour
//			  , timeStruct.tm_min
//			  , timeStruct.tm_sec );

//	snprintf( commandBuffer, 512, "%s %s\n"
//			  , _logStatsCommand.c_str()
//			  , _testData->logPath.c_str() );

	system( _removeBandwidthStatsFile.c_str() );

	if ( _testData->runInParallel )
	{
		for ( IPVector::iterator it = ipVector->begin(); it < ipVector->end(); ++it )
		{
			// fork and check for error
			if ( ( pid = fork() ) == -1 )
			{
				PRINT("BAD FORK!!!!\n");
				break;
			}

			// is this the child
			if ( pid == 0 )
			{
				if ( clientTest( *(*it) ) )
				{
					exit( EXIT_FAILURE );
				}
				else
				{
					exit( EXIT_SUCCESS );
				}
			}
			else
			{
				PRINT("Pushing pid %i\n", pid );
				_pidVector.push_back( pid );
			}
		}

		// done forking processes, time to collect them
		for ( PIDVector::iterator it = _pidVector.begin(); it < _pidVector.end(); ++it )
		{
			// wait for a subprocess to complete
			pid_t pid = wait( &status );

			// close down the subprocess
			if ( handleTerminatedSubprocess( status, pid ) )
			{
				PRINT("PID %i\n", (*it) );
			}
			else
			{
				PRINT("BAD PID %i\n", (*it) );
			}
		}
	}
	else
	{
		for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it )
		{
			clientTest( *(*it) );
		}
	}

	system( _logStatsCommand.c_str() );
	return true;
}

bool SingleClientTest::handleTerminatedSubprocess(int status, int pid)
{
//	PRINT("EXIT %i\n", WEXITSTATUS( status ) );
	if ( WIFEXITED( status) && WEXITSTATUS( status ) != EXIT_SUCCESS )
	{
//		PRINT( "A process did not complete successfully\n" );
		return false;
	}

//	PRINT("Got pid %i\n", pid );
	// if this pid has been collected
	if ( pid <= 0 )
	{
		// just return
		return false;
	}

	return true;
}

bool SingleClientTest::clientTest( const std::string& ipAddress )
{
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -c %s %s %s %s %s %s %s 2>&1 > /dev/null\n"
				, ipAddress.c_str()
				, strcmp( _testData->targetBandwidth, "-" ) ? "-b" : ""
				, strcmp( _testData->targetBandwidth, "-" ) ? _testData->targetBandwidth : ""

				, strcmp( _testData->bytesToBeTransmitted, "-" ) ? "-n" : ""
				, strcmp( _testData->bytesToBeTransmitted, "-" ) ? _testData->bytesToBeTransmitted : ""

				, strcmp( _testData->duration, "-" ) ? "-t" : ""
				, strcmp( _testData->duration, "-" ) ? _testData->duration : "" );

//	PRINT( "%s", buffer);

	toReturn = system( buffer );

	sleep( 20 );
	switch ( toReturn )
	{
	case -1:
	case 127:
		return true;
	default:
		return toReturn;
	}
}
}
