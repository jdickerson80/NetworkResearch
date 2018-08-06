#include "WCEnablerObject.h"

#include <algorithm>
#include <ifaddrs.h>
#include <netdb.h>
#include <sstream>
#include <string.h>

#include "BandwidthCalculator.h"
#include "BandwidthCommunicator.h"
#include "LoggingHandler.h"
#include "WorkConservationFlowHandler.h"

WCEnablerObject::WCEnablerObject( const std::string& bgAdaptorIPAddress )
{
	// init local variables
	ifaddrs* ifaddr;
	ifaddrs* ifa;
	int family, s, n;
	char host[NI_MAXHOST];
	std::string interface;

	// check for error when trying to get if address
	if ( getifaddrs( &ifaddr ) == -1 )
	{
		perror("getifaddrs");
	}

	// loop through the list of interfaces
	for ( ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++ )
	{
		// if the name is empty or if it is the loopback address, start
		// the loop over again
		if ( ifa->ifa_addr == NULL || !strcmp( ifa->ifa_name, "lo" ) )
			continue;

		// grab the family
		family = ifa->ifa_addr->sa_family;

		// check if the family is internet protocol
		if ( family == AF_INET || family == AF_INET6 )
		{
			// get the name info
			s = getnameinfo( ifa->ifa_addr
							, (family == AF_INET) ? sizeof( sockaddr_in ) : sizeof( sockaddr_in6 )
							, host
							, NI_MAXHOST
							, NULL
							, 0
							, NI_NUMERICHOST );
			// check the return value
			if ( s != 0 )
			{
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
			}

			// save the local ip address and interface name
			_ipAddress = host;
			interface = ifa->ifa_name;
		}
	}

	// free the memory
	freeifaddrs( ifaddr );

	// create the instances
	_bandwidthCalculator = new BandwidthCalculator( buildLogger( interface, "BandwidthCalculator" ) );
	_bandwidthCommunicator = new BandwidthCommunicator( bgAdaptorIPAddress
														, interface
														, _bandwidthCalculator
														, buildLogger( interface, "BandwidthCommunicator" ) );
	_workConservationFlowHandler = new WorkConservationFlowHandler( interface
																	, 1.0
																	, _bandwidthCalculator
																	, buildLogger( interface, "WorkConservation" ) );
	_workConservationFlowHandler->setWCSubFlowEnabled( false );
}

WCEnablerObject::~WCEnablerObject()
{
	delete _bandwidthCalculator;
	delete _bandwidthCommunicator;
	delete _workConservationFlowHandler;
}

Common::LoggingHandler* WCEnablerObject::buildLogger( const std::string& interface, const std::string& filename )
{
	// remove the -eth0 from the interface name
	char removeInterface[] = "-eth0";
	std::string newInterface( interface );

	// loop through the string, removing appropriate characters
	for ( unsigned int i = 0; i < strlen( removeInterface ); ++i )
	{
		newInterface.erase( remove( ++newInterface.begin(), newInterface.end(), removeInterface[ i ] ), newInterface.end()  );
	}

	// stream the logging string, create and return the pointer to it
	std::ostringstream stream;
	stream << "/tmp/" << newInterface << "/" << filename << ".log";
	return new Common::LoggingHandler( stream.str() );
}
