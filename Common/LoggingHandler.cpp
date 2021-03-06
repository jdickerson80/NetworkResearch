#include "LoggingHandler.h"

namespace Common {

LoggingHandler::LoggingHandler( const std::string& loggingPath )
    : _loggingPath( loggingPath )
{

}

LoggingHandler::~LoggingHandler()
{

}

void LoggingHandler::log( const std::string& stringToLog )
{
	// put the string into the file
	_outputStream.open( _loggingPath.c_str(), std::ofstream::out | std::ofstream::app );
    _outputStream << stringToLog;
	_outputStream.close();
}

void LoggingHandler::log( const char* const stringToLog )
{
	// put the string into the file
	_outputStream.open( _loggingPath.c_str(), std::ofstream::out | std::ofstream::app );
	_outputStream << stringToLog;
	_outputStream.close();
}

const std::string &LoggingHandler::loggingPath() const
{
	return _loggingPath;
}

} // namespace Common
