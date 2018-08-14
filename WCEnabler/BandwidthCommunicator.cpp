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
#include "BandwidthValues.h"
#include "LoggingHandler.h"
#include "TCControl.h"
#include "ThreadHelper.h"

namespace WCEnabler {
BandwidthCommunicator::BandwidthCommunicator(
		const std::string& bGAdaptorAddress
		, const std::string& interface
		, BandwidthValues* bandwidthValues
		, Common::LoggingHandler* bandwidthLimitLogger
		, Common::LoggingHandler* bandwidthUsageLogger )
	: _incomingBandwidthThreadRunning( false )
	, _outgoingBandwidthThreadRunning( false )
	, _currentBandwidthGuarantee( 0 )
	, _bandwidthLimitLogger( bandwidthLimitLogger )
	, _bandwidthUsageLogger( bandwidthUsageLogger )
	, _bandwidthValues( bandwidthValues )
	, _interface( interface )
{
	// create the udp socket to communicate with the BGAdaptor
	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket communcation failed\n" );
	}

	// set up the BGAdaptor's address
	inet_aton( bGAdaptorAddress.c_str(), &_bGAdaptorAddress.sin_addr );
	_bGAdaptorAddress.sin_family = AF_INET;
	_bGAdaptorAddress.sin_port = htons( 8888 );

	// start the sending and receiving threads
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
	// shut off all of the threads, close the socket, and delete the loggers
	_incomingBandwidthThreadRunning = false;
	_outgoingBandwidthThreadRunning = false;
	close( _socketFileDescriptor );
	delete _bandwidthLimitLogger;
	delete _bandwidthUsageLogger;
	Common::TCControl::clearTCCommands( _interface );
}

void* BandwidthCommunicator::handleIncomingBandwidthRequest( void* input )
{
	// init the variables
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	unsigned int localBandwidthGuarantee;
	unsigned int* bandwidthGuarantee = &bandwidthCommunicator->_bandwidthValues->bandwidthGuarantee;
	unsigned int bandwidthLength = sizeof( localBandwidthGuarantee );
	unsigned int length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );
	unsigned int receiveLength;
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthLimitLogger;

	// while the thread should run
	while ( bandwidthCommunicator->_incomingBandwidthThreadRunning )
	{
		// listen for incoming bandwidth rates
		receiveLength = recvfrom( bandwidthCommunicator->_socketFileDescriptor
								  , &localBandwidthGuarantee
								  , bandwidthLength
								  , 0
								  , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
								  , &length );

		/// @todo add checking of address to make sure it is coming from BGAdaptor
		/// instead of some random sender

		// check for error
		if ( receiveLength != bandwidthLength )
		{
			printf("!!!!!receive error\n");
			continue;
		}

		(*bandwidthGuarantee) = localBandwidthGuarantee;

		// enforce the rate limit
		Common::TCControl::setEgressBandwidth( bandwidthCommunicator->_interface, localBandwidthGuarantee );

		// create the stream
		std::ostringstream stream;
		stream << localBandwidthGuarantee << "\n";

		// log the bandwidth limit
		logger->log( stream.str() );
	}

	pthread_exit( NULL );
	return NULL;
}

void* BandwidthCommunicator::handleOutgoingBandwidthRequest( void* input )
{
	// init the variables
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	unsigned int* totalRate = &bandwidthCommunicator->_bandwidthValues->totalRate;
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthUsageLogger;
	size_t bandwidthSize = sizeof( unsigned int );
	unsigned int localTotalRate;

	// while the thread should run
	while ( bandwidthCommunicator->_outgoingBandwidthThreadRunning )
	{
		// get the total rate
		localTotalRate = *totalRate;

		// send the current rate
		if ( sendto( bandwidthCommunicator->_socketFileDescriptor
					 , &localTotalRate
					 , bandwidthSize
					 , 0
					 , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
					 , sizeof( bandwidthCommunicator->_bGAdaptorAddress ) ) < 0 )
		{
			printf("Send failed with %s\n", strerror( errno ));
		}

		/// @todo fix the sendto function!!

		// create the stream
		std::ostringstream stream;
		stream << totalRate << "\n";

		// log the bandwidth limit
		logger->log( stream.str() );

		// send the bandwidth rate every second
		sleep( 1 );
	}

	pthread_exit( NULL );
	return NULL;
}

} // namespace WCEnabler
