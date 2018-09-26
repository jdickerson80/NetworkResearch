#include "TestBaseClass.h"

#include <sstream>

#include "HelperMethods.h"
#include "PrintHandler.h"

namespace TestHandler {

TestBaseClass::TestBaseClass( const TestData* const data, const TestBaseClass::Tests::Enum test )
	: _testData( data )
	, _test( test )
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
}

bool TestBaseClass::runTest( TestBaseClass::IPVector* ipVector )
{
	return impl_runTest( ipVector );
}

TestBaseClass::Tests::Enum TestBaseClass::test() const
{
	return _test;
}

bool TestBaseClass::handleTerminatedSubprocess( int status, int pid )
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

}
