#ifndef SINGLECLIENTTEST_H
#define SINGLECLIENTTEST_H

#include "TestBaseClass.h"

namespace TestHandler {

class SingleClientTest : public TestBaseClass
{
public:

	SingleClientTest( const TestData* const testData );

	~SingleClientTest();

private:

	bool clientTest( const std::string& ipAddress, unsigned int port );

	bool impl_runTest( IPVector* ipVector );
};
}
#endif // SINGLECLIENTTEST_H
