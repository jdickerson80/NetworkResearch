#ifndef CLIENTSERVERTEST_H
#define CLIENTSERVERTEST_H

#include "TestBaseClass.h"
#include <iostream>

namespace TestHandler {

class ClientServerTest : public TestBaseClass
{
public:

	ClientServerTest( const TestData* const testData );

	~ClientServerTest();

private:



private:


	struct ServerClientData
	{
		std::string ipAddress;
		unsigned int port;
		const TestData* const testData;

		ServerClientData( const std::string& ipAddress, unsigned int port, const TestData* const testData )
			: ipAddress( ipAddress )
			, port( port )
			, testData( testData )
		{}

		ServerClientData( const TestData* const testData )
			: ipAddress()
			, port( 0 )
			, testData( testData )
		{}

		friend std::ostream& operator<<( std::ostream& os, const ServerClientData& serverClientData )
		{
			os << "IP: " << serverClientData.ipAddress << std::endl;
			os << "Port: " << serverClientData.port << std::endl;
			os << "TestData: " << serverClientData.testData << std::endl;
			return os;
		}
	};

private:

	bool impl_runTest( IPVector* ipVector );

	static void* clientTest( void* input );

	bool serverTest( unsigned int port = 5001 );
};

}
#endif // CLIENTSERVERTEST_H
