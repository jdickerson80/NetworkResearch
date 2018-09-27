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
			SingleClient,
			SingleServer
		};
	};

protected:

	typedef std::vector< pid_t > PIDVector;

	const TestData* const _testData;
	IPVector* _ipVector;
	PIDVector _pidVector;
	std::string _removeBandwidthStatsFile;
	std::string _logStatsCommand;

	bool handleTerminatedSubprocess( int status, int pid );

private:

	const Tests::Enum _test;

public:

	virtual ~TestBaseClass() {}

	bool runTest( IPVector* ipVector );

	Tests::Enum test() const;

protected:

	TestBaseClass( const TestData* const data, const Tests::Enum test );

	virtual bool impl_runTest( IPVector* ipVector ) = 0;
};
}

#endif // TESTBASECLASS_H
