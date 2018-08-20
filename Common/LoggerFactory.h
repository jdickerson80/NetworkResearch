#ifndef LOGGERFACTORY_H
#define LOGGERFACTORY_H

#include <string>

namespace Common {

class LoggingHandler;

class LoggerFactory
{
public:

	/**
	 * @brief	buildLogger method creates the appropriate logging handler and returns a pointer to it.
	 * @param interface string of the interface
	 * @param filename string of the filename
	 * @return pointer to the logging handler
	 */
	static Common::LoggingHandler* buildLogger( const std::string& interface, const std::string& filename, bool isCSV );

private:

	LoggerFactory();
};

} // namespace Common

#endif // LOGGERFACTORY_H
