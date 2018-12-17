#include "CommandLineArgumentParser.h"

#include <getopt.h>
#include <stdio.h>

using namespace std;

static char usage[] = \
	"Usage: WCEnabler [-h] [-b]\n" \
				"-h, --help\t\tprint this help message\n" \
				"-b, --bgAdaptorAddress\t\t\n";

// setup the long options
static struct option longOptions[] =
{
	{ "bgAdaptorAddress",	required_argument,	nullptr, CommandLineArgumentParser::UsageArguments::BGAdaptorAddress },
	{ "help",				no_argument,		nullptr, CommandLineArgumentParser::UsageArguments::Help },
	{ nullptr,				0,					nullptr, 0  }
};

std::string CommandLineArgumentParser::parseCommandLineArgument( int argc, char*const* argv )
{
	int opt;
	string bgAdaptorRate = string();

	// parse the user's arguments
	while ( ( opt = getopt_long( argc, argv, "hb:", longOptions, nullptr ) ) != -1 )
	{
		switch ( opt )
		{
		case UsageArguments::BGAdaptorAddress:
			bgAdaptorRate = optarg;
			break;
		}
	}

	return bgAdaptorRate;
}

void CommandLineArgumentParser::printUsage()
{
	perror( usage );
}
