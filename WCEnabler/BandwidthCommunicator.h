#ifndef BANDWIDTHCOMMUNICATOR_H
#define BANDWIDTHCOMMUNICATOR_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string>

namespace Common {
class LoggingHandler;
}

namespace WCEnabler {

/// Forward declarations
class BandwidthCalculator;
class WorkConservationFlowHandler;

/**
 * @brief	The BandwidthCommunicator class handles the communication and setting
 *			of the hosts bandwidth.
 */
class BandwidthCommunicator
{
private:

	bool _incomingBandwidthThreadRunning;
	bool _outgoingBandwidthThreadRunning;
	int _socketFileDescriptor;
	Common::LoggingHandler* _bandwidthLimitLogger;
	Common::LoggingHandler* _bandwidthUsageLogger;
	sockaddr_in _bGAdaptorAddress;
	pthread_t _incomingBandwidthThread;
	pthread_t _outgoingBandwidthThread;
	std::string _interface;
	BandwidthCalculator* _bandwidthCalculator;
	WorkConservationFlowHandler* _workConservationFlowHandler;

public:

	/**
	 * @brief BandwidthCommunicator constructor
	 * @param bGAdaptorAddress address of the BGAdaptor
	 * @param interface of this host
	 * @param bandwidthCalculator pointer to the bandwidth calculator
	 * @param bandwidthLimitLogger pointer to the bandwidth limit logger
	 * @param bandwidthUsageLogger pointer to the bandwidth usage logger
	 * @param workConservationFlowHandler pointer to the WorkConservationFlowHandler
	 */
	BandwidthCommunicator( const std::string& bGAdaptorAddress
						   , const std::string& interface
						   , BandwidthCalculator* bandwidthCalculator
						   , Common::LoggingHandler* bandwidthLimitLogger
						   , Common::LoggingHandler* bandwidthUsageLogger
						   , WorkConservationFlowHandler* workConservationFlowHandler );

	~BandwidthCommunicator();

private:

	/**
	 * @brief	handleIncomingBandwidthRequest method is what the incoming bandwidth thread
	 *			runs. It just listens to the rate sent by the BGAdaptor, and sets the
	 *			rate limit by tc.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* handleIncomingBandwidthRequest( void* input );

	/**
	 * @brief	handleOutgoingBandwidthRequest method is what the outgoing bandwidth thread
	 *			runs. It just sends the current rate to the BGAdaptor.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* handleOutgoingBandwidthRequest( void* input );
};

} /// namespace WCEnabler
#endif /// BANDWIDTHCOMMUNICATOR_H
