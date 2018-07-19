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
    _outputStream.open( _loggingPath.c_str(), std::ofstream::out | std::ofstream::app );
    _outputStream << stringToLog;
    _outputStream.close();
}

} // namespace Common
