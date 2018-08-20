#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

#include "HandleHostBandwidth.h"
using namespace std;

static bool isRunning = true;

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
    if ( getuid() != 0 )
	{
		printf("You must run this program as root. Exiting.\n");
		exit( EXIT_FAILURE );
	}

	if ( argc < 3 )
	{
		perror( "You must give the number of hosts and the tenant's total bandwidth in kilobytes the command line arguments. Exiting.\n" );
		exit( EXIT_FAILURE );
	}

	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );

	setlocale( LC_ALL, "" );

	BGAdaptor::HandleHostBandwidth hostBandwidth( atoi( argv[ 1 ] ) - 1, atoi( argv[ 2 ] ) );

	while ( isRunning )
	{
		hostBandwidth.sendBandwidthRates();
		hostBandwidth.logBandwidths();
		usleep( 250000 );
	}

	hostBandwidth.setRunning( false );
	exit( EXIT_SUCCESS );
}

void signalHandler( int signal )
{
	switch ( signal )
	{
	case SIGINT:
		printf( "Caught Ctrl + C\r\n" );
		isRunning = false;
		break;

	case SIGTERM:
		printf( "Caught Terminate\r\n" );
		isRunning = false;
		break;

	default:
		printf( "In default signal\n" );
		break;
	}
}
