/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <string>

namespace Common {
class TCControl
{
private:

	TCControl();

private:

//	bool _hasTCBeenModified;
public:

//	static TCControl& instance();

	static void setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth, const std::string& latency );

	static void setEgressBandwidth( const std::string& interface, const int desiredBandwidth, const std::string& latency );

	static void setIgressBandwidth( const std::string& interface, const std::string& desiredBandwidth, const std::string& latency );

	static void setIgressBandwidth( const std::string& interface, const int desiredBandwidth, const std::string& latency );

	static void clearTCCommands( const std::string& interface );
};

} // namespace Common
#endif // TCCONTROL_H
