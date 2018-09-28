#include "PrintUsage.h"

#include <stdio.h>

namespace TestHandler {
void printUsage()
{
	const char* const usage = \
			"Usage: TestHandler [-b] [-h] [-l] [-n] [-p] [-q] [-r] [-t] [-z]\n" \
			"-b, --bitrate\ttarget bitrate in bits/sec. (0 for unlimited)\n" \
			"-h, --help\tprint this help message\n" \
			"-l, --logfile\tpath and name of the file to log test results\n" \
			"-n, --bytes\tbytes to be transmitted\n" \
			"-p, --port\tport to use\n" \
			"-q, --parallel\truns all tests in parallel. So, all of the ip range will run simultaneously\n" \
			"-r, --range\trange of ip addresses with a hyphen(-) being a range and a comma (,) inidividual IP address\n" \
			"\t\te.g. ""10.0.0.1-10.0.0.4,10.0.0.18,10.0.0.19,10.0.0.20""\n" \
			"-t, --time\tduration of the test in seconds\n" \
			"-z, --test\tcomma separated list of test to run. tests are:\n"  \
			"\t\te.g. ""SingleClient: Runs a single client with the given settings on the range of IPs""\n" \
			"\t\te.g. ""SingleServer: Runs a single server with the given settings""\n";
	fprintf( stderr, "%s", usage );
}
}
