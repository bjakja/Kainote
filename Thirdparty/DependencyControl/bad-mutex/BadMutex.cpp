#include "BadMutex.hpp"

std::once_flag BadMutex::cFlag;
std::unique_ptr<BadMutex> BadMutex::instance;

BadMutex::BadMutex( void ) : mutex() {}

BadMutex& BadMutex::getInstance( void ) {
	std::call_once( cFlag, [] {
		instance.reset(new BadMutex);
	});
	return *instance.get( );
}

void BadMutex::lock( void ) {
	mutex.lock( );
}

bool BadMutex::try_lock( void ) {
	return mutex.try_lock( );
}

void BadMutex::unlock( void ) {
	mutex.unlock( );
}
