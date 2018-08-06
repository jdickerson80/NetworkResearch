/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <string>

namespace Common {

/**
 * @brief The TCControl class wraps the appropriate system call to TC in Linux.
 * @note This class consists of all static methods
 */
class TCControl
{
private:

	TCControl();

public:

	/**
	 * @brief setEgressBandwidth set the outgoing bandwidth rate on an interface
	 * @param interface to limit the outgoing rate
	 * @param desiredBandwidth in megabits
	 */
	static void setEgressBandwidth( const std::string& interface, const std::string& desiredBandwidth );

	/**
	 * @brief setEgressBandwidth
	 * @param interface to limit the outgoing rate
	 * @param desiredBandwidth in megabits
	 */
	static void setEgressBandwidth( const std::string& interface, const int desiredBandwidth );

	/**
	 * @brief setIgressBandwidth
	 * @param interface to limit the incoming rate
	 * @param desiredBandwidth in megabits
	 */
	static void setIgressBandwidth( const std::string& interface, const std::string& desiredBandwidth );

	/**
	 * @brief setIgressBandwidth
	 * @param interface to limit the incoming rate
	 * @param desiredBandwidth in megabits
	 */
	static void setIgressBandwidth( const std::string& interface, const int desiredBandwidth );

	/**
	 * @brief clearTCCommands clears the current tc settings
	 * @param interface to limit the incoming rate
	 */
	static void clearTCCommands( const std::string& interface );
};

} // namespace Common
#endif // TCCONTROL_H
