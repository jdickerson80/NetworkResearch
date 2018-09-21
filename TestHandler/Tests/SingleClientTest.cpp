#include "SingleClientTest.h"

#include <stdlib.h>

#include "PrintHandler.h"

namespace TestHandler {
SingleClientTest::SingleClientTest( const TestData* const testData )
	: TestBaseClass( testData, Tests::SingleClient )
{

}

SingleClientTest::~SingleClientTest()
{

}

bool SingleClientTest::impl_runTest( IPVector* ipVector )
{
	_ipVector = ipVector;

	Common::ThreadHelper::startJoinableThread( &_thread
											   , clientTest
											   , static_cast< void* >( this ) );

	pthread_join( _thread, NULL );
	return true;
}

void* SingleClientTest::clientTest( void* input )
{
#define BufferSize ( 1024 )
	SingleClientTest* singleClientTest = static_cast< SingleClientTest* >( input );
	const TestData* const testData = singleClientTest->_testData;
	IPVector* ipVector = singleClientTest->_ipVector;

	char buffer[ BufferSize ];

	for ( IPVector::iterator it = ipVector->begin(); it != ipVector->end(); ++it )
	{
		//   -b	val -t val -n val
		snprintf(
					buffer
					, 20
					, "iperf3 -i 1 -c %s %s %c %s %c %s %c\n"
					, (*it)
					, testData->targetBandwidth ? " " : "-b"
					, testData->targetBandwidth ? ' ' : testData->targetBandwidth + '0'
					, testData->duration ? " " : "-t"
					, testData->duration ? ' ' : testData->duration + '0'
					, testData->bytesToBeTransmitted ? " " : "-n"
					, testData->bytesToBeTransmitted ? ' ' : testData->bytesToBeTransmitted + '0');
//		system( buffer );

		PRINT( "%s", buffer)
	}

	pthread_exit( NULL );
	return NULL;
}
}
