#ifndef HELPERMETHODS_H
#define HELPERMETHODS_H

#include <string>

namespace Common {

class HelperMethods
{
public:

	struct InterfaceInfo
	{
		std::string interfaceName;
		std::string ipAddress;

		InterfaceInfo()
			: interfaceName()
			, ipAddress()
		{}
	};

private:

	HelperMethods();

public:

	/**
	 * @brief getInterfaceName getter
	 * @return the name of the interface
	 */
	static InterfaceInfo getInterfaceName();

	/**
	 * @brief getHostName getter
	 * @return the host name. this is the interface name without the -eth0
	 */
	static std::string getHostName();
};

} // namespace Common

#endif // HELPERMETHODS_H
