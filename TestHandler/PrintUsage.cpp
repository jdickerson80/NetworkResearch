#include "PrintUsage.h"

#include <stdio.h>

namespace TestHandler {
void printUsage()
{
	const char* const usage = \
			"Usage: TestHandler [-d] [-h] [-l] [-r] [-t]\n" \
			"-d, --duration\tduration of the test in seconds\n" \
			"-h, --help\tprint this help message\n" \
			"-l, --logfile\tpath and name of the file to log test results\n" \
			"-r, --range\trange of ip addresses with a hyphen(-) being a range and a comma (,) inidividual IP address\n" \
			"\t\te.g. ""10.0.0.1-10.0.0.4,10.0.0.18,10.0.0.19,10.0.0.20""\n" \
			"-t, --test\tcomma separated list of test to run. tests are: todo\n";
	fprintf( stderr, "%s", usage );
}
}
