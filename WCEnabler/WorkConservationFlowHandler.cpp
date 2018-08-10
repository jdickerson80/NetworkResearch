#include "WorkConservationFlowHandler.h"

#include <netinet/tcp.h>
#include <sstream>
#include <stdlib.h>

#include "LoggingHandler.h"

namespace WCEnabler {
WorkConservationFlowHandler::WorkConservationFlowHandler( const std::string& interface
														  , float beta
														  , float safetyFactor
														  , Common::LoggingHandler* logger )
	: _beta( beta )
	, _safetyFactor( safetyFactor )
	, _logger( logger )
{
	/// init the stream
	std::ostringstream stream1;
	std::ostringstream stream2;

	/// stream the multipath off command
	stream1 << "ip link set dev " << interface << " multipath off > /dev/null";
	_multipathBackupCommand = stream1.str();

	/// stream the multipath on command
	stream2 << "ip link set dev " << interface << " multipath on > /dev/null";
	_multipathNonBackupCommand = stream2.str();

	/// set the state to GuaranteedBandwidthSufficient
	setState( FlowState::GuaranteedBandwidthSufficient );
}

WorkConservationFlowHandler::~WorkConservationFlowHandler()
{
	delete _logger;
}

WorkConservationFlowHandler::FlowState::Enum WorkConservationFlowHandler::updateWorkConservation( float currentBandwidthGuaranteeRate
																								  , float workConservingRate
																								  , uint8_t ECNValue
																								  , float currentBandwidthGuarantee)
{
	switch ( _currentState )
	{
	/// state where there is a flow but no WC flow
	case FlowState::ExistingFlowWithoutWorkConservation:
		/// if the previous time slot has an ECN,
		if ( _values.ecn )
		{
			/// start the WC flow
			setState( FlowState::ExistingFlowWithWorkConservation );
		}
		else /// there is no ECN
		{
			/// set the state of GuaranteedBandwidthSufficient
			setState( FlowState::GuaranteedBandwidthSufficient );
		}
		break;

	/// state where there is an existing flow with a WC flow
	case FlowState::ExistingFlowWithWorkConservation:
	{
		/// init the local variables
		float averageBandwidthGuaranteeRate	= _values.bandwidthGuaranteeAverage.rate();
		float averageWorkConservingRate		= _values.workConservingAverage.rate();

		/// average bandwidth rate < average work conserving rate * a safety factor
		if ( averageBandwidthGuaranteeRate < ( averageWorkConservingRate * _beta ) )
		{
			setState( FlowState::GuaranteedBandwidthSufficient );
		}
	}
		break;

	/// state where the bandwidth guarantee flow is sufficient
	case FlowState::GuaranteedBandwidthSufficient:
		/// do the vm level check
		if ( !vmLevelCheck( currentBandwidthGuarantee ) )
		{
			// set the state to new wc flow
			setState( FlowState::NewWCFlow );
		}
		/// bandwidth guarantee flow is still sufficient
		break;

	/// new flow state
	case FlowState::NewWCFlow:
		// do nothing state
		break;
	}

//	printf( "lwc %f wc %f sf %f\n"
//			, _values.workConservingRate
//			, workConservingRate
//			, _safetyFactor );

	/// update the values. basically, set the last values to the current ones
	_values.update( currentBandwidthGuaranteeRate, ECNValue, workConservingRate );

	/// return the current state
	return _currentState;
}

WorkConservationFlowHandler::FlowState::Enum WorkConservationFlowHandler::currentState() const
{
	return _currentState;
}

void WorkConservationFlowHandler::setWCSubFlowEnabled( bool isEnabled )
{
	if ( isEnabled )
	{
		system( _multipathNonBackupCommand.c_str() );
	}
	else
	{
		system( _multipathBackupCommand.c_str() );
	}
}

std::string WorkConservationFlowHandler::stateToString( FlowState::Enum currentState )
{
	switch ( currentState )
	{
	case FlowState::ExistingFlowWithoutWorkConservation:
		return std::string( "ExistingFlowWithoutWorkConservation" );
		break;

	case FlowState::ExistingFlowWithWorkConservation:
		return std::string( "ExistingFlowWithWorkConservation" );
		break;

	case FlowState::GuaranteedBandwidthSufficient:
		return std::string( "GuaranteedBandwidthSufficient" );
		break;

	case FlowState::NewWCFlow:
		return std::string( "NewWCFlow" );
		break;
	}

	return std::string();
}

void WorkConservationFlowHandler::setState( FlowState::Enum flowState )
{
	/// if the current state is the correct one
	if ( flowState == _currentState )
	{
		/// leave the method
		return;
	}

	/// set the correct state
	_currentState = flowState;

	/// do the transition into each state
	switch ( _currentState )
	{
	case FlowState::ExistingFlowWithoutWorkConservation:
		setWCSubFlowEnabled( false );
		break;

	case FlowState::ExistingFlowWithWorkConservation:
		setWCSubFlowEnabled( true );
		break;

	case FlowState::GuaranteedBandwidthSufficient:
		setWCSubFlowEnabled( false );
		break;

	case FlowState::NewWCFlow:
		_currentState = FlowState::ExistingFlowWithoutWorkConservation;
		break;
	}

	printf("%s\n", stateToString( _currentState ).c_str() );
}

bool WorkConservationFlowHandler::vmLevelCheck( float bandwidthGuarantee )
{
	float bandwidthGuaranteeAverage = _values.bandwidthGuaranteeAverage.rate();
	float workConservationGuaranteeRate = _values.workConservingAverage.rate();

	/// bandwidth guarantee average + work conservation average rate
	/// < current bandwidth guarantee * safety factor
	return ( bandwidthGuaranteeAverage + workConservationGuaranteeRate )
			< ( bandwidthGuarantee * _safetyFactor );
//	printf( "%2.2f, %2.2f, %2.2f, %2.2f, %i\n"
//			, bandwidthGuaranteeAverage
//			, workConservationGuaranteeRate
//			, bandwidthGuarantee
//			, _safetyFactor
//			, ret );
//	return ret;
}

} /// namespace WCEnabler
