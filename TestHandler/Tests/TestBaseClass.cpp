#include "TestBaseClass.h"

#include "PrintHandler.h"

namespace TestHandler {

TestBaseClass::TestBaseClass( const TestData* const data, const TestBaseClass::Tests::Enum test )
	: _testData( data )
	, _test( test )
{

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
