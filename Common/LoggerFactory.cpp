#include "LoggerFactory.h"

#include <algorithm>
#include <sstream>
#include <string.h>

#include "LoggingHandler.h"

namespace Common {

Common::LoggingHandler* LoggerFactory::buildLogger( const std::string& interface, const std::string& filename, bool isCSV )
{
	// remove the -eth0 from the interface name
	char removeInterface[] = "-eth0";
	std::string newInterface( interface );

	// loop through the string, removing appropriate characters
	for ( unsigned int i = 0; i < strlen( removeInterface ); ++i )
	{
		newInterface.erase( remove( ++newInterface.begin(), newInterface.end(), removeInterface[ i ] ), newInterface.end()  );
	}

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
