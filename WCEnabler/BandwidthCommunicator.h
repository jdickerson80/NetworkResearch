#ifndef BANDWIDTHCOMMUNICATOR_H
#define BANDWIDTHCOMMUNICATOR_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string>

namespace Common {
class LoggingHandler;
}

class BandwidthCalculator;

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
	Common::LoggingHandler* _logger;
	sockaddr_in _bGAdaptorAddress;
	pthread_t _incomingBandwidthThread;
	pthread_t _outgoingBandwidthThread;
	std::string _interface;
	BandwidthCalculator* _bandwidthCalculator;

public:

	/**
	 * @brief BandwidthCommunicator constructor
	 * @param bGAdaptorAddress address of the BGAdaptor
	 * @param interface of this host
	 */
	BandwidthCommunicator( const std::string& bGAdaptorAddress
						   , const std::string& interface
						   , BandwidthCalculator* bandwidthCalculator
						   , Common::LoggingHandler* logger );

	~BandwidthCommunicator();

	/**
	 * @brief setBandwidthThreadRunning turn on the thread
	 * @param isRunning true the thread runs, false it shuts it off
	 */
	void setBandwidthThreadRunning( bool isRunning );

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

#endif // BANDWIDTHCOMMUNICATOR_H
