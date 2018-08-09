#ifndef MAINOBJECT_H
#define MAINOBJECT_H

#include <string>

// Forward declarations
namespace Common {
class LoggingHandler;
}

namespace WCEnabler {
class BandwidthCalculator;
class BandwidthCommunicator;
class WorkConservationFlowHandler;

/**
 * @brief	The WCEnablerObject class is the main object of the app. This class holds all of the
 *			instances of the classes.
 */
class MainObject
{
private:

	BandwidthCalculator* _bandwidthCalculator;
	BandwidthCommunicator* _bandwidthCommunicator;
	std::string _ipAddress;
	WorkConservationFlowHandler* _workConservationFlowHandler;

public:

	/**
	 * @brief MainObject constructor for the app
	 * @param interface ip address of the interface
	 * @param bgAdaptorIPAddress ip address of the BGAdaptor module
	 */
	MainObject( const std::string& bgAdaptorIPAddress );

	~MainObject();

private:

	/**
	 * @brief	buildLogger method creates the appropriate logging handler and returns a pointer to it.
	 * @param interface string of the interface
	 * @param filename string of the filename
	 * @return pointer to the logging handler
	 */
	static Common::LoggingHandler* buildLogger( const std::string& interface, const std::string& filename, bool isCSV );

	/**
	 * @brief getInterfaceName getter
	 * @return the name of the interface
	 */
	std::string getInterfaceName();

	/**
	 * @brief setECNEnabled setter
	 * @param isEnabled true if ECN is enabled, false if ECN is disabled
	 */
	static void setECNEnabled( bool isEnabled );
};

} // namespace WCEnabler

#endif // MAINOBJECT_H
