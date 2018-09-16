#ifndef COMMANDLINEARGUMENTPARSER_H
#define COMMANDLINEARGUMENTPARSER_H

#include <string>
#include <vector>

namespace TestHandler {
class CommandLineArgumentParser
{
public:

	struct UsageArguments
	{
		enum
		{
			Duration	= 'd',
			Help		= 'h',
			HostRange	= 'r',
			LogFile		= 'l',
			Test		= 't'
		};
	};

private:
#define BUFFERSIZE ( 1024 )
	unsigned int _start;
	unsigned int _finish;

	char _startIP[ BUFFERSIZE ];
	char _endIP[ BUFFERSIZE ];
	char _lastOctet[ BUFFERSIZE ];


public:
	CommandLineArgumentParser();


	void parseCommandLineArguments( int argc, char*const* argv, std::vector< std::string* >& ipVector );

	static void printUsage();

private:

	inline void parseIPRange( char* optarg, std::vector< std::string* >& ipVector );

	inline void fillIPVector( std::vector< std::string* >& ipVector );

};
}

#endif // COMMANDLINEARGUMENTPARSER_H
