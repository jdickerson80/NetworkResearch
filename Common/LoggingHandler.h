/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include <string>
#include <fstream>

namespace Common {
class LoggingHandler
{
private:

	const std::string _loggingPath;

	std::ofstream _outputStream;

public:

	LoggingHandler( const std::string& loggingPath );

	~LoggingHandler();

	void log( const std::string& stringToLog );
};

} // namespace Common

#endif // LOGGINGHANDLER_H
