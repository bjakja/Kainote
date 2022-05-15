#include "PreciseTimer.hpp"

PreciseTimer::PreciseTimer( void ) : startTime(UsefulClock::now( )) { }

double PreciseTimer::getElapsedTime( void ) {
	return std::chrono::duration<double>(UsefulClock::now( ) - this->startTime).count( );
}
