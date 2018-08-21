#include "HandleHostBandwidth.h"

#include <sstream>
#include <string.h>
#include <unistd.h>

#include "ThreadHelper.h"

#define LogPackets ( 1 )

namespace BGAdaptor {

HandleHostBandwidth::HandleHostBandwidth( unsigned int totalBandwidth )
	: _incomingBandwidthRunning( false )
	, _outgoingBandwidthRunning( false )
	, _incomingBandwidthlogger( "/tmp/h1/incoming.log" )
	, _outgoingBandwidthlogger( "/tmp/h1/outgoing.log" )
	, _totalBandwidth( totalBandwidth )
{
	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		printf( "socket failed\n" );
	}

	_localAddresses.sin_addr.s_addr = htonl( INADDR_ANY );
	_localAddresses.sin_family = AF_INET;
	_localAddresses.sin_port = htons( 8888 );

	if ( bind( _socketFileDescriptor, (sockaddr*)&_localAddresses, sizeof( _localAddresses ) ) == -1 )
	{
		printf( "bind failed\n" );
	}

	Common::ThreadHelper::startDetachedThread( &_receiveThread, handleHostsIncomingBandwidth, _incomingBandwidthRunning, static_cast< void* >( this ) );
	Common::ThreadHelper::startDetachedThread( &_sendThread, handleHostsOutgoingBandwidth, _outgoingBandwidthRunning, static_cast< void* >( this ) );
}

HandleHostBandwidth::~HandleHostBandwidth()
{
	_incomingBandwidthRunning = false;
	_outgoingBandwidthRunning = false;

	close( _socketFileDescriptor );
}

void* HandleHostBandwidth::handleHostsIncomingBandwidth( void* input )
{
	HandleHostBandwidth* bandwidthManager = static_cast< HandleHostBandwidth* >( input );
	std::atomic_bool& threadRunning = bandwidthManager->_incomingBandwidthRunning;
	unsigned int bandwidth;
	unsigned int bandwidthLength = sizeof( bandwidth );
	unsigned int receiveLength;
	unsigned int length = sizeof( bandwidthManager->_localAddresses );
	sockaddr_in receiveAddress;
	BandwidthMap& bandwidthMap = bandwidthManager->_bandwidthMap;
	BandwidthMap::mapped_type* localStatistics;
	in_addr loopBack;
	inet_aton( "127.0.0.1", &loopBack );
#if defined( LogPackets )
	Common::LoggingHandler* logger = &bandwidthManager->_incomingBandwidthlogger;
#endif

	while ( threadRunning.load() )
	{
		receiveLength = recvfrom( bandwidthManager->_socketFileDescriptor
								  , &bandwidth
								  , bandwidthLength
								  , 0
								  , (sockaddr*)&receiveAddress
								  , &length );

		if ( receiveLength != bandwidthLength )
		{
			printf("continued\n");
			continue;
		}

		if ( receiveAddress.sin_addr.s_addr == loopBack.s_addr )
		{
			printf("got loop\n");
			continue;
		}

		in_addr_t inAddress = receiveAddress.sin_addr.s_addr;

		localStatistics = &bandwidthMap[ inAddress ];
		localStatistics->address = receiveAddress;
		localStatistics->lastDemand = localStatistics->demand.load();
		localStatistics->demand = bandwidth;

#if defined( LogPackets )
		// create the stream
		std::ostringstream stream;
		stream << inet_ntoa( receiveAddress.sin_addr ) << " " << bandwidth << "\n";

		// log the bandwidth limit
		logger->log( stream.str() );
#endif
	}

	pthread_exit( NULL );
	return NULL;
}

void* HandleHostBandwidth::handleHostsOutgoingBandwidth( void* input )
{
	HandleHostBandwidth* bandwidthManager = static_cast< HandleHostBandwidth* >( input );
	BandwidthMap& bandwidthMap = bandwidthManager->_bandwidthMap;
	std::atomic_bool& threadRunning = bandwidthManager->_outgoingBandwidthRunning;

#if defined( LogPackets )
	Common::LoggingHandler* logger = &bandwidthManager->_outgoingBandwidthlogger;
#endif

	unsigned int rate;
	size_t rateSize = sizeof( rate );
	size_t size = sizeof( sockaddr_in );
	std::stringstream stream;
	BandwidthMap::iterator end = bandwidthMap.end();
	BandwidthMap::iterator it = bandwidthMap.begin();

	while ( threadRunning.load() )
	{
		end = bandwidthMap.end();

		for ( it = bandwidthMap.begin(); it != end; ++it )
		{
			bandwidthManager->calculateHostBandwidth( &it->second );
			rate = it->second.guarantee;

			if ( sendto( bandwidthManager->_socketFileDescriptor, &rate, rateSize, 0, (sockaddr*)&it->second.address, size ) <= 0 )
			{
				printf( "Send failed with %s\n", strerror( errno ) );
			}

#if defined( LogPackets )
			// create the stream
			std::ostringstream stream;
			stream << inet_ntoa( it->second.address.sin_addr ) << " " << rate << "\n";

			// log the bandwidth limit
			logger->log( stream.str() );
#endif
		}

		usleep( 500000 );
	}

	pthread_exit( NULL );
	return NULL;
}

void HandleHostBandwidth::calculateHostBandwidth( HostBandwidthStatistics* hostStatistics )
{
	hostStatistics->lastGuarantee = hostStatistics->guarantee.load();
	hostStatistics->guarantee = _totalBandwidth / _bandwidthMap.size();// / 2;
}

} // namespace BGAdaptor
