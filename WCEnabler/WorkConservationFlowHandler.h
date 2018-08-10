/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef WORKCONSERVATIONFLOWHANDLER_H
#define WORKCONSERVATIONFLOWHANDLER_H

#include <stdint.h>
#include <string>

#include "ExponentialSmoothingCalculator.h"

/// Forward declarations
namespace Common {
class LoggingHandler;
}

namespace WCEnabler {
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

	/// typedef to make declarations easier
	typedef Common::Math::ExponentialSmoothingCalculator< float, float > RateCalculator;

	/**
	 * @brief The Values struct holds the pertinent values for flow control
	 */
	struct Values
	{
		float bandwidthGuaranteeRate;
		uint8_t ecn;
		float workConservingRate;

		RateCalculator bandwidthGuaranteeAverage;
		RateCalculator workConservingAverage;

		Values()
			: bandwidthGuaranteeRate( 0 )
			, ecn( 0 )
			, workConservingRate( 0 )
			, bandwidthGuaranteeAverage( 1.0 )
			, workConservingAverage( 1.0 ) {}

		/// convenience method to update the variables
		void update( float bandwidthGuaranteeRate, uint8_t ecn, float workConservingRate )
		{
			bandwidthGuaranteeRate = bandwidthGuaranteeRate;
			ecn = ecn;
			workConservingRate = workConservingRate;

			bandwidthGuaranteeAverage.calculateRate( bandwidthGuaranteeRate );
			workConservingAverage.calculateRate( workConservingRate );
		}
	};

private:

	float _beta;
	float _safetyFactor;
	Values _values;
	FlowState::Enum _currentState;
	std::string _multipathBackupCommand;
	std::string _multipathNonBackupCommand;
	Common::LoggingHandler* _logger;

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
								 , Common::LoggingHandler* logger );

	~WorkConservationFlowHandler();

	/**
	 * @brief updateWorkConservation method updates the work conservation flow
	 * @param currentBandwidthGuaranteeRate current bandwidth guarantee flow rate
	 * @param workConservingRate current work conserving flow rate
	 * @param ECNValue current ecn value
	 * @param currentBandwidthGuarantee current bandwidth guarantee
	 * @return current state
	 */
	FlowState::Enum updateWorkConservation( float currentBandwidthGuaranteeRate
											, float workConservingRate
											, uint8_t ECNValue
											, float currentBandwidthGuarantee );

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
	static std::string stateToString( FlowState::Enum currentState );

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
};

} /// namespace WCEnabler
#endif /// WORKCONSERVATIONFLOWHANDLER_H
