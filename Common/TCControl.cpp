#include "TCControl.h"

#include <stdlib.h>
#include <sstream>
#include "LoggingHandler.h"
#include <stdio.h>

using namespace std;

//TCControl::TCControl()
//	: _hasTCBeenModified( false )
//{
//}

//TCControl& TCControl::instance()
//{
//	static TCControl control;
//	return control;
//}
namespace Common {
void TCControl::setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth, const std::string& latency )
{
	clearTCCommands( interface );
	std::string command;
	ostringstream stream;

	stream << "tc qdisc add dev " << interface << "-eth0 handle 1: root htb default 11;\n";
	stream << "tc class add dev " << interface << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";
	stream << "tc class add dev " << interface << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit;\n";

	command = stream.str();
	system( command.c_str() );
}

void TCControl::setEgressBandwidth( const std::string& interface, const int desiredBandwidth, const std::string& latency )
{
	clearTCCommands( interface );
	std::string command;
	ostringstream stream;

	stream << "tc qdisc add dev " << interface << "-eth0 handle 1: root htb default 11;\n";
	stream << "tc class add dev " << interface << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";
	stream << "tc class add dev " << interface << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit;\n";

	command = stream.str();
	system( command.c_str() );
}

void TCControl::setIgressBandwidth( const std::string& interface, const std::string& desiredBandwidth, const std::string& latency )
{
//	stream << "tc class add dev " << interface << " parent 1: classid 1:2 htb rate $UPLD";

}

void TCControl::setIgressBandwidth( const std::string& interface, const int desiredBandwidth, const std::string& latency )
{

}

void TCControl::clearTCCommands( const string& interface )
{
	std::string command;
	ostringstream stream;

	stream << "tc qdisc del dev " << interface << "-eth0 root;";

	command = stream.str();
//	printf("!!!!!!!!!!!!!!%s com is %s\n", interface.c_str(), command.c_str() );
	system( command.c_str() );
}

} // namespace Common
