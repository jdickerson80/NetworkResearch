#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <errno.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <signal.h>

#include "ExponentialSmoothing.h"
#include "DataRateCalculator.h"

using namespace std;

static const string ReceiveBytesPath( "/sys/class/net/wlp2s0/statistics/rx_bytes" );
static const string SentBytesPath( "/sys/class/net/wlp2s0/statistics/tx_bytes" );
static const string TCCommand( "tc -s -d class show dev wlp2s0" );

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
	if( getuid() != 0 )
	{
		printf("You must run this program as root. Exiting.\n");
		exit( EXIT_FAILURE );
	}

	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );

	unsigned int counter = 0;

	DataRateCalculator< unsigned int, float, Math::ExponentialSmoothing > receiveCalculator( ReceiveBytesPath );
	DataRateCalculator< unsigned int, float, Math::ExponentialSmoothing > sentCalculator( SentBytesPath );

	system( TCCommand.c_str() );

	setlocale( LC_ALL, "" );

	while ( isRunning )
	{
		receiveCalculator.calculateRate();
		sentCalculator.calculateRate();

//		if ( counter % 10 == 0 )
		{
			printf("Rec. rate is:\t%'2.2f\tB/sec\n", receiveCalculator.rate() );
//			printf("Send rate is:\t%'2.2f\tB/sec\n", sentCalculator.rate() );
		}

		++counter;

		sleep( 1 );
	}

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
