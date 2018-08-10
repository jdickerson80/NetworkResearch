#ifndef HANDLEHOSTBANDWIDTH_H
#define HANDLEHOSTBANDWIDTH_H

#include <arpa/inet.h>
#include <pthread.h>

#include "LoggingHandler.h"

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
		unsigned int demand;
		unsigned int guarantee;
		unsigned int lastDemand;
		unsigned int lastGuarantee;
		sockaddr_in address;

		HostBandwidthStatistics()
			: demand( 0 )
			, guarantee( 0 )
			, lastDemand( 0 )
			, lastGuarantee( 0 )
		{}
	};

private:

	bool _isRunning;
	Common::LoggingHandler _logger;
	unsigned int _totalBandwidth;
	HostBandwidthStatistics* _hostsBandwidth;
	int _socketFileDescriptor;
	sockaddr_in _localAddresses;
	unsigned int _numberOfHosts;
	pthread_t _receiveThread;

public:

	/**
	 * @brief HandleHostBandwidth constructor
	 * @param numberOfHosts number of hosts in the network
	 * @param totalBandwidth total bandwidth of the tenant
	 */
	HandleHostBandwidth( unsigned int numberOfHosts, unsigned int totalBandwidth );

	~HandleHostBandwidth();

	/**
	 * @brief setRunning turn on the thread that receives the VMs bandwidth
	 * @param isRunning true the thread runs, false it shuts it off
	 */
	void setRunning( bool isRunning );

	/**
	 * @brief printBandwidths prints VM's bandwidth to the terminal
	 */
	void logBandwidths();

	/**
	 * @brief sendBandwidthRates sends all of the VMs bandwidth
	 */
	void sendBandwidthRates();

private:

	/**
	 * @brief handleConnections method that handles connections
	 * @param input pointer to an instance of this class
	 * @return NULL
	 */
	static void* handleConnections( void* input );

	/**
	 * @brief findHostIndex finds the array index of the ip address
	 * @param ipAddress
	 * @return index
	 */
	static size_t findHostIndex( char* ipAddress );

	/**
	 * @brief calculateHostBandwidth methods calculates each VMs bandwidth limit
	 * @param hostStatistics pointer to the bandwidth statistics
	 */
	void calculateHostBandwidth( HostBandwidthStatistics* hostStatistics );
};

#endif // HANDLEHOSTBANDWIDTH_H
