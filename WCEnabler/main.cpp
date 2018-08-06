#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Macros.h"
#include "WCEnablerObject.h"

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

	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );
	setlocale( LC_ALL, "" );

//	system( "sysctl -w net.ipv4.tcp_ecn=3" );
//	system( "sysctl net.mptcp.mptcp_path_manager=ndiffports" );
//	system( "echo 2 > /sys/module/mptcp_ndiffports/parameters/num_subflows" );

	WCEnablerObject object( std::string( BGAdaptorIPAddress ) );

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
