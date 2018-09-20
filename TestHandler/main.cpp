#include <stdio.h>
#include <unistd.h>

#include "MainObject.h"
#include "PrintUsage.h"

using namespace TestHandler;

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
		printUsage();
		exit( EXIT_FAILURE );
	}

	MainObject& object = MainObject::instance();

	if ( !object.parseCommandLineArguments( argc, argv ) )
	{
		printUsage();
		exit( EXIT_FAILURE );
	}

//	// do nothing loop to keep the app going
//	while ( object.isRunning() )
//	{
//		sleep( 1 );
//	}

	object.setRunning( false );
	exit( EXIT_SUCCESS );
}


