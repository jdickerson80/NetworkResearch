#ifndef FILECONTROL_H
#define FILECONTROL_H

#include <string>

class FileControl
{	
public:

	static std::string buildReceivePath( const std::string& interface );
	static std::string buildSendPath( const std::string& interface );
	static std::string buildOutputFilePath( const std::string& interface );

private:
	FileControl();

};

#endif // FILECONTROL_H
