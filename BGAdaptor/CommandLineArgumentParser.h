#ifndef COMMANDLINEARGUMENTPARSER_H
#define COMMANDLINEARGUMENTPARSER_H

#include <string>

/**
 * @brief	The CommandLineArgumentParser class parses the command line arguments
 * @note	This is a static class
 */
class CommandLineArgumentParser
{
public:

	/**
	 * @brief The UsageArguments struct contains the valid short form arguments
	 */
	struct UsageArguments
	{
		enum
		{
			BandwidthGuarantee	= 'b',
			DynamicAllocation	= 'd',
			Help				= 'h',
			NumberOfHosts		= 'n'
		};
	};

	/**
	 * @brief parseCommandLineArgument parses the command line
	 * @param argc
	 * @param argv
	 * @return
	 */
	static void parseCommandLineArgument( int argc, char*const* argv, uint* dynamicAllocation, uint* numberOfHosts, uint* bandwidthGuarantee );

	// prints the usage to stderr
	static void printUsage();

	~CommandLineArgumentParser();

private:

	/**
	 * @brief CommandLineArgumentParser unimplemented constructor because this class is static
	 */
	CommandLineArgumentParser();
};

#endif // COMMANDLINEARGUMENTPARSER_H
