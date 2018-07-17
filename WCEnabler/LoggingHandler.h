#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include <string>
#include <fstream>

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

#endif // LOGGINGHANDLER_H
