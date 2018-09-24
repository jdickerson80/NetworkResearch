#ifndef COMMANDLINEARGUMENTPARSER_H
#define COMMANDLINEARGUMENTPARSER_H

#include <string>
#include <vector>

namespace TestHandler {

class TestBaseClass;
class TestData;

class CommandLineArgumentParser
{
public:

	struct UsageArguments
	{
		enum
		{
			Bytes				= 'n',
			Duration			= 't',
			Help				= 'h',
			HostRange			= 'r',
			LogFile				= 'l',
			ParallelTests		= 'p',
			Targetbandwidth		= 'b',
			Test				= 'z'
		};
	};

private:
#define BUFFERSIZE ( 1024 )
	bool _goodParse;
	unsigned int _start;
	unsigned int _finish;

	TestData* _testData;

	char _startIP[ BUFFERSIZE ];
	char _endIP[ BUFFERSIZE ];
	char _lastOctet[ BUFFERSIZE ];


public:
	CommandLineArgumentParser();
	~CommandLineArgumentParser();

	bool parseCommandLineArguments(
			int argc
			, char*const* argv
			, TestData* testData
			, std::vector< std::string* >& ipVector
			, std::vector< TestBaseClass* >& test );

private:

	inline void parseIPRange( char* optarg, std::vector< std::string* >& ipVector );

	inline void fillIPVector( std::vector< std::string* >& ipVector );

	inline void parseTests( char* optarg, std::vector< TestBaseClass* >& test );

	inline TestBaseClass* getTest( const char* const testString );

};
}

#endif // COMMANDLINEARGUMENTPARSER_H
