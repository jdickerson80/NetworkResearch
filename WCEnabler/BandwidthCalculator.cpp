#include "BandwidthCalculator.h"

#include <arpa/inet.h>
#include <netinet/ip.h>

#include <errno.h>
#include <netinet/if_ether.h>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include "LoggingHandler.h"
#include "Macros.h"
#include "ThreadHelper.h"

//#define BufferSize ( 65536 )
#define BufferSize ( 1024 )

BandwidthCalculator::BandwidthCalculator( Common::LoggingHandler* logger )
	: _packetSniffingThreadRunning( false )
	, _calculationThreadRunning( false )
	, _bandwidthGuaranteeRate( 0 )
	, _workConservingRate( 0 )
	, _totalRate( 0 )
	, _bandwidthGuaranteeCounter( 0 )
	, _workConservingCounter( 0 )
	, _logger( logger )
{
	// create the raw socket that intercepts ALL packets
	_socketFileDescriptor = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket sniffer failed\n" );
	}

	// start both threads
	Common::ThreadHelper::startDetachedThread( &_packetSniffingThread
											   , handlePacketSniffing
											   , &_packetSniffingThreadRunning
											   , static_cast< void* >( this ) );

	Common::ThreadHelper::startDetachedThread( &_calculationThread
											   , handleRateCalculation
											   , &_calculationThreadRunning
											   , static_cast< void* >( this ) );
}

BandwidthCalculator::~BandwidthCalculator()
{
	_calculationThreadRunning = false;
	_packetSniffingThreadRunning = false;
	close( _socketFileDescriptor );
	delete _logger;
}

float BandwidthCalculator::bandwidthGuaranteeRate() const
{
	return _bandwidthGuaranteeRate;
}

float BandwidthCalculator::workConservingRate() const
{
	return _workConservingRate;
}

float BandwidthCalculator::totalRate() const
{
	return _totalRate;
}

void BandwidthCalculator::updateBandwidthGuaranteeRate()
{
	_bandwidthGuaranteeRate = _bandwidthGuaranteeRateCalculator.calculateRate( _bandwidthGuaranteeCounter );
}

void BandwidthCalculator::updateWorkConservingRate()
{
	_workConservingRate = _workConservingRateCalculator.calculateRate( _workConservingCounter );
}

void BandwidthCalculator::updateTotalBandwidthRate()
{
	_totalRate = _bandwidthGuaranteeRate + _workConservingRate;
}

void* BandwidthCalculator::handlePacketSniffing( void* input )
{
	BandwidthCalculator* bandwidthCalculator = static_cast< BandwidthCalculator* >( input );
	sockaddr socketAddress;
	int socketAddressLength = sizeof( socketAddress );
	int dataSize;
	unsigned int* bandwidthGuaranteeCounter = &bandwidthCalculator->_bandwidthGuaranteeCounter;
	unsigned int* workConservingCounter = &bandwidthCalculator->_workConservingCounter;
	unsigned char buffer[ BufferSize ];
	Common::LoggingHandler* logger = bandwidthCalculator->_logger;

	while ( bandwidthCalculator->_packetSniffingThreadRunning )
	{
		dataSize = recvfrom( bandwidthCalculator->_socketFileDescriptor, buffer, BufferSize, 0, &socketAddress, (socklen_t*)&socketAddressLength );

		if ( dataSize < 0 )
		{
			printf("Err %s\n", strerror( errno ) );
		}

//		printf( "fam %i, data %s\n", socketAddress.sa_family, socketAddress.sa_data );
		iphdr* ipHeader = (iphdr*)( buffer + sizeof( ethhdr ) );

		if ( ipHeader->tos == WorkConservationFlowDecimalForm )
		{
			++(*workConservingCounter);
//			printf("wc %u\n", *workConservingCounter );
		}
		else //if ( ipHeader->tos == 0 ) // does the if need to be there??
		{
			++(*bandwidthGuaranteeCounter);
//			printf("bg %u\n", *bandwidthGuaranteeCounter );
		}

//		printf("bg %f wc %f tot %f\n"
//			   , bandwidthCalculator->bandwidthGuaranteeRate()
//			   , bandwidthCalculator->workConservingRate()
//			   , bandwidthCalculator->totalRate() );

//		logger->log( getLoggingString( ipHeader) );
	}

	pthread_exit( NULL );
	return NULL;
}

void* BandwidthCalculator::handleRateCalculation( void* input )
{
	BandwidthCalculator* bandwidthCalculator = static_cast< BandwidthCalculator* >( input );

	while ( bandwidthCalculator->_calculationThreadRunning )
	{
		bandwidthCalculator->updateBandwidthGuaranteeRate();
		bandwidthCalculator->updateWorkConservingRate();
		bandwidthCalculator->updateTotalBandwidthRate();

		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}

std::string BandwidthCalculator::getLoggingString( iphdr* ipHeader )
{
	std::ostringstream stream;
	stream << inet_ntoa( *((in_addr*)&ipHeader->saddr ) ) << ", "
			<< inet_ntoa( *((in_addr*)&ipHeader->daddr) ) << ", "
			<< (unsigned int)ipHeader->protocol << ", "
			<< ( (unsigned int)ipHeader->tos /*>> 6*/ )<< "\n";
	return stream.str();
}
