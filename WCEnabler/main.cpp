#include <signal.h>
#include <sstream>
#include <unistd.h>

#include "BandwidthCommunicator.h"
#include "DataRateCalculator.h"
#include "ExponentialSmoothing.h"
#include "FileControl.h"
#include "LoggingHandler.h"
#include "TCControl.h"

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

	unsigned int counter = 0;
	unsigned int bw = 2;

	string outputFile = FileControl::buildOutputFilePath( argv[ 1 ] );
	Common::LoggingHandler bandwidthLogger( outputFile );
	std::stringstream outputStream;
	Common::TCControl::setEgressBandwidth( string( argv[ 1 ] ), bw, "50" );

	printf( "%s\n", argv[ 1 ] );

	Common::DataRateCalculator< unsigned int, float, Common::Math::ExponentialSmoothing > receiveCalculator( FileControl::buildReceivePath( argv[ 1 ] ) );
	Common::DataRateCalculator< unsigned int, float, Common::Math::ExponentialSmoothing > sentCalculator( FileControl::buildSendPath( argv[ 1 ] ) );

	setlocale( LC_ALL, "" );

	BandwidthCommunicator communicator( "10.0.0.1", string( argv[ 1 ] ) );



	while ( isRunning )
	{
//		if ( counter % 20 == 0 )
//		{
//			bw += 2;
//			TCControl::setEgressBandwidth( string( argv[ 1 ] ), bw, "50" );
//			printf("set %d\n", bw );
//		}

		outputStream << "Rec. rate is: " << receiveCalculator.calculateRate() << " B/sec\n";
		outputStream << "Send rate is: " << sentCalculator.calculateRate() << " B/sec\n";
		outputStream << "~~~~~~~~~~~~~~~~\n";

		bandwidthLogger.log( outputStream.str() );

		communicator.sendBandwidth( receiveCalculator.rate() );

		++counter;

		usleep( 500000 );
	}

	communicator.setBandwidthThreadRunning( false );
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
