#include "CommandLineArgumentParser.h"

#include <getopt.h>
#include <string.h>
//#include <stdlib.h>

#include "PrintHandler.h"
#include "TestData.h"

#include "Tests/ClientServerTest.h"
#include "Tests/SingleClientTest.h"
#include "Tests/SingleServerTest.h"

using namespace std;
namespace TestHandler {

// setup the long options
static struct option longOptions[] =
{
	{ "bytes",		optional_argument,	0, CommandLineArgumentParser::UsageArguments::Bytes },
	{ "time",		optional_argument,	0, CommandLineArgumentParser::UsageArguments::Duration },
	{ "help",		no_argument,		0, CommandLineArgumentParser::UsageArguments::Help },
	{ "range",		required_argument,	0, CommandLineArgumentParser::UsageArguments::HostRange },
	{ "logfile",	required_argument,	0, CommandLineArgumentParser::UsageArguments::LogFile },
	{ "port",		optional_argument,	0, CommandLineArgumentParser::UsageArguments::Port },
	{ "parallel",	no_argument,		0, CommandLineArgumentParser::UsageArguments::ParallelTests },
	{ "bitrate",	optional_argument,	0, CommandLineArgumentParser::UsageArguments::Targetbandwidth },
	{ "test",		required_argument,	0, CommandLineArgumentParser::UsageArguments::Test },
	{ 0,			0,					0,	0  }
};

void printVector( std::vector< std::string* >& ipVector )
{
	std::vector< std::string* >::const_iterator it = ipVector.begin();

	for ( ; it < ipVector.end(); ++it )
	{
		PRINT( "%s ", (*it)->c_str() );
	}

	PRINT( "\n" );
}

/// @todo think about converting it to a functor
CommandLineArgumentParser::CommandLineArgumentParser()
	: _goodParse( true )
	, _start( 0 )
	, _finish( 0 )
{
	memset( _startIP, 0, BUFFERSIZE );
	memset( _endIP, 0, BUFFERSIZE );
	memset( _lastOctet, 0, BUFFERSIZE );
}

CommandLineArgumentParser::~CommandLineArgumentParser()
{

}

bool CommandLineArgumentParser::parseCommandLineArguments(
		int argc
		, char*const* argv
		, TestData* testData
		, std::vector< std::string* >& ipVector
		, std::vector< TestBaseClass* >& test )
{
	int opt;
	size_t stringLength;

	_testData = testData;

	// parse the user's arguments
	while ( ( opt = getopt_long( argc, argv, "hb:l:n:pr:t:z:", longOptions, NULL ) ) != -1 )
	{
		if ( optarg )
		{
			stringLength = strlen( optarg );
		}

//		PRINT("Got %c\n", opt );
		switch ( opt )
		{
		case UsageArguments::Bytes:
//			PRINT( "Bytes %s\n", testData->bytesToBeTransmitted );
			memcpy( testData->bytesToBeTransmitted, optarg, stringLength );
			break;

		case UsageArguments::Duration:
//			PRINT( "Duration %s\n", testData->duration );
			memcpy( testData->duration, optarg, stringLength );
			break;

		case UsageArguments::Help:
			return false;
			break;

		case UsageArguments::HostRange:
//			PRINT( "Range %s\n", optarg );
			parseIPRange( optarg, ipVector );
			_goodParse = !ipVector.empty();
			break;

		case UsageArguments::LogFile:
//			PRINT( "LogFile %s\n", optarg );
			testData->logPath = optarg;
			break;

		case UsageArguments::ParallelTests:
			testData->runInParallel = true;
//			PRINT( "Parallel %u\n", testData->runInParallel );
			break;

		case UsageArguments::Port:
			testData->port = optarg;
//			PRINT( "Parallel %u\n", testData->runInParallel );
			break;

		case UsageArguments::Targetbandwidth:
//			PRINT( "TB %i\n", atoi( optarg ) );
			memcpy( testData->targetBandwidth, optarg, stringLength );
			break;

		case UsageArguments::Test:
//			PRINT( "Test %s\n", optarg );
			parseTests( optarg, test );
			break;

		default: /* '?' */
			PRINT( "?????????\n" );
			return false;
		}
	}

	return _goodParse;
}

void CommandLineArgumentParser::parseIPRange( char* optarg, std::vector<string *>& ipVector )
{
#define FirstOctet	( 1 )
#define SecondOctet	( 2 )
#define ThirdOctet	( 3 )
#define FourthOctet	( 4 )
#define LastPass	( 7 )
	bool isRange = false;
	char* currentBuffer = _startIP;
	size_t stringLength = strlen( optarg );
	size_t lastPass = stringLength - 1;
	size_t bufferIndex = 0;
	size_t lastOctetIndex = 0;
	size_t dotCount = FirstOctet;

	for ( size_t i = 0; i < stringLength; ++i )
	{
		if ( optarg[ i ] == ' ' )
		{
			continue;
		}

		if ( optarg[ i ] == '.' )
		{
			++dotCount;
			currentBuffer[ bufferIndex++ ] = optarg[ i ];
			continue;
		}

		if ( i == lastPass  )
		{
			dotCount = LastPass;
		}

		switch ( dotCount )
		{
		case FirstOctet:
		case SecondOctet:
		case ThirdOctet:
			currentBuffer[ bufferIndex++ ] = optarg[ i ];
			break;

		case LastPass:
			_lastOctet[ lastOctetIndex++ ] = optarg[ i ];
			currentBuffer[ bufferIndex++ ] = optarg[ i ];
			optarg[ i ] = ',';

		case FourthOctet:
			if ( optarg[ i ] == '-' )
			{
				_start = (uint)atoi( _lastOctet );
				currentBuffer = _endIP;
				bufferIndex = 0;
				memset( _lastOctet, 0, lastOctetIndex );
				lastOctetIndex = 0;
				isRange = true;
				dotCount = FirstOctet;
				continue;
			}

			if ( optarg[ i ] == ',' )
			{
				if ( isRange )
				{
					// handle range
					_finish = (uint)atoi( _lastOctet );
					fillIPVector( ipVector );
					memset( _endIP, 0, bufferIndex );
					currentBuffer = _startIP;
					isRange = false;
				}
				else
				{
					ipVector.push_back( new string( currentBuffer ) );
				}

				memset( _startIP, 0, bufferIndex );
				memset( _lastOctet, 0, lastOctetIndex );
				lastOctetIndex = 0;
				bufferIndex = 0;
				dotCount = FirstOctet;
				continue;
			}

			_lastOctet[ lastOctetIndex++ ] = optarg[ i ];
			currentBuffer[ bufferIndex++ ] = optarg[ i ];
			break;
		}
	}

//	printVector( ipVector );
}

void CommandLineArgumentParser::fillIPVector( std::vector<string *> &ipVector )
{
	char buffer[ 1024 ];
	for ( size_t i = _start; i <= _finish; ++i )
	{
		snprintf( buffer, 20, "10.0.0.%lu", i );
		ipVector.push_back( new std::string( buffer ) );
	}
}

void CommandLineArgumentParser::parseTests( char *optarg, std::vector<TestBaseClass *>& test )
{
	char buffer[ BUFFERSIZE ];
	memset( buffer, 0, BUFFERSIZE );
	size_t stringLength = strlen( optarg );
	size_t end = stringLength - 1;
	size_t bufferIndex = 0;
	TestBaseClass* baseTest = NULL;

	for ( size_t i = 0; i < stringLength; ++i )
	{
		if ( optarg[ i ] == ' ' )
		{
			continue;
		}

		if ( i == end )
		{
			buffer[ bufferIndex++ ] = optarg[ i ];
			optarg[ i ] = ',';
		}

		if ( optarg[ i ] == ',' )
		{
			buffer[ bufferIndex++ ] = '\0';
			baseTest = getTest( buffer );

			if ( baseTest )
			{
				test.push_back( baseTest );
			}

			memset( buffer, 0, bufferIndex );
			bufferIndex = 0;
			baseTest = NULL;
			continue;
		}

		buffer[ bufferIndex++ ] = optarg[ i ];
	}
}

TestBaseClass* CommandLineArgumentParser::getTest( const char* const testString )
{
	TestBaseClass* ret = NULL;

//	PRINT( "got: %s\n", testString );

	if ( !strcmp( testString, "SingleClient" ) )
	{
		ret = new SingleClientTest( _testData );
		PRINT( "SingleClient\n" );
	}
	else if ( !strcmp( testString, "SingleServer" ) )
	{
		ret = new SingleServerTest( _testData );
		PRINT( "SingleServer\n" );
	}
	else if ( !strcmp( testString, "ClientServer" ) )
	{
		ret = new ClientServerTest( _testData );
	}
	else
	{
		PRINT( "!!!!Default\n" );
	}

	return ret;
}

}
