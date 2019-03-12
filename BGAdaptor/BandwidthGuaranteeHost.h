#ifndef BANDWIDTHGUARANTEEHOST_H
#define BANDWIDTHGUARANTEEHOST_H

#include <atomic>
#include <arpa/inet.h>

namespace BGAdaptor {

/**
 * @brief The HostBandwidthStatistics struct holds statistics of each VM
 */
class BandwidthGuaranteeHost
{
private:

	sockaddr_in _address;
	std::atomic_int _delta;
	std::atomic_uint _demand;
	std::atomic_uint _guarantee;
	std::atomic_uint _lastDemand;
	std::atomic_uint _lastGuarantee;

public:

	BandwidthGuaranteeHost();

	const sockaddr_in& address() const;
	void setAddress( const sockaddr_in address );

	const std::atomic_int& delta() const;
//	void setDelta( int delta );

	const std::atomic_uint& demand() const;
	void setDemand( uint demand );

	const std::atomic_uint& guarantee() const;
	void setGuarantee( uint guarantee );

private:

	inline void calculateDelta();
};

} // namespace BGAdaptor
#endif // BANDWIDTHGUARANTEEHOST_H
