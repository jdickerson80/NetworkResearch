#include "BandwidthCalculator.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include "BandwidthValues.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "ThreadHelper.h"
#include "WCPrintHandler.h"

// macros to avoid "magic numbers"
#define BufferSize ( 1024 )
#define IPAddressSize ( 20 )
#define LogBufferSize ( 1024 )
#define PacketBufferSize ( 65536 )

#define BandwidthGuaranteeLine ( 1 )
#define WorkConservationLine ( 13 )

#define StatisticTCCommand ( "tc -s class show dev " )

//#define LogPackets ( 1 )

namespace WCEnabler {

BandwidthCalculator::BandwidthCalculator(
		Common::LoggingHandler* logger
		, const std::string& interfaceIPAddress
		, BandwidthValues* bandwidthValues
		, const std::string& interfaceName )
	: _packetSniffingThreadRunning( false )
	, _calculationThreadRunning( false )
	, _bandwidthGuaranteeCounter( 0 )
	, _workConservingCounter( 0 )
	, _bandwidthValues( bandwidthValues )
	, _logger( logger )
	, _interfaceIPAddress( interfaceIPAddress )
	, _interfaceName( interfaceName )
{
	// create the raw socket that intercepts ALL packets
	_socketFileDescriptor = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_IP ) );

	// check for socket fd error
	if ( _socketFileDescriptor == -1 )
	{
		PRINT( "socket sniffer failed\n" );
	}

#if defined( LogPackets )
	// add the header to the log
	char buffer[ LogBufferSize ];
	snprintf( buffer, LogBufferSize, "protocol, source, destination, ecn\n" );
	_logger->log( buffer );
#endif

	// start both threads
	Common::ThreadHelper::startDetachedThread(
				&_packetSniffingThread
				, handlePacketSniffing
				, _packetSniffingThreadRunning
				, static_cast< void* >( this ) );

	Common::ThreadHelper::startDetachedThread(
				&_calculationThread
				, handleRateCalculation
				, _calculationThreadRunning
				, static_cast< void* >( this ) );
}

BandwidthCalculator::~BandwidthCalculator()
{
	// shut off all of the threads, close the socket, and delete the logger
	_calculationThreadRunning = false;
	_packetSniffingThreadRunning = false;
	close( _socketFileDescriptor );
	delete _logger;
}

void* BandwidthCalculator::handlePacketSniffing( void* input )
{
	// init the variables
	BandwidthCalculator* bandwidthCalculator = static_cast< BandwidthCalculator* >( input );
	sockaddr socketAddress;
	ssize_t dataSize;
	int socketAddressLength = sizeof( socketAddress );
	std::atomic_bool& threadRunning = bandwidthCalculator->_packetSniffingThreadRunning;
	std::atomic_uint& ecn = bandwidthCalculator->_bandwidthValues->ecnValue;
	unsigned int localECN = 0;
	unsigned char packetBuffer[ PacketBufferSize ];
	Common::LoggingHandler* logger = bandwidthCalculator->_logger;
	iphdr* ipHeader;

#if defined( LogPackets )
	char sourceAddress[ IPAddressSize ];
	char destinationAddress[ IPAddressSize ];
	char logBuffer[ LogBufferSize ];
	ethhdr* ethernetHeader;
#endif

	// get the appropriate headers
	ipHeader = (iphdr*)( packetBuffer + sizeof( ethhdr ) );

	// while the thread should run
	while ( threadRunning.load() )
	{
		// if there is already an ecn, just continue
		if ( ecn.load() )
		{
			// sleep for a second
			usleep( 25000 );
			continue;
		}

		// receive the BG Adaptor's rate
		dataSize = recvfrom( bandwidthCalculator->_socketFileDescriptor
							 , packetBuffer
							 , PacketBufferSize
							 , 0
							 , &socketAddress
							 , (socklen_t*)&socketAddressLength );

		// check for error
		if ( dataSize < 0 )
		{
			// log the error and continue
			logger->log( strerror( errno ) );
			continue;
		}

		// no ecn, so mask out the DSCP field and check if congestion has been encountered
		localECN = ( ipHeader->tos & INET_ECN_MASK ) == INET_ECN_CE;
		ecn = localECN;

//		if ( ecn.load() == true )
//		{
//			PRINT("GOT ECN!!!!!\n");
//		}


#if defined( LogPackets )
		// put the addresses into the string
		// @note This HAS to be done sequentially and stored because the inet_ntoa char* buffer
		// will be overridden every time it is called. This will make both addresses the
		// same.
		snprintf( destinationAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->daddr ) ) );
		snprintf( sourceAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->saddr ) ) );

		// set up the logging string
		ethernetHeader = (ethhdr*)packetBuffer;
		snprintf( logBuffer, LogBufferSize, "%u, %s, %s, %u\n"
				  , ethernetHeader->h_proto
				  , sourceAddress
				  , destinationAddress
				  , ecn.load() );

		// log it
		logger->log( logBuffer );
#endif
	}

	pthread_exit( nullptr );
}

unsigned int BandwidthCalculator::findValueAfterCharacter( char* buffer, const char deliminatingCharacter, size_t startLoopPosition )
{
#define BuffSize ( 10 )
	// init variables
	char loopBuffer[ BuffSize ] = {'\0'};
	size_t loopVariable = 0;

	// skip the space at the beginning of the line
	++startLoopPosition;

	// find the deliminating character
	while ( buffer[ ++startLoopPosition ] != deliminatingCharacter )
	{}

	// decrement one, since it is a post fix ++ above
//	--startLoopPosition;

	// add all non-spaces to the integer
	while ( buffer[ ++startLoopPosition ] != ' ' )
	{
		loopBuffer[ loopVariable++ ] = buffer[ startLoopPosition ];
	}

	// cast the buffer to a uint data type
	return static_cast< uint >( atoi( loopBuffer ) );
}

void* BandwidthCalculator::handleRateCalculation( void* input )
{
#define TCFileSize ( 673 )
	// init the variables
	BandwidthCalculator* bandwidthCalculator = static_cast< BandwidthCalculator* >( input );
	std::atomic_bool& threadRunning = bandwidthCalculator->_calculationThreadRunning;

	char buffer[ BufferSize ];
	memset( buffer, 0, BufferSize );
	const char* loggingFilePath = bandwidthCalculator->_logger->loggingPath().c_str();
	int fileDescriptor;
	int bytesRead;

	// init the stream
	std::ostringstream stream;
	std::string tcCommand;

	// stream the command
	stream << StatisticTCCommand << bandwidthCalculator->_interfaceName << " > " << loggingFilePath;
	tcCommand = stream.str();

	// while the thread should run
	while ( threadRunning.load() )
	{
		// run tc command
		system( tcCommand.c_str() );

		// open the file the tc output was redirected to
		fileDescriptor = open( loggingFilePath, O_RDONLY );

		// buffer the file
		bytesRead = read( fileDescriptor, buffer, BufferSize );

		// close the file
		close( fileDescriptor );

		// error check
		if ( bytesRead < TCFileSize )
		{
			PRINT("CONT!!!\n");
			// sleep for a second
			usleep( 125000 );
			continue;
		}

		// parse the file
		bandwidthCalculator->parseTCFile( buffer );

		// sleep for a second
		usleep( 250000 );
	}

	pthread_exit( nullptr );
}

void BandwidthCalculator::parseTCFile( char* buffer )
{
#define IntBufferSize ( 10 )
#define BWGNewLineOffset ( 0 )
#define WCNewLineOffset ( 10 )
#define WCFlowID ( 12 )
#define BWGFlowID ( 11 )
	std::atomic_uint& bandwidthGuaranteeRate = _bandwidthValues->bandwidthGuaranteeRate;
	std::atomic_uint& totalRate = _bandwidthValues->totalRate;
	std::atomic_uint& workConservingRate = _bandwidthValues->workConservingRate;
	size_t loop = 0;
	unsigned int tempBandwidth = 0;

	int newLineCounter = 0;

	while ( newLineCounter != BWGNewLineOffset )
	{
		while ( buffer[ loop++ ] != '\n' );
		++loop;
		++newLineCounter;
	}

	if ( findValueAfterCharacter( buffer, ':', loop ) == BWGFlowID )
	{
		while ( buffer[ loop++ ] != '\n' );
		++loop;
		++newLineCounter;
		_bandwidthGuaranteeCounter = findValueAfterCharacter( buffer, ' ', ++loop );
	}

	while ( newLineCounter != WCNewLineOffset )
	{
		while ( buffer[ loop++ ] != '\n' );
		++loop;
		++newLineCounter;
	}

	if ( findValueAfterCharacter( buffer, ':', loop ) == WCFlowID )
	{
		while ( buffer[ loop++ ] != '\n' );
		++loop;
		++newLineCounter;
		_workConservingCounter = findValueAfterCharacter( buffer, ' ', ++loop );
	}

//	PRINT("%u %u ", _bandwidthGuaranteeCounter.load(), _workConservingCounter.load() );
	// calculate the rate
	tempBandwidth = _bandwidthGuaranteeRateCalculator.calculateRate( _bandwidthGuaranteeCounter.load( ) );

	// set it equal to temp variable
	bandwidthGuaranteeRate = tempBandwidth;

	// calculate the rate
	tempBandwidth = _workConservingRateCalculator.calculateRate( _workConservingCounter.load( ) );

	// set it equal to temp variable
	workConservingRate = tempBandwidth;

	// last value, so calculate total rate
	tempBandwidth = bandwidthGuaranteeRate.load() + workConservingRate.load();
	totalRate = tempBandwidth;

	PRINT("bwg %u\twc %u\ttot %u\n"
		   , bandwidthGuaranteeRate.load()
		   , workConservingRate.load()
		   , totalRate.load() );
}

} // namespace WCEnabler
