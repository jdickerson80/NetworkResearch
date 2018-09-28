#ifndef MAINOBJECT_H
#define MAINOBJECT_H

#include <string>
#include <vector>

namespace TestHandler {

class TestBaseClass;
class TestData;

class MainObject
{
public:

	typedef std::vector< std::string* > IPVector;
	typedef std::vector< TestBaseClass* > TestVector;

private:

	IPVector _ipVector;
	TestData* _testData;
	TestVector _testToRun;
	std::string _removeBandwidthStatsFile;
	std::string _logStatsCommand;

public:

	MainObject( int argc, char* const* argv );

	~MainObject();

	void setRunning( bool isRunning );

	bool isRunning() const;

	bool runTests();

private:

	bool parseCommandLineArguments( int argc, char*const* argv );

	/**
	 * @brief	signalHandler method handles all of the signals that come from Linux.
	 *			For instance, this method catches when the user presses Ctrl + C or
	 *			when the user presses X on the terminal window. This method sets the
	 *			_isRunning flag to false, triggering main to exit the app.
	 * @param	signal to be received from Linux
	 */
	static void signalHandler( int signal );

	void deleteIPVector();

	void deleteTestVector();
};

}

#endif // MAINOBJECT_H
