#include "MainObject.h"

#include <stdio.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <unistd.h>

#include "HelperMethods.h"
#include "TestData.h"
#include "CommandLineArgumentParser.h"
#include "PrintHandler.h"
#include "Tests/TestBaseClass.h"

static bool _isRunning = true;

namespace TestHandler {

MainObject::MainObject( int argc, char* const* argv )
	: _ipVector()
	, _testToRun()
{
	_testData = new TestData();
	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );
	setlocale( LC_ALL, "" );

	_isRunning = parseCommandLineArguments( argc, argv );

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

MainObject::~MainObject()
{
	deleteIPVector();
	deleteTestVector();
	delete _testData;
}

void MainObject::setRunning( bool isRunning )
{
	_isRunning = isRunning;
}

bool MainObject::isRunning() const
{
	return _isRunning;
}

bool MainObject::runTests()
{
	system( _removeBandwidthStatsFile.c_str() );
	bool allGood = true;

	for ( TestVector::iterator it = _testToRun.begin(); it != _testToRun.end(); ++it )
	{
		allGood |= (*it)->runTest( &_ipVector );
	}

	_isRunning = false;
	system( _logStatsCommand.c_str() );
	return allGood;
}

bool MainObject::parseCommandLineArguments( int argc, char* const* argv )
{
	CommandLineArgumentParser parser;

	if ( !parser.parseCommandLineArguments( argc, argv, _testData, _ipVector, _testToRun ) )
	{
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
	for ( size_t i = 0; i < _ipVector.size(); ++i )
	{
		delete _ipVector[ i ];
	}
}

void MainObject::deleteTestVector()
{
	for ( size_t i = 0; i < _testToRun.size(); ++i )
	{
		delete _testToRun[ i ];
	}
}

}
