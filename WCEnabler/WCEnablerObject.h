#ifndef WCENABLEROBJECT_H
#define WCENABLEROBJECT_H

#include <string>

// Forward declarations
namespace Common {
class LoggingHandler;
}

class BandwidthCalculator;
class BandwidthCommunicator;
class WorkConservationFlowHandler;

/**
 * @brief	The WCEnablerObject class is the main object of the app. This class holds all of the
 *			instances of the classes.
 */
class WCEnablerObject
{
private:

	BandwidthCalculator* _bandwidthCalculator;
	BandwidthCommunicator* _bandwidthCommunicator;
	std::string _ipAddress;
	WorkConservationFlowHandler* _workConservationFlowHandler;

public:

	/**
	 * @brief WCEnablerObject constructor for the app
	 * @param interface ip address of the interface
	 * @param bgAdaptorIPAddress ip address of the BGAdaptor module
	 */
	WCEnablerObject( const std::string& bgAdaptorIPAddress );

	~WCEnablerObject();

private:

	/**
	 * @brief	buildLogger method creates the appropriate logging handler and returns a pointer to it.
	 * @param interface string of the interface
	 * @param filename string of the filename
	 * @return pointer to the logging handler
	 */
	static Common::LoggingHandler* buildLogger( const std::string& interface, const std::string& filename );
};

#endif // WCENABLEROBJECT_H
