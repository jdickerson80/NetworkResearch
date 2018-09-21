#include "ThreadHelper.h"
#include <stdio.h>

namespace Common  {

int ThreadHelper::startDetachedThread( pthread_t* thread, StartRoutine startRoutine, bool* isRunningFlag, void* objectPointer )
{
	if ( *isRunningFlag )
	{
		return -1;
	}

	int returnValue = 0;

	pthread_attr_t attribute;

	pthread_attr_init( &attribute );

	pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_DETACHED );

	returnValue |= pthread_create( thread, &attribute, startRoutine, objectPointer );

	pthread_attr_destroy( &attribute );

	if( returnValue )
	{
		perror( "startDetachedThread: Failed to create the thread" );
		*isRunningFlag = false;
		return -1;
	}
	else
	{
		*isRunningFlag = true;
		return 0;
	}
}

int ThreadHelper::startDetachedThread( pthread_t* thread, StartRoutine startRoutine, std::atomic_bool& isRunningFlag, void* objectPointer )
{
	if ( isRunningFlag.load() )
	{
		return -1;
	}

	int returnValue = 0;

	pthread_attr_t attribute;

	pthread_attr_init( &attribute );

	pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_DETACHED );

	returnValue |= pthread_create( thread, &attribute, startRoutine, objectPointer );

	pthread_attr_destroy( &attribute );

	if ( returnValue )
	{
		perror( "startDetachedThread: Failed to create the thread" );
		isRunningFlag = false;
		return -1;
	}
	else
	{
		isRunningFlag = true;
		return 0;
	}
}

int ThreadHelper::startJoinableThread( pthread_t* thread, StartRoutine startRoutine, bool* isRunningFlag, void* objectPointer )
{
	int val;
	if ( *isRunningFlag )
	{
		return -1;
	}

	val = startJoinableThread( thread, startRoutine, objectPointer );

	if ( val != 0 )
	{
		*isRunningFlag = false;
	}

	return val;
}

int ThreadHelper::startJoinableThread( pthread_t* thread, StartRoutine startRoutine, void* objectPointer )
{
	int returnValue = 0;

	pthread_attr_t attribute;

	pthread_attr_init( &attribute );

	pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_JOINABLE );

	returnValue |= pthread_create( thread, &attribute, startRoutine, objectPointer );

	pthread_attr_destroy( &attribute );

	if ( returnValue )
	{
		perror( "startJoinableThread: Failed to create the thread" );
		return -1;
	}
	else
	{
		return 0;
	}
}

int ThreadHelper::startJoinableThread( pthread_t* thread, StartRoutine startRoutine, std::atomic_bool& isRunningFlag, void* objectPointer )
{
	if ( isRunningFlag.load() )
	{
		return -1;
	}

	int returnValue = 0;

	pthread_attr_t attribute;

	pthread_attr_init( &attribute );

	pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_JOINABLE );

	returnValue |= pthread_create( thread, &attribute, startRoutine, objectPointer );

	pthread_attr_destroy( &attribute );

	if ( returnValue )
	{
		perror( "startJoinableThread: Failed to create the thread" );
		isRunningFlag = false;
		return -1;
	}
	else
	{
		isRunningFlag = true;
		return 0;
	}
}

} // namespace Common
