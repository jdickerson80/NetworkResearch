#include "HelperMethods.h"

#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>

namespace Common {

HelperMethods::InterfaceInfo HelperMethods::getInterfaceName()
{
	// init local variables
	ifaddrs* ifaddr;
	ifaddrs* ifa;
	int family, s, n;
	char host[ NI_MAXHOST ];
	InterfaceInfo interfaceInfo;

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

		// check if the family is internet protocol v4
		if ( family == AF_INET )
		{
			// get the name info
			s = getnameinfo( ifa->ifa_addr
							 , sizeof( sockaddr_in )
							 , host
							 , NI_MAXHOST
							 , NULL
							 , 0
							 , NI_NUMERICHOST );
			// check the return value
			if ( s != 0 )
			{
				printf( "getnameinfo() failed: %s\n", gai_strerror( s ) );
			}

			// save the local ip address and interface name
			interfaceInfo.ipAddress = host;
			interfaceInfo.interfaceName = ifa->ifa_name;
		}
	}

	// free the memory
	freeifaddrs( ifaddr );

	return interfaceInfo;
}

} // namespace Common
