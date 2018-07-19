#include "BandwidthCommunicator.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>

#include <errno.h>

#include "TCControl.h"
#include "ThreadHelper.h"


BandwidthCommunicator::BandwidthCommunicator( const std::string& BGAdaptorAddress, const std::string& interface )
	: _incomingBandwidthThreadRunning( false )
	, _interface( interface )
{
	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket failed\n" );
	}

	inet_aton( BGAdaptorAddress.c_str(), &_bGAdaptorAddress.sin_addr );
	_bGAdaptorAddress.sin_family = AF_INET;
	_bGAdaptorAddress.sin_port = htons( 8888 );

	Common::ThreadHelper::startDetachedThread( &_incomingBandwidthThread, handleIncomingBandwidthRequest, &_incomingBandwidthThreadRunning, static_cast< void* >( this ) );

//	if ( connect( _socketFileDescriptor, (sockaddr*)&BGAdaptor, sizeof( BGAdaptor ) ) < 0 )
//	{
//		close( _socketFileDescriptor );
//		printf( "connection failed\n" );
//	}
}

BandwidthCommunicator::~BandwidthCommunicator()
{
	close( _socketFileDescriptor );
}

int BandwidthCommunicator::sendBandwidth( float rate )
{
	printf("sending %2.2f\n", rate );
	if ( sendto( _socketFileDescriptor, &rate, sizeof( rate ), 0, (sockaddr*)&_bGAdaptorAddress, sizeof( _bGAdaptorAddress ) ) < 0 )
	{
		printf("Send failed with %s\n", strerror( errno ));
		return 1;
	}

	return 0;
}

float BandwidthCommunicator::bandwidth()
{
	float bandwidth;
	unsigned int length = sizeof( _bGAdaptorAddress );
	recvfrom( _socketFileDescriptor, &bandwidth, sizeof( bandwidth ), 0, (sockaddr*)&_bGAdaptorAddress, (socklen_t*)&length );

	return bandwidth;
}

void BandwidthCommunicator::setBandwidthThreadRunning( bool isRunning )
{
	_incomingBandwidthThreadRunning = isRunning;
}

void* BandwidthCommunicator::handleIncomingBandwidthRequest( void* input )
{
	BandwidthCommunicator* bandwidthCommunicator = static_cast< BandwidthCommunicator* >( input );
	int receiveLength;
	float bandwidth;
	unsigned int length = sizeof( bandwidthCommunicator->_bGAdaptorAddress );

	while ( bandwidthCommunicator->_incomingBandwidthThreadRunning )
	{
		receiveLength = recvfrom( bandwidthCommunicator->_socketFileDescriptor, &bandwidth, sizeof( bandwidth ), 0, (sockaddr*)&bandwidthCommunicator->_bGAdaptorAddress, &length );
		Common::TCControl::setEgressBandwidth( bandwidthCommunicator->_interface, bandwidth, "50" );
		printf("setting bw %2.2f\n", bandwidth );
	}

	pthread_exit( NULL );
	return NULL;
}
