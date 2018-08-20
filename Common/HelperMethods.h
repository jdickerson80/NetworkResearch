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
};

} // namespace Common

#endif // HELPERMETHODS_H
