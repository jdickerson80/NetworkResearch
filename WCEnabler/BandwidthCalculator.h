#ifndef BANDWIDTHCALCULATOR_H
#define BANDWIDTHCALCULATOR_H

#include <pthread.h>
#include <stdint.h>
#include <string>

#include "SimpleRateCalculator.h"

/// Forward declarations
namespace Common {
class LoggingHandler;
}
class iphdr;

namespace WCEnabler {

class BandwidthValues;

/**
 * @brief	The BandwidthCalculator class calculates the bandwidth of
 *			the bandwidth guarantee and work conservation flows by,
 *			simply, sniffing all of the packets on the interface.
 *			Each packet is logged, but only the outgoing packets
 *			are considered for the rate.
 */
class BandwidthCalculator
{
private:

	typedef Common::Math::SimpleRateCalculator< unsigned int, unsigned int > RateCalculator;

private:

	bool _packetSniffingThreadRunning;
	bool _calculationThreadRunning;
	int _socketFileDescriptor;
	unsigned int _bandwidthGuaranteeCounter;
	unsigned int _workConservingCounter;
	BandwidthValues* _bandwidthValues;
	RateCalculator _bandwidthGuaranteeRateCalculator;
	RateCalculator _workConservingRateCalculator;
	pthread_t _packetSniffingThread;
	pthread_t _calculationThread;
	Common::LoggingHandler* _logger;
	const std::string _interfaceIPAddress;

public:

	/**
	 * @brief BandwidthCalculator constructor
	 * @param logger pointer to the logging handler
	 */
	BandwidthCalculator( Common::LoggingHandler* logger, const std::string& interfaceIPAddress, BandwidthValues* bandwidthValues );
	~BandwidthCalculator();

private:

	/**
	 * @brief updateBandwidthGuaranteeRate updates the bandwidth guarantee's rate
	 */
	inline void updateBandwidthGuaranteeRate();

	/**
	 * @brief updateWorkConservingRate updates the work consversation's rate
	 */
	inline void updateWorkConservingRate();

	/**
	 * @brief updateTotalBandwidthRate updates the total bandwidth
	 */
	inline void updateTotalBandwidthRate();

	/**
	 * @brief	handlePacketSniffing is the method that the packet sniffing thread runs.
	 *			This method just sniffs each packets and increments a variable that represents
	 *			each flow's count.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* handlePacketSniffing( void* input );

	/**
	 * @brief	handleRateCalculation is the method that the rate calculation thread runs.
	 *			This method simply calls the above update* methods.
	 * @note	This thread exists so the public getter methods can just return the variable.
	 *			Otherwise, if the getter methods update the calculation, it may not be in
	 *			bytes/sec, or the variables may be different after a subsequent totalRate call.
	 *			This is because the packet sniffing thread could have updated the counters in
	 *			between two update calls. So, this thread ensures the rate is in bytes per
	 *			second, and helps the rate accuracy between subsequent calls
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* handleRateCalculation( void* input );
};

} // namespace WCEnabler
#endif // BANDWIDTHCALCULATOR_H