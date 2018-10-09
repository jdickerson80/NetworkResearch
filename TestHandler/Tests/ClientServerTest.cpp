#include "ClientServerTest.h"

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
	unsigned int port = _testData->port ? _testData->port : 5001;
	_ipVector = ipVector;
	bool toReturn = true;
	pthread_t serverThread;

	Common::ThreadHelper::startJoinableThread( &serverThread, serverTest, static_cast< void* >( &port ) );

	if ( _testData->runInParallel )
	{
		size_t ipSize = ipVector->size();
		pthread_t threadPool[ ipSize ];
		ServerClientData serverClientData( _testData );
		size_t i = 0;

		for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it, ++i )
		{
			serverClientData.port = port;
			serverClientData.ipAddress = *(*it);
			Common::ThreadHelper::startJoinableThread( &threadPool[ i ], clientTest, static_cast< void* >( &serverClientData ) );
			++port;
			usleep( 125000 );
		}


		// done forking processes, time to collect them
		for ( size_t i = 0; i < ipSize; ++i )
		{
			pthread_join( threadPool[ i ], NULL );
		}

		pthread_join( serverThread, NULL );
	}
	else
	{
		ServerClientData serverClientData( _testData );
		for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it )
		{
			serverClientData.port = port;
			serverClientData.ipAddress = *(*it);
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

	PRINT("%s", buffer );
	sleep( 2 );
//	toReturn = system( buffer );
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
//	unsigned int* port = static_cast< unsigned int* >( input );
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -s -1 -p 5001\n" );
	PRINT( "%s", buffer );
//	toReturn = system( buffer );

	switch ( toReturn )
	{
	case -1:
	case 127:
		return NULL;
	default:
		return NULL;
	}
}

}
