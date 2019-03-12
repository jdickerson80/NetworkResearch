#include "TCControl.h"

#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#include "WCPrintHandler.h"
#define UseTC ( 1 )
using namespace std;

namespace Common {

void TCControl::setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth )
{
#if defined( UseTC )
	setEgressBandwidth( interface, atoi( desiredBandwidth.c_str() ) );
#endif
}

void TCControl::setEgressBandwidth( const std::string& interface, const int desiredBandwidth )
{
#if defined( UseTC )
	/// clear the tc commands
	clearTCCommands( interface );
	std::string command;
	ostringstream stream;

	// create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
	// where all traffic not covered in the filters goes to flow 11
	stream << "tc qdisc add dev " << interface <<
			  " root handle 1: htb default 11;\n";

	// add class to root qdisc with id 1:1 with htb with the max rate given as the argument
	stream << "tc class add dev " << interface <<
			  " parent 1: classid 1:1 htb rate " << desiredBandwidth << "mbit;\n";

	// add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
	// as high as the max rate where the queue has the highest priority.
	// This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
	stream << "tc class add dev " << interface <<
			  " parent 1:1 classid 1:11 htb rate " << desiredBandwidth << "mbit ceil " << desiredBandwidth << "mbit prio 0;\n";

	// add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
	// with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
	stream << "tc class add dev " << interface <<
			  " parent 1:1 classid 1:12 htb rate 0.1kbit ceil " << desiredBandwidth << "mbit prio 1;\n";

	// create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
	// this filters all BwG tcp packets into class 1:11
	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11;\n";

	// create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
	// this filters all WC tcp packets into class 1:12
	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12;\n";

	// create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
	// this filters all BwG udp packets into class 1:11
	stream << "tc filter add dev " << interface <<
			  " parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11;\n";

	command = stream.str();
	PRINT( "egress: %s\n", command.c_str() );
	system( command.c_str() );
#endif
}

void TCControl::clearTCCommands( const string& interface )
{
#if defined( UseTC )
	std::string command;
	ostringstream stream;

	stream << "tc qdisc del dev " << interface << " root;";

	command = stream.str();
	PRINT( "clear: %s\n", command.c_str() );
	system( command.c_str() );
#endif
}

} /// namespace Common
