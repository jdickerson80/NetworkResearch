#include <stdio.h>
#include <unistd.h>

#include "MainObject.h"
#include "PrintUsage.h"

int main( int argc, char* argv[] )
{
	// check if the app is running as root
//	if ( getuid() != 0 )
//	{
//		printf("You must run this program as root. Exiting.\n");
//		exit( EXIT_FAILURE );
//	}

	if ( argc < 4 )
	{
		TestHandler::printUsage();
		exit( EXIT_FAILURE );
	}

	TestHandler::MainObject object( argc, argv );

	if ( !object.isRunning() )
	{
		TestHandler::printUsage();
		exit( EXIT_FAILURE );
	}

	object.runTests();
	// do nothing loop to keep the app going
	while ( object.isRunning() )
	{
		sleep( 1 );
	}

	object.setRunning( false );
	exit( EXIT_SUCCESS );
}


