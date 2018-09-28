#ifndef TESTDATA_H
#define TESTDATA_H

#include <string>
#include <string.h>

namespace TestHandler {
struct TestData
{
#define THISBUFFERSIZE ( 10 )
	bool runInParallel = 0;
	char bytesToBeTransmitted[ THISBUFFERSIZE ];
	char duration[ THISBUFFERSIZE ];
    std::string logPath;
	std::string port;
	char targetBandwidth[ THISBUFFERSIZE ];

	TestData()
	{
		memset( bytesToBeTransmitted, 0, THISBUFFERSIZE );
		memset( duration, 0, THISBUFFERSIZE );
		memset( targetBandwidth, 0, THISBUFFERSIZE );

		memset( bytesToBeTransmitted, '-', 1 );
		memset( duration, '-', 1 );
		memset( targetBandwidth, '-', 1 );
	}
};
}

#endif // TESTDATA_H
