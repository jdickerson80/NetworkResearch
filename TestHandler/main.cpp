#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>

#include "UsageArguments.h"

static bool isRunning = true;

using namespace TestHandler;
/**
 * @brief	signalHandler method handles all of the signals that come from Linux.
 *			For instance, this method catches when the user presses Ctrl + C or
 *			when the user presses X on the terminal window. This method sets the
 *			_isRunning flag to false, triggering main to exit the app.
 * @param	signal to be received from Linux
 */
static void signalHandler( int signal );

int main( int argc, char* argv[] )
{
	// check if the app is running as root
//  if ( getuid() != 0 )
//	{
//		printf("You must run this program as root. Exiting.\n");
//		exit( EXIT_FAILURE );
//	}

	// add the signal handlers
	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );
	setlocale( LC_ALL, "" );

	if ( argc < 4 )
	{
		printUsage();
		exit( EXIT_FAILURE );
	}

	int opt;
	unsigned int* hostRange;

	// setup the long options
	static struct option longOptions[] =
	{
		{ "duration",	required_argument,	0, UsageArguments::Duration },
		{ "help",		no_argument,		0, UsageArguments::Help },
		{ "host-range",	required_argument,	0, UsageArguments::HostRange },
		{ "logfile",	required_argument,	0, UsageArguments::LogFile },
		{ 0,			0,					0,	0  }
	};

	// parse the user's arguements
	while ( ( opt = getopt_long( argc, argv, "hd:r:l:", longOptions, NULL ) ) != -1 )
	{
		switch ( opt )
		{
		case UsageArguments::Duration:
			printf( "Duration %i\n", atoi( optarg ) );
			break;

		case UsageArguments::HostRange:
		{
			unsigned int delta;
			unsigned int start;
			unsigned int finish;

			start	= atoi( (char*)&optarg[ 0 ] );
			finish	= atoi( (char*)&optarg[ 2 ] );

			delta = ( finish - start ) + 1;

			hostRange = (unsigned int*)malloc( ( delta ) * sizeof( unsigned int ) );

			printf("start %u finish %u delta %u\n", start, finish, delta );

			hostRange[ 0 ] = start;
			for ( size_t i = 1; i < delta; ++i )
			{
				hostRange[ i ] = ++start;
			}

			for ( size_t i = 0; i < delta; ++i )
			{
				printf("%i, ", hostRange[ i ] );
			}
		}
			break;

		case UsageArguments::Help:
			printUsage();
			exit( EXIT_FAILURE );
			break;

		case UsageArguments::LogFile:
			printf( "LogFile %s\n", optarg );
			break;

		case UsageArguments::Test:
			printf( "Test %s\n", optarg );
			break;

		default: /* '?' */
			printUsage();
			exit( EXIT_FAILURE );
		}
	}

	// do nothing loop to keep the app going
	while ( isRunning )
	{
		sleep( 1 );
	}

	exit( EXIT_SUCCESS );
}

void signalHandler( int signal )
{
	switch ( signal )
	{
	case SIGINT:
		printf( "Caught Ctrl + C\n" );
		isRunning = false;
		break;

	case SIGTERM:
		printf( "Caught Terminate\n" );
		isRunning = false;
		break;

	default:
		printf( "In default signal\n" );
		break;
	}
}
