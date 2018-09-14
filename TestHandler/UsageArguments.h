#ifndef USAGEARGUMENTS_H
#define USAGEARGUMENTS_H

#include <stdio.h>

namespace TestHandler {
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

void printUsage()
{
	const char* usage = "-d duration, -h print this usage, -r range of hosts ex. 1 4" \
						" -l log file, -t list of tests\n";
	fprintf( stderr, "%s", usage );
}

}

#endif // USAGEARGUMENTS_H
