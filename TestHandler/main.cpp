#include <stdio.h>
#include <unistd.h>

#include "MainObject.h"
#include "PrintUsage.h"

int main( int argc, char* argv[] )
{
	if ( argc < 4 )
	{
		TestHandler::printUsage();
		exit( EXIT_FAILURE );
	}

	bool badRun;
	TestHandler::MainObject* object = new TestHandler::MainObject( argc, argv );

	if ( !object->isRunning() )
	{
		TestHandler::printUsage();
		exit( EXIT_FAILURE );
	}

	badRun = object->runTests();

	delete object;

	exit( badRun ? EXIT_FAILURE : EXIT_SUCCESS );
}


