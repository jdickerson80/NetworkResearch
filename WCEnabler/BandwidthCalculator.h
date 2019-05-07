#ifndef BANDWIDTHCALCULATOR_H
#define BANDWIDTHCALCULATOR_H

#include <atomic>
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

	int _socketFileDescriptor;
	std::atomic_bool _packetSniffingThreadRunning;
	std::atomic_bool _calculationThreadRunning;
	std::atomic_uint _bandwidthGuaranteeCounter;
	std::atomic_uint _workConservingCounter;
	BandwidthValues* _bandwidthValues;
	RateCalculator _bandwidthGuaranteeRateCalculator;
	RateCalculator _workConservingRateCalculator;
	pthread_t _packetSniffingThread;
	pthread_t _calculationThread;
	Common::LoggingHandler* _logger;
	const std::string _interfaceIPAddress;
	const std::string _interfaceName;

public:

	/**
	 * @brief BandwidthCalculator constructor
	 * @param logger pointer to the logging handler
	 */
	BandwidthCalculator(
			Common::LoggingHandler* logger
			, const std::string& interfaceIPAddress
			, BandwidthValues* bandwidthValues
			, const std::string& interfaceName );

	~BandwidthCalculator();

private:

	/**
	 * @brief parseTCFile parses the file that TC outputs to find the rate of each flow
	 * @param buffer that holds the contents of the TC file
	 * @param bufferSize number of bytes in the file
	 * @note the tc output looks like the below lines. The values that are imports are
	 *		 surrounded by --> and <--
	 * class htb 1:11 parent 1:1 prio 0 rate 166Mbit ceil 166Mbit burst 1577b cburst 1577b
	 *  Sent -->105003970<-- bytes 69707 pkt (dropped 0, overlimits 0 requeues 0)
	 *  backlog 0b 0p requeues 0
	 *  lended: 1921 borrowed: 0 giants: 0
	 *  tokens: 1135 ctokens: 1135
	 * class htb 1:1 root rate 166Mbit ceil 166Mbit burst 1577b cburst 1577b
	 *  Sent 105003970 bytes 69707 pkt (dropped 0, overlimits 0 requeues 0)
	 *  backlog 0b 0p requeues 0
	 *  lended: 0 borrowed: 0 giants: 0
	 *  tokens: 1135 ctokens: 1135
	 * class htb 1:12 parent 1:1 prio 1 rate 96bit ceil 166Mbit burst 1599b cburst 1577b
	 *  Sent -->0<-- bytes 0 pkt (dropped 0, overlimits 0 requeues 0)
	 *  backlog 0b 0p requeues 0
	 *  lended: 0 borrowed: 0 giants: 0
	 *  tokens: 2083333328 ctokens: 1203
	 */
	void parseTCFile( char* buffer );

	/**
	 * @brief	handlePacketSniffing is the method that the packet sniffing thread runs.
	 *			This method just sniffs each packets and increments a variable that represents
	 *			each flow's count.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* handlePacketSniffing( void* input );

	static unsigned int findValueAfterCharacter( char* buffer, const char deliminatingCharacter, size_t startLoopPosition );

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
