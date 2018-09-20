#ifndef TESTBASECLASS_H
#define TESTBASECLASS_H

#include <string>

namespace TestHandler {

class TestBaseClass
{
public:

	struct Tests
	{
		enum Enum
		{
			Efficiency = 0,
			LongFlowHandling,
			RandomFlowHandling,
			ShortFlowHandling,
			WCBandwidthUtilization,
			WCLogic
		};
	};

private:

	std::string _logPath;
	const Tests::Enum _test;

public:

	virtual ~TestBaseClass() {}
	bool runTest( const std::string& logPath )
	{
		_logPath = logPath;

		return impl_runTest();
	}

protected:

	TestBaseClass( const Tests::Enum test )
		: _logPath()
		, _test( test )
		{}

	virtual bool impl_runTest() = 0;
};
}

#endif // TESTBASECLASS_H
