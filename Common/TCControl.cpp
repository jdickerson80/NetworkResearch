#include "TCControl.h"

#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#define UseTC ( 1 )
using namespace std;

namespace Common {

void TCControl::setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth )
{
#if defined( UseTC )
	/// clear the tc commands
	clearTCCommands( interface );
//	std::string command;
//	ostringstream stream;

//	stream << "tc qdisc add dev " << interface <<
//			  " root handle 1: htb default 11;\n";

//	stream << "tc class add dev " << interface <<
//			  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";

//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";

//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";

//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

//	stream << "tc qdisc add dev " << interface <<
//			  " root handle 1: htb default 11;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";
//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";
//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11;\n";
//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

//	command = stream.str();
//	system( command.c_str() );
#endif

///	stream.flush();

///	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x00 -j MARK --set-mark 0x10;\n";

///	stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x38 -j MARK --set-mark 0x11;\n";

///	command = stream.str();
///	system( command.c_str() );
}

void TCControl::setEgressBandwidth( const std::string& interface, const int desiredBandwidth )
{
#if defined( UseTC )
	/// clear the tc commands
	clearTCCommands( interface );
	std::string command;
	ostringstream stream;

//	printf("db %i\n", desiredBandwidth );

	stream << "tc qdisc add dev " << interface <<
			  " root handle 1: htb default 11;\n";

	stream << "tc class add dev " << interface <<
			  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:11 htb rate 400mbit ceil 400mbit prio 0;\n";

	stream << "tc class add dev " << interface <<
			  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";

	stream << "tc class add dev " << interface <<
			  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";

	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";

	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11;\n";



//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";

//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11;\n";



//	stream << "tc qdisc add dev " << interface <<
//			  " root handle 1: htb default 11;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";
//	stream << "tc class add dev " << interface <<
//			  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";
//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";
//	stream << "tc filter add dev " << interface <<
//			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";


	command = stream.str();
//	printf("\n%s\n", command.c_str() );
	system( command.c_str() );
#endif

///	clearTCCommands( interface );
///	string command;
///	ostringstream stream;

///	// stream the command into the string
///	stream << "tc qdisc add dev " << interface << "-eth0 handle 1: root htb default 11;\n";
///	stream << "tc class add dev " << interface << "-eth0 parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

///	/// @todo move this into igress method??
///	stream << "tc class add dev " << interface << "-eth0 parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit;\n";

///	command = stream.str();
///	system( command.c_str() );
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
#if defined( UseTC )
	std::string command;
	ostringstream stream;

	stream << "tc qdisc del dev " << interface << " root;";

	command = stream.str();
	system( command.c_str() );
#endif
}

} /// namespace Common


//stream << "tc qdisc add dev " << interface <<
//		  " root handle 1: htb;\n";

//stream << "tc class add dev " << interface <<
//		  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

//stream << "tc class add dev " << interface <<
//		  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";

//stream << "tc class add dev " << interface <<
//		  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";

//stream << "tc filter add dev " << interface <<
//		  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";

//stream << "tc filter add dev " << interface <<
//		  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

//stream << "tc qdisc add dev " << interface <<
//		  " root handle 1: htb default 11;\n";
//stream << "tc class add dev " << interface <<
//		  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";
//stream << "tc class add dev " << interface <<
//		  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";
//stream << "tc class add dev " << interface <<
//		  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";
//stream << "tc filter add dev " << interface <<
//		  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";
//stream << "tc filter add dev " << interface <<
//		  " parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11;\n";
//stream << "tc filter add dev " << interface <<
//		  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

///// set up the rate limit on the interface
//stream << "tc class add dev " << interface
//	   << " parent 1: classid 1:1 htb rate " << desiredBandwidth
//	   << "mbit ceil "<< desiredBandwidth << "mbit;\n";

//stream << "tc class add dev " << interface
//	   << " parent 1:1 classid 1:10 htb rate " << desiredBandwidth
//	   << "mbit ceil "<< desiredBandwidth << "mbit prio 0;\n";

//stream << "tc class add dev " << interface
//	   << " parent 1:1 classid 1:11 htb rate " << desiredBandwidth
//	   << "mbit ceil "<< desiredBandwidth << "mbit prio 1;\n";

//stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x00 -j MARK --set-mark 0x10;\n";

//stream << "iptables -t mangle -A PREROUTING -m tos --tos 0x38 -j MARK --set-mark 0x11;\n";

//command = stream.str();
//system( command.c_str() );
