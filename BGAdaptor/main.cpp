#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

#include "BGPrintHandler.h"
#include "CommandLineArgumentParser.h"
#include "HostBandwidthHandler.h"

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
		perror("You must run this program as root. Exiting.\n");
		exit( EXIT_FAILURE );
	}

	if ( argc != 7 )
	{
		CommandLineArgumentParser::printUsage();
		exit( EXIT_FAILURE );
	}

	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );

	setlocale( LC_ALL, "" );
	uint numberOfHosts = 0;
	uint bandwidthGuarantee = 0;
	uint dynamicAllocation = 0;

	CommandLineArgumentParser::parseCommandLineArgument( argc, argv, &dynamicAllocation, &numberOfHosts, &bandwidthGuarantee );

	if ( !numberOfHosts || !bandwidthGuarantee )
	{
		CommandLineArgumentParser::printUsage();
		exit( EXIT_FAILURE );
	}

	BGAdaptor::HostBandwidthHandler handler( bandwidthGuarantee, dynamicAllocation, numberOfHosts );

	while ( isRunning )
	{
		sleep( 1 );
	}

	handler.~HostBandwidthHandler();

	exit( EXIT_SUCCESS );
}

void signalHandler( int signal )
{
	switch ( signal )
	{
	case SIGINT:
		PRINT( "Caught Ctrl + C\r\n" );
		isRunning = false;
		break;

	case SIGTERM:
		PRINT( "Caught Terminate\r\n" );
		isRunning = false;
		break;

	default:
		PRINT( "In default signal\n" );
		break;
	}
}
