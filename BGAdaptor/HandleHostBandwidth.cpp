#include "HandleHostBandwidth.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sstream>

#include "ThreadHelper.h"

#define IPBytes ( 4 )

HandleHostBandwidth::HandleHostBandwidth( unsigned int numberOfHosts, unsigned int totalBandwidth )
	: _isRunning( false )
	, _logger( "/tmp/h1/bandwidth.log" )
	, _totalBandwidth( totalBandwidth )
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
//		_logger.log( "socket failed\n" );
		printf( "socket failed\n" );
	}

	_localAddresses.sin_addr.s_addr = htonl( INADDR_ANY );
	_localAddresses.sin_family = AF_INET;
	_localAddresses.sin_port = htons( 8888 );

	if ( bind( _socketFileDescriptor, (sockaddr*)&_localAddresses, sizeof( _localAddresses ) ) == -1 )
	{
//		_logger.log( "bind failed\n" );
		printf( "bind failed\n" );
	}

	Common::ThreadHelper::startDetachedThread( &_receiveThread, handleConnections, &_isRunning, static_cast< void* >( this ) );
}

HandleHostBandwidth::~HandleHostBandwidth()
{
	if ( _isRunning )
	{
		pthread_cancel( _receiveThread );
	}

	close( _socketFileDescriptor );
	delete _hostsBandwidth;
}

void HandleHostBandwidth::logBandwidths()
{
//	FILE* file = fopen( "/tmp/h1/bandwidth.log", "a" );
//	printf("file = %p\n", file );
	std::stringstream stream;

	for ( size_t i = 0; i < _numberOfHosts; ++i )
	{
//		fprintf( file, "%2.2f, ", _hostsBandwidth[ i ].demand );
		stream << _hostsBandwidth[ i ].demand << ",";
	}

//	fprintf( file, "\n");

//	fclose( file );
	stream << "\n";
	_logger.log( stream.str() );
}

void HandleHostBandwidth::setRunning( bool isRunning )
{
	if ( _isRunning == isRunning )
	{
		return;
	}

	if ( isRunning )
	{
		Common::ThreadHelper::startDetachedThread( &_receiveThread
												   , handleConnections
												   , &_isRunning
												   , static_cast< void* >( this ) );
	}
	else
	{
		_isRunning = isRunning;
	}
}

void* HandleHostBandwidth::handleConnections( void* input )
{
	HandleHostBandwidth* bandwidthManager = static_cast< HandleHostBandwidth* >( input );
	float bandwidth;
	unsigned int bandwidthLength = sizeof( bandwidth );
	unsigned int receiveLength;
	unsigned int length = sizeof( bandwidthManager->_localAddresses );
	sockaddr_in receiveAddress;
	size_t index;
	HostBandwidthStatistics* hostsBandwidth = bandwidthManager->_hostsBandwidth;
	HostBandwidthStatistics* localStatistics;
	char* address;

	while ( bandwidthManager->_isRunning )
	{
		receiveLength = recvfrom( bandwidthManager->_socketFileDescriptor
								  , &bandwidth
								  , bandwidthLength
								  , 0
								  , (sockaddr*)&receiveAddress
								  , &length );

		if ( receiveLength < bandwidthLength )
		{
			printf("continued\n");
			continue;
		}

		address = inet_ntoa( receiveAddress.sin_addr );
		if ( strcmp( address, "127.0.0.1" ) == 0 )
		{
			continue;
		}
		index = findHostIndex( address );
		localStatistics = &hostsBandwidth[ index ];
		localStatistics->lastDemand = localStatistics->demand;
		localStatistics->demand = bandwidth;
		localStatistics->address = receiveAddress;
//		printf("index %lu band %f\n", index, bandwidth );
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
//	size_t nonZeroPosition;

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
//	for ( int q = IPBytes - 2; q > 0; --q )
//	{
////		printf( "%u ", ipbytes[ q ] );
//		if ( ipbytes[ q ] != 0 )
//		{
//			nonZeroPosition = q;
//			break;
//		}
//	}


	size_t index = ipbytes[ 0 ];
//	printf("add %s IND %lu ", ipAddress, index - 2);
	return index - 2;
//	printf("\n");


}

void HandleHostBandwidth::calculateHostBandwidth( HostBandwidthStatistics* hostStatistics )
{
	hostStatistics->lastGuarantee = hostStatistics->guarantee;
	hostStatistics->guarantee = _totalBandwidth / _numberOfHosts;
}

void HandleHostBandwidth::sendBandwidthRates()
{
	float rate;
	size_t rateSize = sizeof( rate );
	size_t size = sizeof( _hostsBandwidth[ 0 ].address );
	std::stringstream stream;

	for ( unsigned int q = 0; q < _numberOfHosts; ++q )
	{
		calculateHostBandwidth( &_hostsBandwidth[ q ] );
		rate = _hostsBandwidth[ q ].guarantee;

//		printf( "%f\n", rate );
//		stream << "per host guarantee is " << rate << std::endl;
//		_logger.log( stream.str() );
//		stream.flush();

		if ( sendto( _socketFileDescriptor, &rate, rateSize, 0, (sockaddr*)&_hostsBandwidth[ q ].address, size ) <= 0 )
		{
			printf( "Send failed with %s\n", strerror( errno ) );
		}
	}
}
