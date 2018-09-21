#ifndef SINGLECLIENTTEST_H
#define SINGLECLIENTTEST_H

#include "ThreadHelper.h"
#include "TestBaseClass.h"

namespace TestHandler {

class SingleClientTest : public TestBaseClass
{
private:

	pthread_t _thread;

	IPVector* _ipVector;

public:

	SingleClientTest( const TestData* const testData );

	~SingleClientTest();

private:

	static void* clientTest( void* input );

	bool impl_runTest( IPVector* ipVector );
};
}
#endif // SINGLECLIENTTEST_H
