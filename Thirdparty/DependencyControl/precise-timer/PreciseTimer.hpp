#pragma once
#include <chrono>
#include <type_traits>

typedef std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type UsefulClock;

class PreciseTimer {
	protected:
		UsefulClock::time_point startTime;
	public:
		static constexpr unsigned int version = 0x000101;

		PreciseTimer( );
		double getElapsedTime( );
};
