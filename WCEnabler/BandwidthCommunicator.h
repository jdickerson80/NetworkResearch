#ifndef BANDWIDTHCOMMUNICATOR_H
#define BANDWIDTHCOMMUNICATOR_H

#include <arpa/inet.h>
#include <string>
#include <pthread.h>

class BandwidthCommunicator
{
private:

	bool _incomingBandwidthThreadRunning;
	int _socketFileDescriptor;
	sockaddr_in _bGAdaptorAddress;
	pthread_t _incomingBandwidthThread;
	std::string _interface;

public:

	BandwidthCommunicator( const std::string& BGAdaptorAddress, const std::string& interface );

	~BandwidthCommunicator();

	int sendBandwidth( float rate );

	float bandwidth();

	void setBandwidthThreadRunning( bool isRunning );

private:

	static void* handleIncomingBandwidthRequest( void* input );
};

#endif // BANDWIDTHCOMMUNICATOR_H
