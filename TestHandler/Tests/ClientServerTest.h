#ifndef CLIENTSERVERTEST_H
#define CLIENTSERVERTEST_H

#include "TestBaseClass.h"

namespace TestHandler {

class ClientServerTest : public TestBaseClass
{
public:

	ClientServerTest( const TestData* const testData );

	~ClientServerTest();

private:

	bool impl_runTest( IPVector* ipVector );

	bool clientTest( const std::string& ipAddress, unsigned int port );

	bool serverTest( const std::string& ipAddress, unsigned int port );
};

}
#endif // CLIENTSERVERTEST_H
