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

	/**
	 * @brief MainObject constructor for the app
	 */
	MainObject();

private:

	BandwidthCalculator* _bandwidthCalculator;
	BandwidthCommunicator* _bandwidthCommunicator;
	std::string _ipAddress;
	WorkConservationFlowHandler* _workConservationFlowHandler;

public:

	/**
	 * @brief instance instance method is the Singleton method of creating and accessing this
	 *			class. This ensures there can only be ONE instance of this class.
	 * @return reference to this class.
	 * @note	https://en.wikipedia.org/wiki/Singleton_pattern
	 */
	static MainObject& instance();

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
};

} // namespace WCEnabler

#endif // MAINOBJECT_H
