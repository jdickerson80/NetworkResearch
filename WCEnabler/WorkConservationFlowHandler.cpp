#include "WorkConservationFlowHandler.h"

#include <netinet/tcp.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "BandwidthValues.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "ThreadHelper.h"
#include "WCPrintHandler.h"

#define ModifyIPLink ( 1 )
#if !defined ( ModifyIPLink )
	#define WCOn ( 1 )
#endif

namespace WCEnabler {
WorkConservationFlowHandler::WorkConservationFlowHandler(const std::string& interface
		, float beta
		, float safetyFactor
		, Common::LoggingHandler* logger
		, BandwidthValues * const bandwidthValues )
	: _beta( beta )
	, _safetyFactor( safetyFactor )
	, _updateThreadRunning( false )
	, _bandwidthValues( bandwidthValues )
	, _bandwidthGuaranteeAverage( 1.0 )
	, _workConservingAverage( 1.0 )
	, _logger( logger )
{
	// init the stream
	std::ostringstream stream1;
	std::ostringstream stream2;
	_interface = const_cast<char*>(interface.c_str());
	// stream the multipath off command
	stream1 << "ip link set dev " << interface << " multipath off &> /dev/null";
	_multipathBackupCommand = stream1.str();

	// stream the multipath on command
	stream2 << "ip link set dev " << interface << " multipath on &> /dev/null";
	_multipathNonBackupCommand = stream2.str();

#if !defined ( ModifyIPLink )
	#if defined ( WCOn )
		system( _multipathNonBackupCommand.c_str() );
	#else
		system( _multipathBackupCommand.c_str() );
	#endif
#endif
	// set the state to GuaranteedBandwidthSufficient
	setState( FlowState::GuaranteedBandwidthSufficient );

	// start both threads
	Common::ThreadHelper::startDetachedThread( &_updateThread
											   , updateWorkConservation
											   , _updateThreadRunning
											   , static_cast< void* >( this ) );
}

WorkConservationFlowHandler::~WorkConservationFlowHandler()
{
	_updateThreadRunning = false;
	delete _logger;
}

WorkConservationFlowHandler::FlowState::Enum WorkConservationFlowHandler::currentState() const
{
	return _currentState;
}

void WorkConservationFlowHandler::setWCSubFlowEnabled( bool isEnabled )
{
#if defined ( ModifyIPLink )
	if ( isEnabled )
	{
		PRINT("turning on mptcp\n");
		system( _multipathNonBackupCommand.c_str() );
	}
	else
	{
		PRINT("turning on mptcp\n");
		system( _multipathBackupCommand.c_str() );
	}
#endif
}

char* WorkConservationFlowHandler::stateToString( FlowState::Enum currentState )
{
	switch ( currentState )
	{
	case FlowState::ExistingFlowWithoutWorkConservation:
		return "ExistingFlowWithoutWorkConservation\n";

	case FlowState::ExistingFlowWithWorkConservation:
		return "ExistingFlowWithWorkConservation\n";

	case FlowState::GuaranteedBandwidthSufficient:
		return "GuaranteedBandwidthSufficient\n";

	case FlowState::NewWCFlow:
		return "NewWCFlow\n";
	}

	return nullptr;
}

void WorkConservationFlowHandler::setState( FlowState::Enum flowState )
{
	// if the current state is the correct one
	if ( flowState == _currentState )
	{
		// leave the method
		return;
	}

	// set the correct state
	_currentState = flowState;

	// do the transition into each state
	switch ( _currentState )
	{
	case FlowState::ExistingFlowWithoutWorkConservation:
		setWCSubFlowEnabled( false );
		break;

	case FlowState::ExistingFlowWithWorkConservation:
		setWCSubFlowEnabled( true );
		break;

	case FlowState::GuaranteedBandwidthSufficient:
		_bandwidthValues->ecnValue = false;
		setWCSubFlowEnabled( false );
		break;

	case FlowState::NewWCFlow:
		_currentState = FlowState::ExistingFlowWithoutWorkConservation;
		break;
	}

	memset( _logBuffer, 0, LogBuffer );
	// create the stream
	snprintf( _logBuffer, LogBuffer, "%lu, %s"
			  , clock()
			  , stateToString( _currentState ) );

	_logger->log( _logBuffer );
}

bool WorkConservationFlowHandler::vmLevelCheck( float bandwidthGuarantee )
{
	float bandwidthGuaranteeAverage = _bandwidthGuaranteeAverage.rate();
	float workConservationGuaranteeRate = _workConservingAverage.rate();

	// bandwidth guarantee average + work conservation average rate
	// < current bandwidth guarantee * safety factor
	return ( bandwidthGuaranteeAverage + workConservationGuaranteeRate )
			< ( bandwidthGuarantee * _safetyFactor );
}

void* WorkConservationFlowHandler::updateWorkConservation( void* input )
{
	// init the variables
	WorkConservationFlowHandler* workConservationFlowHandler = static_cast< WorkConservationFlowHandler* >( input );
	const std::atomic_uint& bandwidthGuarantee = workConservationFlowHandler->_bandwidthValues->bandwidthGuarantee;
	const std::atomic_uint& bandwidthGuaranteeRate = workConservationFlowHandler->_bandwidthValues->bandwidthGuaranteeRate;
	std::atomic_uint& ecnValue = workConservationFlowHandler->_bandwidthValues->ecnValue;
	const std::atomic_uint& workConservingRate = workConservationFlowHandler->_bandwidthValues->workConservingRate;
	const std::atomic_bool& threadRunning = workConservationFlowHandler->_updateThreadRunning;
	FlowState::Enum* currentState = &workConservationFlowHandler->_currentState;
	RateCalculator* bandwidthGuaranteeAverage = &workConservationFlowHandler->_bandwidthGuaranteeAverage;
	RateCalculator* workConservingAverage = &workConservationFlowHandler->_workConservingAverage;

	while ( threadRunning.load() )
	{
		switch ( *currentState )
		{
		// state where there is a flow but no WC flow
		case FlowState::ExistingFlowWithoutWorkConservation:
			// if the previous time slot has an ECN,
			if ( ecnValue.load() )
			{
				// set the state of GuaranteedBandwidthSufficient
				workConservationFlowHandler->setState( FlowState::GuaranteedBandwidthSufficient );
			}
			else // there is no ECN
			{
				// start the WC flow
				workConservationFlowHandler->setState( FlowState::ExistingFlowWithWorkConservation );
			}
			break;

		// state where there is an existing flow with a WC flow
		case FlowState::ExistingFlowWithWorkConservation:
		{
			// init the local variables
			float averageBandwidthGuaranteeRate	= bandwidthGuaranteeAverage->rate();
			float averageWorkConservingRate		= workConservingAverage->rate();
			float beta							= workConservationFlowHandler->_beta;

			// average bandwidth rate < average work conserving rate * a safety factor
			if ( ( averageBandwidthGuaranteeRate * beta ) > averageWorkConservingRate )
			{
				workConservationFlowHandler->setState( FlowState::GuaranteedBandwidthSufficient );
			}
		}
			break;

		// state where the bandwidth guarantee flow is sufficient
		case FlowState::GuaranteedBandwidthSufficient:
			// do the vm level check
			if ( !workConservationFlowHandler->vmLevelCheck( bandwidthGuarantee.load() ) )
			{
				// set the state to new wc flow
				workConservationFlowHandler->setState( FlowState::NewWCFlow );
			}
			// bandwidth guarantee flow is still sufficient
			break;

		// new flow state
		case FlowState::NewWCFlow:
			// do nothing state
			break;
		}

		bandwidthGuaranteeAverage->calculateRate( (float)bandwidthGuaranteeRate.load() );
		workConservingAverage->calculateRate( (float)workConservingRate.load() );
//		ecnValue = false;
		usleep( 250000 );
	}

	pthread_exit( nullptr );
}

} // namespace WCEnabler
