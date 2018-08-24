#include "MainObject.h"

#include <sstream>

#include "BandwidthCalculator.h"
#include "BandwidthCommunicator.h"
#include "BandwidthValues.h"
#include "HelperMethods.h"
#include "LoggerFactory.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "WorkConservationFlowHandler.h"

namespace WCEnabler {

MainObject::MainObject()
{
	setECNEnabled( true );

	// get the interface's name
	Common::HelperMethods::InterfaceInfo interface = Common::HelperMethods::getInterfaceName();

	_bandwidthValues = new BandwidthValues();

	// create the threads
	_bandwidthCommunicator = new BandwidthCommunicator(
				BGAdaptorIPAddress
				, interface.interfaceName
				, _bandwidthValues
				, Common::LoggerFactory::buildLogger( interface.interfaceName, "BandwidthLimit", false )
				, Common::LoggerFactory::buildLogger( interface.interfaceName, "BandwidthUsage", false ) );

	_workConservationFlowHandler = new WorkConservationFlowHandler(
				interface.interfaceName
				, 0.05
				, 0.05
				, Common::LoggerFactory::buildLogger( interface.interfaceName, "WorkConservation", false )
				, _bandwidthValues  );

	_bandwidthCalculator = new BandwidthCalculator(
				Common::LoggerFactory::buildLogger( interface.interfaceName, "BandwidthCalculator", true )
				, interface.ipAddress
				, _bandwidthValues );
}

MainObject::~MainObject()
{
	// delete all of the heap memory
    delete _bandwidthCalculator;
    delete _bandwidthCommunicator;
	delete _workConservationFlowHandler;
	delete _bandwidthValues;
}

const BandwidthValues* const MainObject::bandwidthValues() const
{
	return _bandwidthValues;
}

MainObject& MainObject::instance()
{
	static MainObject instance;
	return instance;
}

void MainObject::setECNEnabled( bool isEnabled )
{
	if ( isEnabled )
	{
		system( "sysctl -w net.ipv4.tcp_ecn=1 > /dev/null" );
	}
	else
	{
		system( "sysctl -w net.ipv4.tcp_ecn=0 > /dev/null" );
	}
}

} // namespace WCEnabler
