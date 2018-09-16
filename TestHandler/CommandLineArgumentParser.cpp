#include "CommandLineArgumentParser.h"

#include <getopt.h>
#include <string.h>
//#include <stdlib.h>

#include "PrintHandler.h"

using namespace std;
namespace TestHandler {

// setup the long options
static struct option longOptions[] =
{
	{ "duration",	required_argument,	0, CommandLineArgumentParser::UsageArguments::Duration },
	{ "help",		no_argument,		0, CommandLineArgumentParser::UsageArguments::Help },
	{ "host-range",	required_argument,	0, CommandLineArgumentParser::UsageArguments::HostRange },
	{ "logfile",	required_argument,	0, CommandLineArgumentParser::UsageArguments::LogFile },
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
	: _start( 0 )
	, _finish( 0 )
{
	memset( _startIP, 0, BUFFERSIZE );
	memset( _endIP, 0, BUFFERSIZE );
	memset( _lastOctet, 0, BUFFERSIZE );
}

void CommandLineArgumentParser::parseCommandLineArguments( int argc, char*const* argv, std::vector< std::string* >& ipVector )
{
	int opt;

	// parse the user's arguements
	while ( ( opt = getopt_long( argc, argv, "hd:r:l:", longOptions, NULL ) ) != -1 )
	{
		switch ( opt )
		{
		case UsageArguments::Duration:
			PRINT( "Duration %i\n", atoi( optarg ) );
			break;

		case UsageArguments::HostRange:
			parseIPRange( optarg, ipVector );
			break;

		case UsageArguments::Help:
			printUsage();
			break;

		case UsageArguments::LogFile:
//			PRINT( "LogFile %s\n", optarg );
			break;

		case UsageArguments::Test:
//			PRINT( "Test %s\n", optarg );
			break;

		default: /* '?' */
			printUsage();
		}
	}
}

void CommandLineArgumentParser::printUsage()
{
	const char* const usage = \
						"Usage: TestHandler [-d] [-h] [-l] [-r] [-t]\n" \
						"-d, --duration\tduration of the test in seconds\n" \
						"-h, --help\tprint this help message\n" \
						"-l, --logfile\tpath and name of the file to log test results\n" \
						"-r, --range\trange of ip addresses with a hyphen(-) being a range and a comma (,) inidividual IP address\n" \
						"\t\te.g. ""10.0.0.1-10.0.0.4,10.0.0.18,10.0.0.19,10.0.0.20""\n" \
						"-t, --test\tcomma separated list of test to run. tests are: todo\n";
	fprintf( stderr, "%s", usage );
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
}
