#include "ClientServerTest.h"

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "PrintHandler.h"
#include "ThreadHelper.h"

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
	unsigned int port = _testData->port ? _testData->port : 5001;
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
				pthread_t clientThread;
				pthread_t serverThread;
				ServerClientData serverClientData( _testData );
				serverClientData.port = port;
				serverClientData.ipAddress = *(*it);
				Common::ThreadHelper::startJoinableThread( &clientThread, clientTest, static_cast< void* >( &serverClientData ) );
				Common::ThreadHelper::startJoinableThread( &serverThread, serverTest, static_cast< void* >( &serverClientData ) );
				pthread_join( clientThread, NULL );
				pthread_join( serverThread, NULL );
				return toReturn;
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
			ServerClientData serverClientData( _testData );
			serverClientData.port = port;
			serverClientData.ipAddress = *(*it);
			serverTest( &serverClientData );
			clientTest( &serverClientData );
			++port;
		}
	}

	return toReturn;
}

void* ClientServerTest::clientTest( void* input )
{
	ServerClientData* serverClientData = static_cast< ServerClientData* >( input );
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -c %s %s %s %s %s %s %s -p %u\n"
				, serverClientData->ipAddress.c_str()
				, strcmp( serverClientData->testData->targetBandwidth, "-" ) ? "-b" : ""
				, strcmp( serverClientData->testData->targetBandwidth, "-" ) ? serverClientData->testData->targetBandwidth : ""

				, strcmp( serverClientData->testData->bytesToBeTransmitted, "-" ) ? "-n" : ""
				, strcmp( serverClientData->testData->bytesToBeTransmitted, "-" ) ? serverClientData->testData->bytesToBeTransmitted : ""

				, strcmp( serverClientData->testData->duration, "-" ) ? "-t" : ""
				, strcmp( serverClientData->testData->duration, "-" ) ? serverClientData->testData->duration : ""
				, serverClientData->port );

//	toReturn = system( buffer );

	sleep( 4 );
	switch ( toReturn )
	{
	case -1:
	case 127:
		pthread_exit( NULL );
		return NULL;
	default:
		pthread_exit( NULL );
		return NULL;
	}
}

void* ClientServerTest::serverTest( void* input )
{
	ServerClientData* serverClientData = static_cast< ServerClientData* >( input );
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -s -1 -p %i\n", serverClientData->port );

//	toReturn = system( buffer );

	sleep( 4 );
	switch ( toReturn )
	{
	case -1:
	case 127:
		pthread_exit( NULL );
		return NULL;
	default:
		pthread_exit( NULL );
		return NULL;
	}
}

}
