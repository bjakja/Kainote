#include "PreciseTimer.hpp"
#include "PreciseTimerC.h"
extern "C" {
	EXPORT CPT* startTimer( void ) {
		return reinterpret_cast<CPT*>(new PreciseTimer);
	}

	EXPORT double getDuration( CPT *pt ) {
		return reinterpret_cast<PreciseTimer*>(pt)->getElapsedTime( );
	}

	EXPORT unsigned int version( void ) {
		return PreciseTimer::version;
	}

	EXPORT void freeTimer( CPT *pt ) {
		delete reinterpret_cast<PreciseTimer*>(pt);
	}
}
