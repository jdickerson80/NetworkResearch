#ifndef TESTDATA_H
#define TESTDATA_H

#include <string>

namespace TestHandler {
struct TestData
{
	bool runInParallel = 0;
    unsigned int bytesToBeTransmitted = 0;
    unsigned int duration;
    std::string logPath;
    unsigned int targetBandwidth;
};
}

#endif // TESTDATA_H
