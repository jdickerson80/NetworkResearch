#include "BandwidthCommunicator.h"

#include <errno.h>
#include <linux/tcp.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

#include "BandwidthCalculator.h"
#include "LoggingHandler.h"
#include "TCControl.h"
#include "ThreadHelper.h"

BandwidthCommunicator::BandwidthCommunicator( const std::string& bGAdaptorAddress
											  , const std::string& interface
											  , BandwidthCalculator* bandwidthCalculator
											  , Common::LoggingHandler* logger   )
	: _incomingBandwidthThreadRunning( false )
	, _outgoingBandwidthThreadRunning( false )
	, _logger( logger )
	, _interface( interface )
	, _bandwidthCalculator( bandwidthCalculator )
{
	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket communcation failed\n" );
	}

//	int hdrincl = 1;
//	if ( setsockopt( _socketSniffingFileDescriptor, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1)
//	{
//		perror("Failed to set IP_HDRINCL");
////		exit(1);
//	}

	inet_aton( bGAdaptorAddress.c_str(), &_bGAdaptorAddress.sin_addr );
	_bGAdaptorAddress.sin_family = AF_INET;
	_bGAdaptorAddress.sin_port = htons( 8888 );

	unsigned char set = 1;
	int ret = setsockopt( _socketFileDescriptor, IPPROTO_IP, IP_RECVTOS, &set, sizeof( set ) );
	if ( ret == -1 )
	{
		printf("setsockopt()");
		close( _socketFileDescriptor );
	}


	std::ostringstream stream;
	stream << "\n\n\n\nSRC:\t," << " DST:\t," << "PROTO:\t" << " TOS:" << "\n";

	_logger->log( stream.str() );


	Common::ThreadHelper::startDetachedThread( &_incomingBandwidthThread
											   , handleIncomingBandwidthRequest
											   , &_incomingBandwidthThreadRunning
											   , static_cast< void* >( this ) );

	Common::ThreadHelper::startDetachedThread( &_outgoingBandwidthThread
											   , handleOutgoingBandwidthRequest
											   , &_outgoingBandwidthThreadRunning
											   , static_cast< void* >( this ) );
}

BandwidthCommunicator::~BandwidthCommunicator()
{
	_incomingBandwidthThreadRunning = false;
	_outgoingBandwidthThreadRunning = false;
	close( _socketFileDescriptor );
	Common::TCControl::clearTCCommands( _interface );
	delete _logger;
}

void BandwidthCommunicator::setBandwidthThreadRunning( bool isRunning )
{
	if ( _incomingBandwidthThreadRunning == isRunning )
	{
		return;
	}

	if ( isRunning )
	{
		Common::ThreadHelper::startDetachedThread( &_incomingBandwidthThread
												   , handleIncomingBandwidthRequest
												   , &_incomingBandwidthThreadRunning
												   , static_cast< void* >( this ) );
	}
	else
	{
		_incomingBandwidthThreadRunning = isRunning;
	}
}

void* BandwidthCommunicator::handleIncomingBandwidthRequest( void* input )
{
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	float bandwidth;
	unsigned int bandwidthLength = sizeof( bandwidth );
	unsigned int length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );
	unsigned int receiveLength;
	unsigned int counter = 0;
	bool backup = true;

	while ( bandwidthCommunicator->_incomingBandwidthThreadRunning )
	{
		receiveLength = recvfrom( bandwidthCommunicator->_socketFileDescriptor
								  , &bandwidth
								  , sizeof( bandwidth )
								  , 0
								  , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
								  , &length );

		if ( receiveLength < bandwidthLength )
		{
			continue;
		}

		Common::TCControl::setEgressBandwidth( bandwidthCommunicator->_interface, bandwidth );

//		++counter;

//		if ( counter % 20 == 0 )
//		{

//			backup = !backup;
//		}
//		printf("setting bw %2.2f\n", bandwidth );
	}

	Common::TCControl::clearTCCommands( bandwidthCommunicator->_interface );
	pthread_exit( NULL );
	return NULL;
}

void* BandwidthCommunicator::handleOutgoingBandwidthRequest( void* input )
{
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	BandwidthCalculator* bandwidthCalculator = bandwidthCommunicator->_bandwidthCalculator;
	size_t floatSize = sizeof( float );

	while ( bandwidthCommunicator->_outgoingBandwidthThreadRunning )
	{
		float totalRate = bandwidthCalculator->totalRate();

		if ( sendto( bandwidthCommunicator->_socketFileDescriptor, &totalRate, floatSize, 0, (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress, sizeof( bandwidthCommunicator->_bGAdaptorAddress ) ) < 0 )
		{
			printf("Send failed with %s\n", strerror( errno ));
		}

		printf("bg %f wc %f out %f\n"
			   , bandwidthCalculator->bandwidthGuaranteeRate()
			   , bandwidthCalculator->workConservingRate()
			   , totalRate );
		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}


