#include "MainObject.h"

#include <stdio.h>
#include <signal.h>
#include <string>
#include <unistd.h>

#include "TestData.h"
#include "CommandLineArgumentParser.h"
#include "PrintHandler.h"
#include "PrintUsage.h"
#include "Tests/TestBaseClass.h"

static bool _isRunning = true;

namespace TestHandler {

MainObject::MainObject()
	: _ipVector()
	, _testToRun()
{
	_testData = new TestData();
	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );
	setlocale( LC_ALL, "" );
}

MainObject::~MainObject()
{
	deleteIPVector();
	deleteTestVector();
	delete _testData;
}

MainObject& MainObject::instance()
{
	static MainObject instance;
	return instance;
}

void MainObject::setRunning( bool isRunning )
{
	_isRunning = isRunning;
}

bool MainObject::isRunning() const
{
	return _isRunning;
}

bool MainObject::parseCommandLineArguments( int argc, char* const* argv )
{
	CommandLineArgumentParser parser;

	if ( !parser.parseCommandLineArguments( argc, argv, _testData, _ipVector, _testToRun ) )
	{
		printUsage();
		return false;
	}

	return true;
}

void MainObject::signalHandler( int signal )
{
	switch ( signal )
	{
	case SIGINT:
		printf( "Caught Ctrl + C\n" );
		_isRunning = false;
		break;

	case SIGTERM:
		printf( "Caught Terminate\n" );
		_isRunning = false;
		break;

	default:
		printf( "In default signal\n" );
		break;
	}
}

void MainObject::deleteIPVector()
{
	for ( size_t i = _ipVector.size(); i > 0; --i )
	{
		delete _ipVector[ i ];
	}
}

void MainObject::deleteTestVector()
{
	for ( size_t i = _testToRun.size(); i > 0; --i )
	{
		delete _testToRun[ i ];
	}
}

}
