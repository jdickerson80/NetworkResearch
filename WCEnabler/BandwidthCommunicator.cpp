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
#include "WorkConservationFlowHandler.h"

BandwidthCommunicator::BandwidthCommunicator( const std::string& bGAdaptorAddress
											  , const std::string& interface
											  , BandwidthCalculator* bandwidthCalculator
											  , Common::LoggingHandler* bandwidthLimitLogger
											  , Common::LoggingHandler* bandwidthUsageLogger
											  , WorkConservationFlowHandler* workConservationFlowHandler )
	: _incomingBandwidthThreadRunning( false )
	, _outgoingBandwidthThreadRunning( false )
	, _bandwidthLimitLogger( bandwidthLimitLogger )
	, _bandwidthUsageLogger( bandwidthUsageLogger )
	, _interface( interface )
	, _bandwidthCalculator( bandwidthCalculator )
	, _workConservationFlowHandler( workConservationFlowHandler )
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
	Common::TCControl::clearTCCommands( _interface );
	delete _bandwidthLimitLogger;
	delete _bandwidthUsageLogger;
}

void* BandwidthCommunicator::handleIncomingBandwidthRequest( void* input )
{
	// init the variables
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	float bandwidth;
	unsigned int bandwidthLength = sizeof( bandwidth );
	unsigned int length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );
	unsigned int receiveLength;
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthLimitLogger;
	WorkConservationFlowHandler* workConservationFlowHandler = bandwidthCommunicator->_workConservationFlowHandler;
	BandwidthCalculator* bandwidthCalculator = bandwidthCommunicator->_bandwidthCalculator;

	// while the thread should run
	while ( bandwidthCommunicator->_incomingBandwidthThreadRunning )
	{
		// listen for incoming bandwidth rates
		receiveLength = recvfrom( bandwidthCommunicator->_socketFileDescriptor
								  , &bandwidth
								  , bandwidthLength
								  , 0
								  , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
								  , &length );

		// check for error
		if ( receiveLength < bandwidthLength )
		{
			continue;
		}

		// enforce the rate limit
		Common::TCControl::setEgressBandwidth( bandwidthCommunicator->_interface, bandwidth );

		// update the work conservation flow handler
		/// @note this is done this way to only check for subflows when a new rate is received
		/// @todo is this correct? should this be done in its own thread?
		workConservationFlowHandler->updateWorkConservation( bandwidthCalculator->bandwidthGuaranteeRate()
															 , bandwidthCalculator->workConservingRate()
															 , bandwidthCalculator->ecn()
															 , bandwidth );
		// create the stream
		std::ostringstream stream;
		stream << bandwidth << "\n";

		// log the bandwidth limit
		logger->log( stream.str() );
	}

	// clear the tc commands
	Common::TCControl::clearTCCommands( bandwidthCommunicator->_interface );
	pthread_exit( NULL );
	return NULL;
}

void* BandwidthCommunicator::handleOutgoingBandwidthRequest( void* input )
{
	// init the variables
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	BandwidthCalculator* bandwidthCalculator = bandwidthCommunicator->_bandwidthCalculator;
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthUsageLogger;
	size_t floatSize = sizeof( float );

	// while the thread should run
	while ( bandwidthCommunicator->_outgoingBandwidthThreadRunning )
	{
		// get the total rate
		float totalRate = bandwidthCalculator->totalRate();

		// send the current rate
		if ( sendto( bandwidthCommunicator->_socketFileDescriptor
					 , &totalRate
					 , floatSize
					 , 0
					 , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
					 , sizeof( bandwidthCommunicator->_bGAdaptorAddress ) ) < 0 )
		{
			printf("Send failed with %s\n", strerror( errno ));
		}

		printf("bwg %2.2f wc %2.2f tot %2.2f\n"
			   , bandwidthCalculator->bandwidthGuaranteeRate()
			   , bandwidthCalculator->workConservingRate()
			   , bandwidthCalculator->totalRate() );

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
