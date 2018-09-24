#include "SingleClientTest.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "PrintHandler.h"

namespace TestHandler {
SingleClientTest::SingleClientTest( const TestData* const testData )
	: TestBaseClass( testData, Tests::SingleClient )
{
	PRINT("constructing\n");
}

SingleClientTest::~SingleClientTest()
{

}

bool SingleClientTest::impl_runTest( IPVector* ipVector )
{
	pid_t pid;
	int status;
	_ipVector = ipVector;

	if ( _testData->runInParallel )
	{
		for ( IPVector::iterator it = ipVector->begin(); it < ipVector->end(); ++it )
		{
			// fork and check for error
			if ( ( pid = fork() ) == -1 )
			{
				break;
			}

			// is this the child
			if ( pid == 0 )
			{
				clientTest( *(*it) );
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
			handleTerminatedSubprocess( status, pid );
		}
	}
	else
	{
		for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it )
		{
			clientTest( *(*it) );
		}
	}
	return true;
}

bool SingleClientTest::handleTerminatedSubprocess(int status, int pid)
{
	if ( WIFEXITED( status) && WEXITSTATUS( status ) != EXIT_SUCCESS )
	{
		fprintf( stderr, "A process did not complete successfully\n" );
		return true;
	}

	PRINT("Got pid %i\n", pid );
	// if this pid has been collected
	if ( pid <= 0 )
	{
		// just return
		return false;
	}
}
/*
bool SingleClientTest::impl_runTest( IPVector* ipVector )
{
	_ipVector = ipVector;

	if ( _testData->runInParallel )
	{
		Common::ThreadHelper::startJoinableThread( &_thread
												   , clientTest
												   , static_cast< void* >( _testData ) );
		pthread_join( _thread, NULL );
	}
	else
	{
		for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it )
		clientTest( static_cast< void* >( this ) );
	}
	return true;
}
*/
bool SingleClientTest::clientTest( const std::string& ipAddress )
{
#define BufferSize ( 512 )
	char buffer[ BufferSize ];

//	PRINT("Testing\n");

	//   -b	val -t val -n val
	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -c %s -b %i 2>&1 >/dev/null\n"
				, ipAddress.c_str()
				, _testData->targetBandwidth );


//	, _testData->targetBandwidth ? " " : "-b"
//	, _testData->targetBandwidth ? ' ' : _testData->targetBandwidth + '0'
//	, _testData->duration ? " " : "-t"
//	, _testData->duration ? ' ' : _testData->duration + '0'
//	, _testData->bytesToBeTransmitted ? " " : "-n"
//	, _testData->bytesToBeTransmitted ? ' ' : _testData->bytesToBeTransmitted + '0');
//	system( buffer );

	PRINT( "%s", buffer);

	return false;

}
}
