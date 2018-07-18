#include "HandleHostBandwidth.h"
#include "ThreadHelper.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <errno.h>

#define IPBytes ( 4 )

HandleHostBandwidth::HandleHostBandwidth( unsigned int numberOfHosts )
	: _isRunning( false )
	, _numberOfHosts( numberOfHosts )
{
	_hostsBandwidth = new HostBandwidthStatistics[ numberOfHosts ];

	for ( size_t i = 0; i < _numberOfHosts; ++i )
	{
		_hostsBandwidth[ i ].address.sin_family = AF_INET;
		_hostsBandwidth[ i ].address.sin_port = htons( 8888 );
	}

	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket failed\n" );
	}

	_localAddresses.sin_addr.s_addr = htonl( INADDR_ANY );
	_localAddresses.sin_family = AF_INET;
	_localAddresses.sin_port = htons( 8888 );

	if ( bind( _socketFileDescriptor, (struct sockaddr*)&_localAddresses, sizeof( _localAddresses ) ) == -1 )
	{
		printf( "bind failed\n" );
	}

	Common::ThreadHelper::startDetachedThread( &_receiveThread, handleConnections, &_isRunning, static_cast< void* >( this ) );
}

HandleHostBandwidth::~HandleHostBandwidth()
{
	close( _socketFileDescriptor );
	delete _hostsBandwidth;
}

void HandleHostBandwidth::printBandwidths() const
{
	for ( size_t i = 0; i < _numberOfHosts; ++i )
	{
		printf("%2.2f, ", _hostsBandwidth[ i ].demand );
	}

	printf( "\n" );
}

void HandleHostBandwidth::setRunning( bool isRunning )
{
	_isRunning = isRunning;
}

void* HandleHostBandwidth::handleConnections( void* input )
{
	HandleHostBandwidth* bandwidthManager = static_cast< HandleHostBandwidth* >( input );
	float bandwidth;
	int receiveLength;
	unsigned int length = sizeof( bandwidthManager->_localAddresses );
	sockaddr_in receiveAddress;
	size_t index;

	while ( bandwidthManager->_isRunning )
	{
		receiveLength = recvfrom( bandwidthManager->_socketFileDescriptor, &bandwidth, sizeof( bandwidth ), 0, (sockaddr*)&receiveAddress, &length );
		index = findHostIndex( inet_ntoa( receiveAddress.sin_addr ) );
		bandwidthManager->_hostsBandwidth[ index ].demand = bandwidth;
		bandwidthManager->_hostsBandwidth[ index ].guarantee = 20.0f;
		bandwidthManager->_hostsBandwidth[ index ].address = receiveAddress;
//		printf("got %i packets %f from %s ", receiveLength, bandwidth, inet_ntoa( receiveAddress.sin_addr ) );
//		printf( "index: %lu\n", findHostIndex( inet_ntoa( receiveAddress.sin_addr ) ) );
	}

	pthread_exit( NULL );
	return NULL;
}

size_t HandleHostBandwidth::findHostIndex( char* ipAddress )
{
	//	size_t dotPosition;
	//	std::string stringAddress( address );
	//	dotPosition = stringAddress.rfind( '.' );

	//	printf("found dot at %lu\n", dotPosition );

	uint8_t ipbytes[ IPBytes ] = {};
	int i = 0;
	int8_t j = 3;
	size_t nonZeroPosition;

	while ( ipAddress + i && i < strlen( ipAddress ) )
	{
		char digit = ipAddress[ i ];
		if ( isdigit( digit ) == 0 && digit != '.' )
		{
			return 0;
		}
		j = digit == '.' ? j - 1 : j;
		ipbytes[ j ]= ipbytes[ j ] * 10 + atoi( &digit );

		i++;
	}

	for ( int q = 0; q < IPBytes; ++q )
	{
//		printf( "%u ", ipbytes[ q ] );
	}

//	printf("!!!!");
	for ( int q = IPBytes - 2; q > 0; --q )
	{
//		printf( "%u ", ipbytes[ q ] );
		if ( ipbytes[ q ] != 0 )
		{
			nonZeroPosition = q;
			break;
		}
	}


	size_t index = ipbytes[ 0 ];
	return index - 2;
//	printf("\n");


}

void HandleHostBandwidth::sendBandwidthRates() const
{
	float rate;
	size_t size = sizeof( _hostsBandwidth[ 0 ].address );

	for ( unsigned int q = 0; q < _numberOfHosts; ++q )
	{
		rate = _hostsBandwidth[ q ].guarantee;

//		printf("sending %f to %lu\n", rate, q );

		if ( sendto( _socketFileDescriptor, &rate, sizeof( rate ), 0, (sockaddr*)&_hostsBandwidth[ q ].address, size ) <= 0 )
		{
			printf("Send failed with %s\n", strerror( errno ));
//			return 1;
		}
	}
}
