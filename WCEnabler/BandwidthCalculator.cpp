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

#include "gen_stats.h"
#include "rtnetlink.h"
#include "libnetlink.h"
#include <param.h>
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
	std::atomic_uint& bandwidthGuaranteeCounter = bandwidthCalculator->_bandwidthGuaranteeCounter;
	std::atomic_uint& workConservingCounter = bandwidthCalculator->_workConservingCounter;
	std::atomic_uint& ecn = bandwidthCalculator->_bandwidthValues->ecnValue;
	unsigned int localECN = 0;
	unsigned char packetBuffer[ PacketBufferSize ];
	Common::LoggingHandler* logger = bandwidthCalculator->_logger;
	uint8_t dscpValue;
	char sourceAddress[ IPAddressSize ];
	char destinationAddress[ IPAddressSize ];
	iphdr* ipHeader;
	in_addr ipAddress;
	ssize_t ethhdrSize = sizeof( ethhdr );
	inet_aton( bandwidthCalculator->_interfaceIPAddress.c_str(), &ipAddress );

#if defined( LogPackets )
	char logBuffer[ LogBufferSize ];
	ethhdr* ethernetHeader;
#endif
		// get the appropriate headers
		ipHeader = (iphdr*)( packetBuffer + ethhdrSize );

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
//			printf("Err %s\n", strerror( errno ) );
			continue;
		}


		// put the addresses into the string
		// @note This HAS to be done sequentially and stored because the inet_ntoa char* buffer
		// will be overridden every time it is called. This will make both addresses the
		// same.
//		snprintf( destinationAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->daddr ) ) );
//		snprintf( sourceAddress, IPAddressSize, "%s", inet_ntoa( *( (in_addr*)&ipHeader->saddr ) ) );

		// if packet source is this interface
		if ( ipHeader->saddr == ipAddress.s_addr )
		{
			// convert the bytes to bits
			dataSize *= 8;

			// mask the ecn field out to yield only the DSCP field
			dscpValue = ipHeader->tos & WorkConvervationMask;

			// determine if the packet is from the work conservation flow
			if ( dscpValue == WorkConservationFlowDecimalForm )
			{
				// add the packet size to the counter
				workConservingCounter += dataSize;
				continue;
			}
//			else // the packet is from the bandwidth guarantee flow
			{
				//if ( ipHeader->tos == 0 ) // does the if need to be there??
				// add the packet size to the counter
				bandwidthGuaranteeCounter += dataSize;
			}
			continue;
		}
//		else // if the packet source is not this interface
		{
			// mask out the DSCP field and check if congestion has been encountered
			localECN = ( ipHeader->tos & INET_ECN_MASK ) == INET_ECN_CE;
			ecn = localECN;

			if ( ecn.load() == true )
			{
				printf("GOT ECN!!!!!\n");
			}
		}

		struct rtattr *tbs[TCA_STATS_MAX + 1];

		parse_rtattr_nested(tbs, TCA_STATS_MAX, rta);

		struct gnet_stats_rate_est re = {0};
		memcpy(&re, RTA_DATA(tbs[TCA_STATS_RATE_EST]), MIN(RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST]), sizeof(re)));
//		fprintf(fp, "\n%srate %s %upps ",
//			prefix, sprint_rate(re.bps, b1), re.pps);

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
	std::atomic_bool& threadRunning = bandwidthCalculator->_calculationThreadRunning;

	std::atomic_uint& bandwidthGuaranteeRate = bandwidthCalculator->_bandwidthValues->bandwidthGuaranteeRate;
	std::atomic_uint& bandwidthGuaranteeCounter = bandwidthCalculator->_bandwidthGuaranteeCounter;

	std::atomic_uint& workConservingRate = bandwidthCalculator->_bandwidthValues->workConservingRate;
	std::atomic_uint& workConservingCounter = bandwidthCalculator->_workConservingCounter;

	std::atomic_uint& totalRate = bandwidthCalculator->_bandwidthValues->totalRate;

	RateCalculator* bandwidthGuaranteeRateCalculator = &bandwidthCalculator->_bandwidthGuaranteeRateCalculator;
	RateCalculator* workConservingRateCalculator = &bandwidthCalculator->_workConservingRateCalculator;
	unsigned int tempBWGValue;
	unsigned int tempWCValue;

	// while the thread should run
	while ( threadRunning.load() )
	{
		// calculate temp rate and atomically set it equal to the actual value
		tempBWGValue = bandwidthGuaranteeRateCalculator->calculateRate( bandwidthGuaranteeCounter.load() );
		bandwidthGuaranteeRate = tempBWGValue;

		// calculate temp rate and atomically set it equal to the actual value
		tempWCValue = workConservingRateCalculator->calculateRate( workConservingCounter.load() );
		workConservingRate = tempWCValue;

		// calculate temp rate and atomically set it equal to the actual value
		totalRate = tempBWGValue + tempWCValue;

		printf("bwg %u wc %u tot %u\n"
			   , tempBWGValue
			   , tempWCValue
			   , totalRate.load() );


		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}

} // namespace WCEnabler
