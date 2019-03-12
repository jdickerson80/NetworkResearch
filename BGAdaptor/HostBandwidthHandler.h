#ifndef HOSTBANDWIDTHHANDLER_H
#define HOSTBANDWIDTHHANDLER_H

#include <arpa/inet.h>
#include <atomic>
#include <map>
#include <pthread.h>

#include "BandwidthGuaranteeHost.h"

// Forward declarations
namespace Common {
class LoggingHandler;
}

namespace BGAdaptor {
/**
 * @brief	The HandleHostBandwidth class handles all of the hosts bandwidth. This class collects and stores
 *			each hosts bandwidth. Then, calculates each VM's bandwidth limit and sends it to the VM.
 */
class HostBandwidthHandler
{
public:

	typedef std::map< in_addr_t, BandwidthGuaranteeHost > BandwidthMap;

private:

	BandwidthMap _bandwidthMap;
	uint _dynamicAllocation;
	std::atomic_bool _incomingBandwidthRunning;
	unsigned int _numberOfHosts;
	std::atomic_bool _outgoingBandwidthRunning;
	unsigned int _totalBandwidth;
	int _socketFileDescriptor;
	sockaddr_in _localAddresses;
	pthread_t _receiveThread;
	pthread_t _sendThread;

	Common::LoggingHandler* _incomingBandwidthlogger;
	Common::LoggingHandler* _outgoingBandwidthlogger;

public:

	/**
	 * @brief HandleHostBandwidth constructor
	 * @param totalBandwidth total bandwidth of the tenant
	 */
	HostBandwidthHandler( uint totalBandwidth, uint dynamicAllocation, uint numberOfHost );

	~HostBandwidthHandler();

private:

	/**
	 * @brief handleHostsIncomingBandwidth method that handles connections
	 * @param input pointer to an instance of this class
	 * @return NULL
	 */
	static void* handleHostsIncomingBandwidth( void* input );

	/**
	 * @brief handleHostsOutgoingBandwidth
	 * @param input
	 * @return
	 */
	static void* handleHostsOutgoingBandwidth( void* input );

	/**
	 * @brief calculateHostBandwidth methods calculates each VMs bandwidth limit
	 * @param hostStatistics pointer to the bandwidth statistics
	 */
	void calculateHostBandwidth( BandwidthGuaranteeHost* hostStatistics );
};

} // namespace BGAdaptor

#endif // HOSTBANDWIDTHHANDLER_H
