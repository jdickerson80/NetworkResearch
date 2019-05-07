#include "BandwidthCommunicator.h"

#include <errno.h>
#include <linux/tcp.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>

#include "BandwidthCalculator.h"
#include "BandwidthValues.h"
#include "LoggingHandler.h"
#include "TCControl.h"
#include "ThreadHelper.h"
#include "WCPrintHandler.h"

#define LogBufferSize ( 1024 )

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
		PRINT( "socket communcation failed\n" );
	}

	// set up the BGAdaptor's address
	inet_aton( bGAdaptorAddress.c_str(), &_bGAdaptorAddress.sin_addr );
	_bGAdaptorAddress.sin_family = AF_INET;
	_bGAdaptorAddress.sin_port = htons( 8888 );

	// start the sending and receiving threads
	Common::ThreadHelper::startDetachedThread( &_incomingBandwidthThread
											   , handleIncomingBandwidthRequest
											   , _incomingBandwidthThreadRunning
											   , static_cast< void* >( this ) );

	Common::ThreadHelper::startDetachedThread( &_outgoingBandwidthThread
											   , handleOutgoingBandwidthRequest
											   , _outgoingBandwidthThreadRunning
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
	std::atomic_bool& threadRunning = bandwidthCommunicator->_incomingBandwidthThreadRunning;
	unsigned int localBandwidthGuarantee;
	std::atomic_uint& bandwidthGuarantee = bandwidthCommunicator->_bandwidthValues->bandwidthGuarantee;
	unsigned int bandwidthLength = sizeof( localBandwidthGuarantee );
	socklen_t length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );
	unsigned int receiveLength;
	char logBuffer[ LogBufferSize ];
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthLimitLogger;

	// while the thread should run
	while ( threadRunning.load() )
	{
		// listen for incoming bandwidth rates
		receiveLength = recvfrom(
					bandwidthCommunicator->_socketFileDescriptor
					, &localBandwidthGuarantee
					, bandwidthLength
					, 0
					, (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
					, &length );

		/// @todo add checking of address to make sure it is coming from BGAdaptor
		/// instead of some random sender

//		PRINT("!!!!!%u %u\n", localBandwidthGuarantee, bandwidthGuarantee.load() );
		// check for error
		if ( receiveLength != bandwidthLength )
		{
			PRINT("!!!!!receive error\n");
			continue;
		}

//		PRINT("%u\n");
		if ( bandwidthGuarantee.load() != localBandwidthGuarantee )
		{
			// set the values equal
			bandwidthGuarantee = localBandwidthGuarantee;

			// enforce the rate limit
			Common::TCControl::setEgressBandwidth( bandwidthCommunicator->_interface, localBandwidthGuarantee );
		}

//		PRINT("&&&&&&&&&!!!!!@@@@@@\n");
		// create the stream
		snprintf( logBuffer, LogBufferSize, "%u\n"
				  , localBandwidthGuarantee );

		// log the bandwidth limit
		logger->log( logBuffer );
	}

	pthread_exit( nullptr );
}

void* BandwidthCommunicator::handleOutgoingBandwidthRequest( void* input )
{
	// init the variables
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	std::atomic_bool& threadRunning = bandwidthCommunicator->_outgoingBandwidthThreadRunning;
	std::atomic_uint& bwgRate = bandwidthCommunicator->_bandwidthValues->bandwidthGuaranteeRate;
	std::atomic_uint& totalRate = bandwidthCommunicator->_bandwidthValues->totalRate;
	std::atomic_uint& wcRate  = bandwidthCommunicator->_bandwidthValues->workConservingRate;
	Common::LoggingHandler* logger = bandwidthCommunicator->_bandwidthUsageLogger;
	char logBuffer[ LogBufferSize ];
	size_t bandwidthSize = sizeof( unsigned int );
	socklen_t length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );
	unsigned int localBWGRate;
	unsigned int localTotalRate;
	unsigned int localWCRate;

	// while the thread should run
	while ( threadRunning.load() )
	{
		// get the total rate
		localBWGRate	= bwgRate.load();
		localTotalRate	= totalRate.load();
		localWCRate		= wcRate.load();

		// send the current rate
		if ( sendto( bandwidthCommunicator->_socketFileDescriptor
					 , &localTotalRate
					 , bandwidthSize
					 , 0
					 , (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress
					 , length ) < 0 )
		{
			PRINT("Send failed with %s\n", strerror( errno ));
		}

		// create the stream
		snprintf( logBuffer, LogBufferSize, "%lu, %u, %u, %u\n"
				  , clock()
				  , localBWGRate
				  , localWCRate
				  , localTotalRate );

//		PRINT( "%s", logBuffer );
		// log the bandwidth limit
		logger->log( logBuffer );

//		std::ostringstream stream;
//		stream << clock() << ", " << localBWGRate << ", " << localWCRate << ", " << localTotalRate << "\n";

//		logger->log( stream.str() );

		// send the bandwidth rate every second
		usleep( 250000 );
	}

	pthread_exit( nullptr );
}

} // namespace WCEnabler
