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
#include <sstream>


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

	if ( argc < 2 )
	{
		printf("You must give the interface in the command line arguments. Exiting.\n");
		exit( EXIT_FAILURE );
	}

	signal( SIGINT, signalHandler );
	signal( SIGTERM, signalHandler );

//	unsigned int counter = 0;
//	unsigned int bw = 2;

//	string outputFile = FileControl::buildOutputFilePath( argv[ 1 ] );
//	LoggingHandler bandwidthLogger( outputFile );
//	std::stringstream outputStream;
//	TCControl::setEgressBandwidth( string( argv[ 1 ] ), bw, "50" );

//	printf( "%s\n", argv[ 1 ] );

//	DataRateCalculator< unsigned int, float, Math::ExponentialSmoothing > receiveCalculator( FileControl::buildReceivePath( argv[ 1 ] ) );
//	DataRateCalculator< unsigned int, float, Math::ExponentialSmoothing > sentCalculator( FileControl::buildSendPath( argv[ 1 ] ) );

	setlocale( LC_ALL, "" );

	while ( isRunning )
	{
//		if ( counter % 20 == 0 )
//		{
//			bw += 2;
//			TCControl::setEgressBandwidth( string( argv[ 1 ] ), bw, "50" );
//			printf("set %d\n", bw );
//		}

//		outputStream << "Rec. rate is: " << receiveCalculator.calculateRate() << " B/sec\n";
//		outputStream << "Send rate is: " << sentCalculator.calculateRate() << " B/sec\n";
//		outputStream << "~~~~~~~~~~~~~~~~\n";

//		bandwidthLogger.log( outputStream.str() );

//		++counter;

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
