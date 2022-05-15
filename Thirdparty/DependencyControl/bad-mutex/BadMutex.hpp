#pragma once
#include <memory>
#include <mutex>

class BadMutex {
	public:
		const static unsigned int version = 0x000100;
		static BadMutex& getInstance( void );
		void lock( void );
		void unlock( void );
		bool try_lock( void );
	private:
		static std::once_flag cFlag;
		static std::unique_ptr<BadMutex> instance;
		std::mutex mutex;
		BadMutex( void );
};
