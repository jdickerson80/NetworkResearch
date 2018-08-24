#include "LoggerFactory.h"

#include <algorithm>
#include <sstream>
#include <string.h>

#include "LoggingHandler.h"

namespace Common {

Common::LoggingHandler* LoggerFactory::buildLogger( const std::string& interface, const std::string& filename, bool isCSV )
{
	// copy the interface name
	std::string newInterface( interface );

	/// @note if there are more than 1 '-' in the string, it will remove
	/// EVERYTHING past the 1st '-'
	// find the '-' in the string and erase it and everything past it
	newInterface.erase( newInterface.begin() + newInterface.find( '-' ), newInterface.end() );

	// stream the logging string, create and return the pointer to it
	std::ostringstream stream;
	stream << "/tmp/" << newInterface << "/" << filename;

	// check if the output file should be a .csv
	if ( isCSV )
	{
		stream << ".csv";
	}
	else // not a .csv field
	{
		stream << ".log";
	}

	return new Common::LoggingHandler( stream.str() );
}

} // namespace Common
