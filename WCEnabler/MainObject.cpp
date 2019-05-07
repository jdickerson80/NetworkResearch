#include "MainObject.h"

#include <sstream>
#include <string.h>

#include "BandwidthCalculator.h"
#include "BandwidthCommunicator.h"
#include "BandwidthValues.h"
#include "HelperMethods.h"
#include "LoggerFactory.h"
#include "LoggingHandler.h"
#include "Macros.h"
#include "WorkConservationFlowHandler.h"
#include "WCPrintHandler.h"

namespace WCEnabler {

MainObject::MainObject( std::string& bgAdaptorAddress )
{
	setECNEnabled( true );

	// get the interface's name
	Common::HelperMethods::InterfaceInfo interface = Common::HelperMethods::getInterfaceName();

	_bandwidthValues = new BandwidthValues();
	char usageName[ 100 ] = {0};

//	strcat( usageName, interface.interfaceName.c_str() );
	strcat( usageName, "BandwidthUsage" );
	PRINT("%s\n", usageName );

	// create the threads
	_bandwidthCommunicator = new BandwidthCommunicator(
				bgAdaptorAddress
				, interface.interfaceName
				, _bandwidthValues
				, Common::LoggerFactory::buildLogger( interface.interfaceName, "BandwidthLimit", false )
				, Common::LoggerFactory::buildLogger( interface.interfaceName, usageName, false ) );

	_workConservationFlowHandler = new WorkConservationFlowHandler(
				interface.interfaceName
				, 0.05f
				, 0.05f
				, Common::LoggerFactory::buildLogger( interface.interfaceName, "WorkConservation", false )
				, _bandwidthValues  );

	_bandwidthCalculator = new BandwidthCalculator(
				Common::LoggerFactory::buildLogger( interface.interfaceName, "BandwidthCalculator", false )
				, interface.ipAddress
				, _bandwidthValues
				, interface.interfaceName );
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
