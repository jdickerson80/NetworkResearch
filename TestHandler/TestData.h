#ifndef TESTDATA_H
#define TESTDATA_H

#include <string>

namespace TestHandler {
struct TestData
{
    unsigned int bytesToBeTransmitted = 0;
    unsigned int duration;
    std::string logPath;
    unsigned int targetBandwidth;
};
}

#endif // TESTDATA_H
