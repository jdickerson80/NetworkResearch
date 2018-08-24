#ifndef HANDLEHOSTBANDWIDTH_H
#define HANDLEHOSTBANDWIDTH_H

#include <arpa/inet.h>
#include <atomic>
#include <map>
#include <pthread.h>

// Forward declarations
namespace Common {
class LoggingHandler;
}

namespace BGAdaptor {
/**
 * @brief	The HandleHostBandwidth class handles all of the hosts bandwidth. This class collects and stores
 *			each hosts bandwidth. Then, calculates each VM's bandwidth limit and sends it to the VM.
 */
class HandleHostBandwidth
{
private:

	/**
	 * @brief The HostBandwidthStatistics struct holds statistics of each VM
	 */
	struct HostBandwidthStatistics
	{
		std::atomic_uint demand;
		std::atomic_uint guarantee;
		std::atomic_uint lastDemand;
		std::atomic_uint lastGuarantee;
		sockaddr_in address;

		HostBandwidthStatistics()
			: demand( 0 )
			, guarantee( 0 )
			, lastDemand( 0 )
			, lastGuarantee( 0 )
		{}
	};

private:

	typedef std::map< in_addr_t, HostBandwidthStatistics > BandwidthMap;

	std::atomic_bool _incomingBandwidthRunning;
	std::atomic_bool _outgoingBandwidthRunning;
	Common::LoggingHandler* _incomingBandwidthlogger;
	Common::LoggingHandler* _outgoingBandwidthlogger;
	unsigned int _totalBandwidth;
	BandwidthMap _bandwidthMap;
	int _socketFileDescriptor;
	sockaddr_in _localAddresses;
	pthread_t _receiveThread;
	pthread_t _sendThread;

public:

	/**
	 * @brief HandleHostBandwidth constructor
	 * @param totalBandwidth total bandwidth of the tenant
	 */
	HandleHostBandwidth( unsigned int totalBandwidth );

	~HandleHostBandwidth();

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
	void calculateHostBandwidth( HostBandwidthStatistics* hostStatistics );
};

} // namespace BGAdaptor
#endif // HANDLEHOSTBANDWIDTH_H
