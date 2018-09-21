#include "CommandLineArgumentParser.h"

#include <getopt.h>
#include <string.h>
//#include <stdlib.h>

#include "PrintHandler.h"
#include "PrintUsage.h"
#include "TestData.h"

#include "Tests/SingleClientTest.h"
#include "Tests/WCBandwidthUtilization.h"

using namespace std;
namespace TestHandler {

// setup the long options
static struct option longOptions[] =
{
	{ "bytes",		required_argument,	0, CommandLineArgumentParser::UsageArguments::Bytes },
	{ "duration",	required_argument,	0, CommandLineArgumentParser::UsageArguments::Duration },
	{ "help",		no_argument,		0, CommandLineArgumentParser::UsageArguments::Help },
	{ "host-range",	required_argument,	0, CommandLineArgumentParser::UsageArguments::HostRange },
	{ "logfile",	required_argument,	0, CommandLineArgumentParser::UsageArguments::LogFile },
	{ "targetBW",	required_argument,	0, CommandLineArgumentParser::UsageArguments::Targetbandwidth },
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

	_testData = testData;
	testData->bytesToBeTransmitted = 0;
	testData->duration = 0;
	testData->targetBandwidth = 0;

	// parse the user's arguments
	while ( ( opt = getopt_long( argc, argv, "hbd:r:l:T:t:", longOptions, NULL ) ) != -1 )
	{
		switch ( opt )
		{
		case UsageArguments::Bytes:
			testData->bytesToBeTransmitted = atoi( optarg );
			PRINT( "Bytes %i\n", atoi( optarg ) );
			break;

		case UsageArguments::Duration:
			testData->duration = atoi( optarg );
			PRINT( "Duration %i\n", atoi( optarg ) );
			break;

		case UsageArguments::Help:
			printUsage();
			_goodParse = false;
			break;

		case UsageArguments::HostRange:
			parseIPRange( optarg, ipVector );
			_goodParse = !ipVector.empty();
			break;

		case UsageArguments::LogFile:
			testData->logPath = optarg;
			PRINT( "LogFile %s\n", optarg );
			break;

		case UsageArguments::Targetbandwidth:
			testData->targetBandwidth = atoi( optarg );
			PRINT( "TB %i\n", atoi( optarg ) );
			break;

		case UsageArguments::Test:
			parseTests( optarg, test );
			PRINT( "Test %s\n", optarg );
			break;

		default: /* '?' */
			printUsage();
			_goodParse = false;
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

	printVector( ipVector );
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

	if ( !strcmp( testString, "ClientServer" ) )
	{
//		ret = new WCBandwidthUtilization();
		PRINT( "ClientServer\n" );
	}
	else if ( !strcmp( testString, "Efficiency" ) )
	{
//		ret = new WCBandwidthUtilization();
		PRINT( "Efficiency\n" );
	}
	else if ( !strcmp( testString, "LongFlowHandling" ) )
	{
		PRINT( "LongFlowHandling\n" );
	}
	else if ( !strcmp( testString, "RandomFlowHandling" ) )
	{
		PRINT( "RandomFlowHandling\n" );
	}
	else if ( !strcmp( testString, "ShortFlowHandling" ) )
	{
		PRINT( "ShortFlowHandling\n" );
	}
	else if ( !strcmp( testString, "SingleClient" ) )
	{

		ret = new SingleClientTest( _testData );
		PRINT( "SingleClient\n" );
	}
	else if ( !strcmp( testString, "SingleServer" ) )
	{
//		ret = new WCBandwidthUtilization();
		PRINT( "SingleServer\n" );
	}
	else if ( !strcmp( testString, "WCBandwidthUtilization" ) )
	{
		PRINT( "WCBandwidthUtilization\n" );
	}
	else if ( !strcmp( testString, "WCLogic" ) )
	{
		PRINT( "WCLogic\n" );
	}
	else
	{
		PRINT( "!!!!Default\n" );
	}

	return ret;
}

}
