#include "FileControl.h"
#include <sstream>

using namespace std;

FileControl::FileControl()
{
}

string FileControl::buildReceivePath( const string& interface )
{
	ostringstream stream;
	stream << "/sys/class/net/" << interface << "-eth0/statistics/rx_bytes";
	return stream.str();
}

string FileControl::buildSendPath( const string& interface )
{
	ostringstream stream;
	stream << "/sys/class/net/" << interface << "-eth0/statistics/tx_bytes";
	return stream.str();
}

string FileControl::buildOutputFilePath( const string& interface )
{
	ostringstream stream;
	stream << "/tmp/" << interface << "/" << interface << ".out";
	return stream.str();
}

