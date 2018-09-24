#ifndef SINGLECLIENTTEST_H
#define SINGLECLIENTTEST_H

#include "ThreadHelper.h"
#include "TestBaseClass.h"

namespace TestHandler {

class SingleClientTest : public TestBaseClass
{
private:

	typedef std::vector< pid_t > PIDVector;

private:

	pthread_t _thread;

	IPVector* _ipVector;

	PIDVector _pidVector;
public:

	SingleClientTest( const TestData* const testData );

	~SingleClientTest();

private:

	bool clientTest( const std::string& ipAddress );

	bool impl_runTest( IPVector* ipVector );

	bool handleTerminatedSubprocess( int status, int pid );
};
}
#endif // SINGLECLIENTTEST_H
