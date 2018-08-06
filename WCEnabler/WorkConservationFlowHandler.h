#ifndef WORKCONSERVATIONFLOWHANDLER_H
#define WORKCONSERVATIONFLOWHANDLER_H

#include <string>

// Forward declarations
namespace Common {
class LoggingHandler;
}
class BandwidthCalculator;

/**
 * @brief The WorkConservationFlowHandler class handles the logic of the work conservation flow.
 * @todo finish this class
 */
class WorkConservationFlowHandler
{
public:

	struct FlowState
	{
		enum Enum
		{
			NewFlow,
			ExistingFlowWithoutWorkConservation,
			ExistingFlowWithWorkConservation
		};
	};

private:

	float _currentBandwidth;
	float _ecn;
	float _safetyFactor;
	FlowState::Enum _currentState;
	std::string _multipathBackupCommand;
	std::string _multipathNonBackupCommand;
	Common::LoggingHandler* _logger;
	BandwidthCalculator* _bandwidthCalculator;

public:

	/**
	 * @brief WorkConservationFlowHandler
	 * @param interface
	 * @param safetyFactor
	 * @param bandwidthCalculator
	 * @param logger
	 */
	WorkConservationFlowHandler( const std::string& interface
								 , float safetyFactor
								 , BandwidthCalculator* bandwidthCalculator
								 , Common::LoggingHandler* logger );

	~WorkConservationFlowHandler();

	/**
	 * @brief updateWorkConservation
	 * @param currentBandwidth
	 * @param ECNValue
	 * @return
	 */
	FlowState::Enum updateWorkConservation( float currentBandwidth, float ECNValue );

	/**
	 * @brief currentState
	 * @return
	 */
	FlowState::Enum currentState() const;

	/**
	 * @brief setWCSubFlowEnabled
	 * @param isEnabled
	 */
	void setWCSubFlowEnabled( bool isEnabled );

private:

	/**
	 * @brief setState
	 * @param flowState
	 */
	void setState( FlowState::Enum flowState );
};

#endif // WORKCONSERVATIONFLOWHANDLER_H
