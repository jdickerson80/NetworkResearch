#include "WorkConservationFlowHandler.h"

#include <sstream>
#include <stdlib.h>

#include "LoggingHandler.h"

WorkConservationFlowHandler::WorkConservationFlowHandler( const std::string& interface
														  , float safetyFactor
														  , BandwidthCalculator* bandwidthCalculator
														  , Common::LoggingHandler* logger )
	: _currentBandwidth( 0 )
	, _ecn( 0 )
	, _safetyFactor( safetyFactor )
	, _currentState( FlowState::ExistingFlowWithoutWorkConservation )
	, _logger( logger )
	, _bandwidthCalculator( bandwidthCalculator )
{
	std::ostringstream stream1;
	std::ostringstream stream2;

	stream1 << "ip link set dev " << interface << " multipath off";
	_multipathBackupCommand = stream1.str();

	stream2 << "ip link set dev " << interface << " multipath on";

	_multipathNonBackupCommand = stream2.str();
}

WorkConservationFlowHandler::~WorkConservationFlowHandler()
{
	delete _logger;
}

WorkConservationFlowHandler::FlowState::Enum WorkConservationFlowHandler::updateWorkConservation( float currentBandwidth, float ECNValue )
{
	switch ( _currentState )
	{
	case FlowState::NewFlow:
		setState( FlowState::ExistingFlowWithoutWorkConservation );
		break;

	case FlowState::ExistingFlowWithoutWorkConservation:

		break;

	case FlowState::ExistingFlowWithWorkConservation:

		break;

	default:
		break;
	}

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
		printf("no back %i\n",
		system( _multipathNonBackupCommand.c_str() ) );
	}
	else
	{
		printf("back %i\n",
		system( _multipathBackupCommand.c_str() ) );
	}
}

void WorkConservationFlowHandler::setState( FlowState::Enum flowState )
{
	_currentState = flowState;

	switch ( _currentState )
	{
	case FlowState::NewFlow:

		break;

	case FlowState::ExistingFlowWithoutWorkConservation:
		setWCSubFlowEnabled( false );
		break;

	case FlowState::ExistingFlowWithWorkConservation:
		setWCSubFlowEnabled( true );
		break;
	}
}
