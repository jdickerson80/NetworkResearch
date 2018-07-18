#ifndef HANDLEHOSTBANDWIDTH_H
#define HANDLEHOSTBANDWIDTH_H

#include <arpa/inet.h>
#include <pthread.h>

class HandleHostBandwidth
{
private:

	struct HostBandwidthStatistics
	{
		float demand;
		float guarantee;
		sockaddr_in address;

		HostBandwidthStatistics()
			: demand( 0 )
			, guarantee( 0 )
		{}
	};

private:

	bool _isRunning;
	HostBandwidthStatistics* _hostsBandwidth;
	int _socketFileDescriptor;
	sockaddr_in _localAddresses;
	unsigned int _numberOfHosts;
	pthread_t _receiveThread;

public:

	HandleHostBandwidth( unsigned int numberOfHosts );

	void setRunning( bool isRunning );

	~HandleHostBandwidth();

	void printBandwidths() const;

	void sendBandwidthRates() const;

private:

	static void* handleConnections( void* input );

	static size_t findHostIndex( char* ipAddress );

};

#endif // HANDLEHOSTBANDWIDTH_H
