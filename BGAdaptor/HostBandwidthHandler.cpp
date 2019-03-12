#include "HostBandwidthHandler.h"

#include <sstream>
#include <string.h>
#include <unistd.h>


#include <vector>

#include "BGPrintHandler.h"
#include "HelperMethods.h"
#include "LoggingHandler.h"
#include "LoggerFactory.h"
#include "ThreadHelper.h"

//#define LogPackets ( 1 )
namespace BGAdaptor {

HostBandwidthHandler::HostBandwidthHandler( uint totalBandwidth, uint dynamicAllocation, uint numberOfHost )
	: _dynamicAllocation( dynamicAllocation )
	, _incomingBandwidthRunning( false )
	, _numberOfHosts( numberOfHost )
	, _outgoingBandwidthRunning( false )
	, _totalBandwidth( totalBandwidth )
{
	_socketFileDescriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( _socketFileDescriptor == -1 )
	{
		PRINT( "socket failed\n" );
	}

	// get the interface's name
	Common::HelperMethods::InterfaceInfo interface = Common::HelperMethods::getInterfaceName();

	_incomingBandwidthlogger = Common::LoggerFactory::buildLogger( interface.interfaceName, "incoming", false );
	_outgoingBandwidthlogger = Common::LoggerFactory::buildLogger( interface.interfaceName, "outgoing", false );

	std::ostringstream stream;
	stream << "tot: " << totalBandwidth << " " << "dyn: " << dynamicAllocation << " " << "hosts: " << numberOfHost << "\n";

	// log the bandwidth limit
	_incomingBandwidthlogger->log( stream.str() );

	_localAddresses.sin_addr.s_addr = htonl( INADDR_ANY );
	_localAddresses.sin_family = AF_INET;
	_localAddresses.sin_port = htons( 8888 );

	if ( bind( _socketFileDescriptor, (sockaddr*)&_localAddresses, sizeof( _localAddresses ) ) == -1 )
	{
		PRINT( "bind failed\n" );
	}

	Common::ThreadHelper::startDetachedThread( &_receiveThread, handleHostsIncomingBandwidth, _incomingBandwidthRunning, static_cast< void* >( this ) );
	Common::ThreadHelper::startDetachedThread( &_sendThread, handleHostsOutgoingBandwidth, _outgoingBandwidthRunning, static_cast< void* >( this ) );
}

HostBandwidthHandler::~HostBandwidthHandler()
{
	_incomingBandwidthRunning = false;
	_outgoingBandwidthRunning = false;

	close( _socketFileDescriptor );
	pthread_join( _receiveThread, nullptr );
	pthread_join( _sendThread, nullptr );
}

void* HostBandwidthHandler::handleHostsIncomingBandwidth( void* input )
{
	HostBandwidthHandler* bandwidthManager = static_cast< HostBandwidthHandler* >( input );
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
	Common::LoggingHandler* logger = bandwidthManager->_incomingBandwidthlogger;
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
			PRINT("continued\n");
			continue;
		}

		if ( receiveAddress.sin_addr.s_addr == loopBack.s_addr )
		{
			PRINT("got loop\n");
			continue;
		}

		in_addr_t inAddress = receiveAddress.sin_addr.s_addr;

		localStatistics = &bandwidthMap[ inAddress ];
		localStatistics->setAddress( receiveAddress ); // fix this
		localStatistics->setDemand( bandwidth );
//		PRINT("%s got %u\n", inet_ntoa( localStatistics->address().sin_addr ), localStatistics->guarantee().load() );

#if defined( LogPackets )
		// create the stream
		std::ostringstream stream;
		stream << inet_ntoa( receiveAddress.sin_addr ) << " " << bandwidth << "\n";

		// log the bandwidth limit
		logger->log( stream.str() );
#endif
	}

	pthread_exit( nullptr );
}

void* HostBandwidthHandler::handleHostsOutgoingBandwidth( void* input )
{
	typedef std::vector< BandwidthGuaranteeHost* > BandwidthGuaranteeVector;
	HostBandwidthHandler* bandwidthManager = static_cast< HostBandwidthHandler* >( input );
	BandwidthMap& bandwidthMap = bandwidthManager->_bandwidthMap;
	std::atomic_bool& threadRunning = bandwidthManager->_outgoingBandwidthRunning;
	BandwidthGuaranteeVector bandwidthDonors;
	BandwidthGuaranteeVector bandwidthReceivers;
//	uint totalBWG = bandwidthManager
//	size_t counter = 0;

#if defined( LogPackets )
	Common::LoggingHandler* logger = bandwidthManager->_outgoingBandwidthlogger;
	std::stringstream stream;
#endif

//	unsigned int rate = 1000;
	size_t rateSize = sizeof( unsigned int );
	size_t size = sizeof( sockaddr_in );
	int delta;
	int totalBandwidthContribution;
	int perHostDonation = 0;

	while ( bandwidthMap.size() != bandwidthManager->_numberOfHosts )
	{
		PRINT("WAITING %lu\n", bandwidthMap.size() );
		usleep( 500000 );
	}

	PRINT("GOT ALL HOSTS\n");

	for ( BandwidthMap::iterator it = bandwidthMap.begin(); it != bandwidthMap.end(); ++it )
	{
		bandwidthManager->calculateHostBandwidth( &it->second );
		if ( sendto( bandwidthManager->_socketFileDescriptor, &it->second.guarantee(), rateSize, 0, (sockaddr*)&it->second.address(), size ) <= 0 )
		{
			PRINT( "Send failed with %s\n", strerror( errno ) );
		}
	}

	if ( !bandwidthManager->_dynamicAllocation )
	{
		PRINT("Dynamic exit\n");
		pthread_exit( nullptr );
	}

	while ( threadRunning.load() )
	{
		totalBandwidthContribution = 0;
		for ( BandwidthMap::iterator it = bandwidthMap.begin(); it != bandwidthMap.end(); ++it )
		{
			delta = it->second.delta().load();

			if ( delta < 0 ) // receiver
			{
				bandwidthReceivers.push_back( &it->second );
//				PRINT("adding a receiver\n");
			}
			else if ( delta > 1000 )
			{
				totalBandwidthContribution += delta;
				bandwidthDonors.push_back( &it->second );
//				PRINT("adding a donor\n");
			}
		}

		if ( !bandwidthDonors.size() || !bandwidthReceivers.size() || totalBandwidthContribution <= 0 )
		{
			bandwidthDonors.clear();
			bandwidthReceivers.clear();
//			PRINT("CONT!!!!!\n");
			usleep( 500000 );
			continue;
		}

		// more donors than receivers
		if ( bandwidthDonors.size() >= bandwidthReceivers.size() )
		{
			perHostDonation = totalBandwidthContribution / static_cast< int >( bandwidthDonors.size() );
			PRINT("!!!!!!!!!!!!!!!!!!!!!!!!!!donor > receiver %i\n", perHostDonation );

			for ( BandwidthGuaranteeVector::iterator it = bandwidthDonors.begin(); it != bandwidthDonors.end(); ++it )
			{
				(*it)->setGuarantee( (*it)->guarantee() - (int)perHostDonation );
				PRINT("donors %s sent %u\n", inet_ntoa( (*it)->address().sin_addr ), (*it)->guarantee().load() );
			}
		}
		else // more receivers than donors
		{
			for ( BandwidthGuaranteeVector::iterator it = bandwidthDonors.begin(); it != bandwidthDonors.end(); ++it )
			{
				(*it)->setGuarantee( (*it)->demand().load() );
//				PRINT("donors %s sent %u\n", inet_ntoa( (*it)->address().sin_addr ), (*it)->guarantee().load() );
			}
		}

		PRINT("total %u per host %u\n", totalBandwidthContribution, perHostDonation );
		perHostDonation = totalBandwidthContribution / static_cast< int >( bandwidthReceivers.size() );

		for ( BandwidthGuaranteeVector::iterator it = bandwidthReceivers.begin(); it != bandwidthReceivers.end(); ++it )
		{
			(*it)->setGuarantee( (*it)->guarantee() + static_cast< int >( perHostDonation ) );
//			PRINT("rec %s sent %u\n", inet_ntoa( (*it)->address().sin_addr ), (*it)->guarantee().load() );
		}

		for ( BandwidthMap::iterator it = bandwidthMap.begin(); it != bandwidthMap.end(); ++it )
		{
			PRINT("%s sent %u\n", inet_ntoa( it->second.address().sin_addr ), it->second.guarantee().load() );
			if ( sendto( bandwidthManager->_socketFileDescriptor, &it->second.guarantee(), rateSize, 0, (sockaddr*)&it->second.address(), size ) <= 0 )
			{
				PRINT( "Send failed with %s\n", strerror( errno ) );
			}
		}

		bandwidthDonors.clear();
		bandwidthReceivers.clear();
		usleep( 500000 );
	}

	pthread_exit( nullptr );
}

void HostBandwidthHandler::calculateHostBandwidth( BandwidthGuaranteeHost* hostStatistics )
{
//	hostStatistics->lastGuarantee = hostStatistics->guarantee.load();
	hostStatistics->setGuarantee( _totalBandwidth / _bandwidthMap.size() );// / 2;
}

} // namespace BGAdaptor
