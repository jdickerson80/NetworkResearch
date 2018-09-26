#ifndef SINGLESERVERTEST_H
#define SINGLESERVERTEST_H

#include "TestBaseClass.h"

namespace TestHandler {
class SingleServerTest : public TestBaseClass
{
public:
	SingleServerTest( const TestData* const testData );

	~SingleServerTest();

private:

	bool serverTest( const std::string& ipAddress, unsigned int port );

	bool impl_runTest( IPVector* ipVector );
};
}

#endif // SINGLESERVERTEST_H
