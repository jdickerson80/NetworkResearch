#include "ClientServerTest.h"

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "PrintHandler.h"

namespace TestHandler {

ClientServerTest::ClientServerTest( const TestData* const testData )
	: TestBaseClass( testData, Tests::ClientServer )
{

}

ClientServerTest::~ClientServerTest()
{

}

bool ClientServerTest::impl_runTest( TestBaseClass::IPVector* ipVector )
{
	pid_t pid;
	int status;
	unsigned int port = 5001;
	_ipVector = ipVector;
	bool toReturn = true;

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
				toReturn |= clientTest( *(*it), port );
				toReturn |= serverTest( *(*it), port );
			}
			else
			{
				PRINT("Pushing pid %i\n", pid );
				_pidVector.push_back( pid );
			}

			++port;
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
			toReturn |= serverTest( *(*it), port );
			toReturn |= clientTest( *(*it), port );
			++port;
		}
	}

	return toReturn;
}

bool ClientServerTest::clientTest( const std::string& ipAddress, unsigned int port )
{
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -c %s %s %s %s %s %s %s -p %u\n"
				, ipAddress.c_str()
				, strcmp( _testData->targetBandwidth, "-" ) ? "-b" : ""
				, strcmp( _testData->targetBandwidth, "-" ) ? _testData->targetBandwidth : ""

				, strcmp( _testData->bytesToBeTransmitted, "-" ) ? "-n" : ""
				, strcmp( _testData->bytesToBeTransmitted, "-" ) ? _testData->bytesToBeTransmitted : ""

				, strcmp( _testData->duration, "-" ) ? "-t" : ""
				, strcmp( _testData->duration, "-" ) ? _testData->duration : ""
				, port );

	PRINT("%s", buffer );
	toReturn = system( buffer );

//	sleep( 2 );
	switch ( toReturn )
	{
	case -1:
	case 127:
		return true;
	default:
		return toReturn;
	}
}

bool ClientServerTest::serverTest( const std::string& /*ipAddress*/, unsigned int port )
{
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -s -1 -p %i\n", port );

	PRINT("%s", buffer );
	toReturn = system( buffer );

//	sleep( 2 );
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
