#include "SingleServerTest.h"

#include <sys/wait.h>
#include <unistd.h>

#include "PrintHandler.h"

namespace TestHandler {

SingleServerTest::SingleServerTest( const TestData* const testData )
	: TestBaseClass( testData, Tests::SingleServer )
{

}

SingleServerTest::~SingleServerTest()
{

}

bool SingleServerTest::serverTest( const std::string& /*ipAddress*/, unsigned int port )
{
#define BufferSize ( 512 )
	char buffer[ BufferSize ];
	int toReturn = false;

	snprintf(
				buffer
				, 150
				, "iperf3 -i 1 -s -1 -p %i 2>&1 > /dev/null\n", port );

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

bool SingleServerTest::impl_runTest( TestBaseClass::IPVector* ipVector )
{
	pid_t pid;
	int status;
	_ipVector = ipVector;
	unsigned int port = 5001;

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
				return serverTest( *(*it), port );
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
			serverTest( *(*it), port );
			++port;
		}
	}

	system( _logStatsCommand.c_str() );
	return true;
}
}
