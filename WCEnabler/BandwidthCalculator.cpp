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
#include "PrintHandler.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "ThreadHelper.h"

// macros to avoid "magic numbers"
#define BufferSize ( 1024 )
#define IPAddressSize ( 20 )
#define LogBufferSize ( 1024 )
#define PacketBufferSize ( 65536 )

#define BandwidthGuaranteeLine ( 1 )
#define WorkConservationLine ( 12 )

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

		// if there is already an ecn, just continue
		if ( ecn.load() )
		{
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

	pthread_exit( NULL );
	return NULL;
}

void* BandwidthCalculator::handleRateCalculation( void* input )
{
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
		if ( bytesRead <= 300 )
		{
			continue;
		}

		// parse the file
		bandwidthCalculator->parseTCFile( buffer, bytesRead );

		// sleep for a second
		usleep( 250000 );
	}

	pthread_exit( NULL );
	return NULL;
}

void BandwidthCalculator::parseTCFile( char* buffer, unsigned int bufferSize )
{
#define NewlineCharacter ( 1 )
#define MovePastSent ( 6 )
#define TotalOffset ( NewlineCharacter + MovePastSent )
#define IntBufferSize ( 10 )
	size_t bufferIndex;
	char intBuffer[ IntBufferSize ];
	unsigned int numberOfNewLines = 0;
	unsigned int tempBandwidth = 0;

	std::atomic_uint& bandwidthGuaranteeRate = _bandwidthValues->bandwidthGuaranteeRate;
	std::atomic_uint& totalRate = _bandwidthValues->totalRate;
	std::atomic_uint& workConservingRate = _bandwidthValues->workConservingRate;

	// loop through the buffer
	for ( size_t loop = 0; loop < bufferSize; ++loop )
	{
		// if there is a new line
		if ( buffer[ loop ] == '\n' )
		{
			// increment the variable
			++numberOfNewLines;

			// if this is the bandwidth guarantee or work conservation line
			if ( numberOfNewLines == BandwidthGuaranteeLine || numberOfNewLines == WorkConservationLine )
			{
				// clear the index
				bufferIndex = 0;

				// add the \n and offset to account for the sent
				loop += TotalOffset;

				// while the buffer does not contain a space
				while ( buffer[ loop ] != ' ' )
				{
					if ( bufferIndex >= IntBufferSize )
					{
						break;
					}

					// add the character to the buffer
					intBuffer[ bufferIndex++ ] = buffer[ loop ];

					// increment the variable
					loop++;
				}

				// convert the characters into an integer
				tempBandwidth = (uint) atoi( intBuffer );

				// convert it from bytes to bits
				tempBandwidth *= 8;

				// if this is the bandwidth guarantee line
				if ( numberOfNewLines == BandwidthGuaranteeLine )
				{
					// set the counter equal to the temp bandwidth
					_bandwidthGuaranteeCounter = tempBandwidth;

					// calculate the rate
					tempBandwidth = _bandwidthGuaranteeRateCalculator.calculateRate( _bandwidthGuaranteeCounter.load( ) );

					// set it equal to temp variable
					bandwidthGuaranteeRate = tempBandwidth;
				}
				// if this is the work conservation line
				else if ( numberOfNewLines == WorkConservationLine )
				{
					// set the counter equal to the temp bandwidth
					_workConservingCounter = tempBandwidth;

					// calculate the rate
					tempBandwidth = _workConservingRateCalculator.calculateRate( _workConservingCounter.load( ) );

					// set it equal to temp variable
					workConservingRate = tempBandwidth;

					// last value, so calculate total rate
					tempBandwidth = bandwidthGuaranteeRate.load() + workConservingRate.load();
					totalRate = tempBandwidth;

					// we have what is necessary, so leave
					break;
				}
			}
		}
	}

	PRINT("bwg %u wc %u tot %u\n"
		   , bandwidthGuaranteeRate.load()
		   , workConservingRate.load()
		   , totalRate.load() );
}


} // namespace WCEnabler
