/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef THREADHELPER_H
#define THREADHELPER_H

#include <atomic>
#include <pthread.h>

namespace Common {

/**
 * @brief The ThreadHelper class wraps the attributes
 *		and other arguements necessary to create a pthread.
 */
class ThreadHelper
{
public:

	/**
	 * Typedef of the prototype necessary for pthread functions.
	 */
	typedef void* ( *StartRoutine )( void* );

	/**
	 * @brief startDetachedThread
	 * @param thread
	 * @param startRoutine
	 * @param isRunningFlag
	 * @param objectPointer
	 * @return
	 */
	static int startDetachedThread( pthread_t* thread, StartRoutine startRoutine, bool* isRunningFlag, void* objectPointer );

	/**
	 * @brief startDetachedThread
	 * @param thread
	 * @param startRoutine
	 * @param isRunningFlag
	 * @param objectPointer
	 * @return
	 */
	static int startDetachedThread( pthread_t* thread, StartRoutine startRoutine, std::atomic_bool& isRunningFlag, void* objectPointer );

	/**
	 * @brief startJoinableThread
	 * @param thread
	 * @param startRoutine
	 * @param isRunningFlag
	 * @param objectPointer
	 * @return
	 */
	static int startJoinableThread( pthread_t* thread, StartRoutine startRoutine, bool* isRunningFlag, void* objectPointer );

	/**
	 * @brief startJoinableThread
	 * @param thread
	 * @param startRoutine
	 * @param objectPointer
	 * @return
	 */
	static int startJoinableThread( pthread_t* thread, StartRoutine startRoutine, void* objectPointer );

	/**
	 * @brief startJoinableThread
	 * @param thread
	 * @param startRoutine
	 * @param isRunningFlag
	 * @param objectPointer
	 * @return
	 */
	static int startJoinableThread( pthread_t* thread, StartRoutine startRoutine, std::atomic_bool& isRunningFlag, void* objectPointer );

private:

	/**
	 * @brief	ThreadHelper private constructor, so an instance of this class cannot be
	 *			created because all methods are declared static.
	 *
	 * @note	This method is not implemented intentionally.
	 */
	ThreadHelper();

	/**
	 * @brief	ThreadHelper private copy constructor, so the compiler does not auto-generate
	 *			one.
	 * @param factory
	 *
	 * @note	This method is not implemented intentionally.
	 */
	ThreadHelper( const ThreadHelper& threadHelper );

	/**
	 * @brief operator = private equal overload, so the compiler does not auto-generate
	 *			one.
	 * @param threadHelper
	 * @return
	 *
	 * @note	This method is not implemented intentionally.
	 */
	ThreadHelper& operator=( const ThreadHelper& threadHelper );
};

} // namespace Common

#endif // THREADHELPER_H
