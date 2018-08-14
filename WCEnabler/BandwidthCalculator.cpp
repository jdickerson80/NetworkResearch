#include "BandwidthCalculator.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include "BandwidthValues.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "ThreadHelper.h"

// macros to avoid "magic numbers"
#define LogBufferSize ( 1024 )
#define PacketBufferSize ( 65536 )
#define IPAddressSize ( 20 )

//#define LogPackets ( 1 )

namespace WCEnabler {

BandwidthCalculator::BandwidthCalculator(
		Common::LoggingHandler* logger
		, const std::string& interfaceIPAddress
		, BandwidthValues* bandwidthValues )
	: _packetSniffingThreadRunning( false )
	, _calculationThreadRunning( false )
	, _bandwidthGuaranteeCounter( 0 )
	, _workConservingCounter( 0 )
	, _bandwidthValues( bandwidthValues )
	, _logger( logger )
	, _interfaceIPAddress( interfaceIPAddress )
{
	// create the raw socket that intercepts ALL packets
	_socketFileDescriptor = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );
//	int results = setsockopt( _socketFileDescriptor, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str(), strlen( interface.c_str() )+ 1 );
//	printf("sock opt %i\n", results );

	// check for socket fd error
	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket sniffer failed\n" );
	}

	// add the header to the log
	char buffer[ LogBufferSize ];
	snprintf( buffer, LogBufferSize, "protocol, source, destination, ecn\n" );
	_logger->log( buffer );

	// start both threads
	Common::ThreadHelper::startDetachedThread(
				&_packetSniffingThread
				, handlePacketSniffing
				, &_packetSniffingThreadRunning
				, static_cast< void* >( this ) );

	Common::ThreadHelper::startDetachedThread(
				&_calculationThread
				, handleRateCalculation
				, &_calculationThreadRunning
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

void BandwidthCalculator::updateBandwidthGuaranteeRate()
{
	// calculate the rates
	_bandwidthValues->bandwidthGuaranteeRate = _bandwidthGuaranteeRateCalculator.calculateRate( _bandwidthGuaranteeCounter );
}

void BandwidthCalculator::updateWorkConservingRate()
{
	// calculate the rates
	_bandwidthValues->workConservingRate = _workConservingRateCalculator.calculateRate( _workConservingCounter );
}

void BandwidthCalculator::updateTotalBandwidthRate()
{
	// calculate the rates
	_bandwidthValues->totalRate = _bandwidthValues->bandwidthGuaranteeRate + _bandwidthValues->workConservingRate;
}

void* BandwidthCalculator::handlePacketSniffing( void* input )
{
	// init the variables
	BandwidthCalculator* bandwidthCalculator = static_cast< BandwidthCalculator* >( input );
	sockaddr socketAddress;
	ssize_t dataSize;
	int socketAddressLength = sizeof( socketAddress );
	unsigned int* bandwidthGuaranteeCounter = &bandwidthCalculator->_bandwidthGuaranteeCounter;
	unsigned int* workConservingCounter = &bandwidthCalculator->_workConservingCounter;
	unsigned int* ecn = &bandwidthCalculator->_bandwidthValues->ecnValue;
	unsigned int localECN = 0;
	unsigned char packetBuffer[ PacketBufferSize ];
	Common::LoggingHandler* logger = bandwidthCalculator->_logger;
	uint8_t dscpValue;
	char sourceAddress[ IPAddressSize ];
	char destinationAddress[ IPAddressSize ];
	iphdr* ipHeader;
	const char* const ipAddress = bandwidthCalculator->_interfaceIPAddress.c_str();

#if defined( LogPackets )
	char logBuffer[ LogBufferSize ];
	ethhdr* ethernetHeader;
#endif

	// while the thread should run
	while ( bandwidthCalculator->_packetSniffingThreadRunning )
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
//			printf("Err %s\n", strerror( errno ) );
			continue;
		}

		// get the appropriate headers
		ipHeader = (iphdr*)( packetBuffer + sizeof( ethhdr ) );

		// put the addresses into the string
		// @note This HAS to be done sequentially and stored because the inet_ntoa char* buffer
		// will be overridden every time it is called. This will make both addresses the
		// same.
		snprintf( destinationAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->daddr ) ) );
		snprintf( sourceAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->saddr ) ) );

		// if the packet source is not this interface
		if ( strcmp( sourceAddress, ipAddress ) )
		{
			// mask out the DSCP field and check if congestion has been encountered
			localECN = ( ipHeader->tos & INET_ECN_MASK ) == INET_ECN_CE;
			(*ecn) = localECN;

			if ( *ecn == true )
			{
//				printf("GOT ECN!!!!!\n");
			}
		}
		else // packet source is this interface
		{
			// convert the bytes to bits
			dataSize *= 8;

			// mask the ecn field out to yield only the DSCP field
			dscpValue = ipHeader->tos & WorkConvervationMask;

			// determine if the packet is from the work conservation flow
			if ( dscpValue == WorkConservationFlowDecimalForm )
			{
				// add the packet size to the counter
				(*workConservingCounter) += dataSize;
			}
			else // the packet is from the bandwidth guarantee flow
			{
				//if ( ipHeader->tos == 0 ) // does the if need to be there??
				// add the packet size to the counter
				(*bandwidthGuaranteeCounter) += dataSize;
			}
		}

#if defined( LogPackets )
		// set up the logging string
		ethernetHeader = (ethhdr*)packetBuffer;
		snprintf( logBuffer, LogBufferSize, "%u, %s, %s, %u\n"
				  , ethernetHeader->h_proto
				  , sourceAddress
				  , destinationAddress
				  , *ecn );

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

	// while the thread should run
	while ( bandwidthCalculator->_calculationThreadRunning )
	{
		// update the bandwidth calculation every second
		bandwidthCalculator->updateBandwidthGuaranteeRate();
		bandwidthCalculator->updateWorkConservingRate();
		bandwidthCalculator->updateTotalBandwidthRate();

		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}

} // namespace WCEnabler