#include "BadMutex.hpp"
#include "BadMutexC.h"

extern "C" {
	EXPORT void lock( void ) {
		BadMutex::getInstance().lock();
	}

	EXPORT bool try_lock( void ) {
		return BadMutex::getInstance().try_lock();
	}

	EXPORT void unlock( void ) {
		BadMutex::getInstance().unlock();
	}

	EXPORT unsigned int version( void ) {
		return BadMutex::version;
	}
}
