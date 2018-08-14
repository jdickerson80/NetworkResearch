#ifndef MAINOBJECT_H
#define MAINOBJECT_H

#include <stdint.h>
#include <string>

// Forward declarations
namespace Common {
class LoggingHandler;
}

// Forward declarations
namespace WCEnabler {
class BandwidthCalculator;
class BandwidthCommunicator;
class WorkConservationFlowHandler;
class BandwidthValues;

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
	BandwidthValues* _bandwidthValues;
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

	/**
	 * @brief bandwidthValues getter
	 * @return return pointer to the bandwidth values
	 */
	const BandwidthValues* const bandwidthValues() const;

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
	 * @param isEnabled true if ecn is enabled, false if ecn is not enabled
	 */
	void setECNEnabled( bool isEnabled );
};

} // namespace WCEnabler

#endif // MAINOBJECT_H