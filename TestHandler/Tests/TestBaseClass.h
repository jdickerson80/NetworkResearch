#ifndef TESTBASECLASS_H
#define TESTBASECLASS_H

#include <string>
#include <vector>

#include "TestData.h"

namespace TestHandler {

class TestBaseClass
{
public:

	typedef std::vector< std::string* > IPVector;

	struct Tests
	{
		enum Enum
		{
			ClientServer = 0,
			Efficiency,
			LongFlowHandling,
			RandomFlowHandling,
			SingleClient,
			SingleServer,
			ShortFlowHandling,
			WCBandwidthUtilization,
			WCLogic
		};
	};

protected:

//	const IPVector* ipVector;
	const TestData* const _testData;

private:

	const Tests::Enum _test;

public:

	virtual ~TestBaseClass() {}

	bool runTest( IPVector* ipVector )
	{
		return impl_runTest( ipVector );
	}

//	bool runTest( const std::string& logPath )
//	{
//		logPath = logPath;

//		return impl_runTest();
//	}

protected:

	TestBaseClass( const TestData* const data, const Tests::Enum test )
		: _testData( data )
		, _test( test )
		{}

	virtual bool impl_runTest( IPVector* ipVector ) = 0;
};
}

#endif // TESTBASECLASS_H
