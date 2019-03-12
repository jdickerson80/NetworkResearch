#include "CommandLineArgumentParser.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "BGPrintHandler.h"

using namespace std;

static char usage[] = \
	"Usage: BGAdaptor [-h] [-b] [-d]\n" \
				"-b, --bandwidthGuarantee\n" \
				"-d, --dynamicAllocation\n" \
				"-h, --help\t\tprint this help message\n" \
				"-n, --numberOfHosts\n";

// setup the long options
static struct option longOptions[] =
{
	{ "bandwidthGuarantee",	required_argument,	nullptr, CommandLineArgumentParser::UsageArguments::BandwidthGuarantee },
	{ "dynamicAllocation",	required_argument,	nullptr, CommandLineArgumentParser::UsageArguments::DynamicAllocation },
	{ "help",				no_argument,		nullptr, CommandLineArgumentParser::UsageArguments::Help },
	{ "numberOfHost",		required_argument,	nullptr, CommandLineArgumentParser::UsageArguments::NumberOfHosts },
	{ nullptr,				0,					nullptr, 0  }
};

void CommandLineArgumentParser::parseCommandLineArgument( int argc, char*const* argv, uint* dynamicAllocation, uint* numberOfHosts, uint* bandwidthGuarantee )
{
	int opt;

	// parse the user's arguments
	while ( ( opt = getopt_long( argc, argv, "b:d:n:h", longOptions, nullptr ) ) != -1 )
	{
		switch ( opt )
		{
		case UsageArguments::BandwidthGuarantee:
			PRINT("%s\n", optarg );
			*bandwidthGuarantee = static_cast< uint >( atoi( optarg ) );
			break;

		case UsageArguments::DynamicAllocation:
			PRINT("%s\n", optarg );
			*dynamicAllocation = static_cast< uint >( atoi( optarg ) );
			break;

		case UsageArguments::NumberOfHosts:
			PRINT("%s\n", optarg );
			*numberOfHosts = static_cast< uint >( atoi( optarg ) );
			break;
		}
	}
}

void CommandLineArgumentParser::printUsage()
{
	perror( usage );
}
