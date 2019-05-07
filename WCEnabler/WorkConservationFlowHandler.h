/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef WORKCONSERVATIONFLOWHANDLER_H
#define WORKCONSERVATIONFLOWHANDLER_H

#include <pthread.h>
#include <stdint.h>
#include <string>

#include <atomic>

#include "ExponentialSmoothingCalculator.h"

// Forward declarations
namespace Common {
class LoggingHandler;
}

namespace WCEnabler {

class BandwidthValues;

/**
 * @brief The WorkConservationFlowHandler class handles the logic of the work conservation flow.
 * @todo finish this class
 */
class WorkConservationFlowHandler
{
public:

	/**
	 * @brief	The FlowState struct holds all of the states of the
	 *			class.
	 */
	struct FlowState
	{
		enum Enum
		{
			ExistingFlowWithoutWorkConservation,
			ExistingFlowWithWorkConservation,
			GuaranteedBandwidthSufficient,
			NewWCFlow
		};
	};

private:

	// typedef to make declarations easier
	typedef Common::Math::ExponentialSmoothingCalculator< float, float > RateCalculator;

private:
#define LogBuffer ( 1024 )
	float _beta;
	float _safetyFactor;
	std::atomic_bool _updateThreadRunning;
	BandwidthValues* const _bandwidthValues;
	pthread_t _updateThread;
	FlowState::Enum _currentState;
	RateCalculator _bandwidthGuaranteeAverage;
	RateCalculator _workConservingAverage;
	std::string _multipathBackupCommand;
	std::string _multipathNonBackupCommand;
	Common::LoggingHandler* _logger;
	char _logBuffer[ LogBuffer ];

	char* _interface;
public:

	/**
	 * @brief WorkConservationFlowHandler constructor
	 * @param interface name
	 * @param beta safety factor for ExistingFlowWithWorkConservation
	 * @param safetyFactor for vmLevelCheck method
	 * @param logger for logging
	 */
	WorkConservationFlowHandler( const std::string& interface
								 , float beta
								 , float safetyFactor
								 , Common::LoggingHandler* logger
								 , BandwidthValues* const bandwidthValues );

	~WorkConservationFlowHandler();

	/**
	 * @brief currentState getter
	 * @return current state
	 */
	FlowState::Enum currentState() const;

	/**
	 * @brief stateToString method converts the state enum to a string
	 * @param currentState current state
	 * @return current state as a string
	 */
	static char* stateToString( FlowState::Enum currentState );

private:

	/**
	 * @brief setWCSubFlowEnabled setter enables/disables the work conserving flow
	 * @param isEnabled
	 */
	void setWCSubFlowEnabled( bool isEnabled );

	/**
	 * @brief	setState method sets the current state. It also does anything that a state
	 *			needs to be done to ENTER (only done once) the state. It is the transition
	 *			to a new state
	 * @param flowState new state
	 */
	void setState( FlowState::Enum flowState );

	/**
	 * @brief	vmLevelCheck method does the following formula:
	 *			bandwidth guarantee average + work conservation average rate
	 *			< current bandwidth guarantee * safety factor
	 * @param bandwidthGuarantee
	 * @return	true if the average rates is less than the bandwidth guarantee * a safety factor
	 *			false if the formula holds
	 */
	bool vmLevelCheck( float bandwidthGuarantee );

	/**
	 * @brief	method updates the work conservation flow
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* updateWorkConservation( void* input );
};

} // namespace WCEnabler
#endif // WORKCONSERVATIONFLOWHANDLER_H
