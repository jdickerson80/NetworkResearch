#include "TCControl.h"

#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace Common {

void TCControl::setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth )
{
//	// clear the tc commands
//	clearTCCommands( interface );
//	std::string command;
//	ostringstream stream;

//	// set up the qdisc
//	stream << "tc qdisc add dev " << interface << "-eth0 root handle 1: htb default 11;\n";

//	// set up the rate limit on the interface
//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit;\n";

//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1:1 classid 1:10 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit prio 0;\n";

//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit prio 1;\n";

////	command = stream.str();
////	system( command.c_str() );

////	stream.flush();

//	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x00 -j MARK --set-mark 0x10;\n";

//	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x38 -j MARK --set-mark 0x11;\n";

//	command = stream.str();
//	system( command.c_str() );
}

void TCControl::setEgressBandwidth( const std::string& interface, const int desiredBandwidth )
{
//	// clear the tc commands
//	clearTCCommands( interface );
//	std::string command;
//	ostringstream stream;

//	// set up the qdisc
//	stream << "tc qdisc add dev " << interface << "-eth0 root handle 1: htb default 11;\n";

//	// set up the rate limit on the interface
//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit;\n";

//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1:1 classid 1:10 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit prio 0;\n";

//	stream << "tc class add dev " << interface
//		   << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth
//		   << "mbit ceil "<< desiredBandwidth << "mbit prio 1;\n";

//	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x00 -j MARK --set-mark 0x10;\n";

//	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x38 -j MARK --set-mark 0x11;\n";

//	command = stream.str();
//	system( command.c_str() );

//	clearTCCommands( interface );
//	std::string command;
//	ostringstream stream;

//	// stream the command into the string
//	stream << "tc qdisc add dev " << interface << "-eth0 handle 1: root htb default 11;\n";
//	stream << "tc class add dev " << interface << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

//	/// @todo move this into igress method??
//	stream << "tc class add dev " << interface << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit;\n";

//	command = stream.str();
//	system( command.c_str() );
}

void TCControl::setIgressBandwidth( const std::string& interface, const std::string& desiredBandwidth )
{
	/// @todo fill this method in
}

void TCControl::setIgressBandwidth( const std::string& interface, const int desiredBandwidth )
{
	/// @todo fill this method in
}

void TCControl::clearTCCommands( const string& interface )
{
//	std::string command;
//	ostringstream stream;

//	stream << "tc qdisc del dev " << interface << "-eth0 root;";

//	command = stream.str();
//	system( command.c_str() );
}

} // namespace Common
