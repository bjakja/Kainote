#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <curl/curl.h>
#include "DownloadManager.hpp"
#include "sha1.h"

#if defined(_WIN32)
#include <wininet.h>

#elif defined(__APPLE__)
#include <netinet/in.h>
#include <SystemConfiguration/SystemConfiguration.h>

#endif

DownloadManager::DownloadManager( void ) {
	curl_global_init( CURL_GLOBAL_ALL );
}

DownloadManager::~DownloadManager( void ) {
	clear( );
	curl_global_cleanup( );
}

double DownloadManager::getProgress( void ) {
	double progress = finishedCount;
	for (const auto &downloader : downloaders) {
		if (downloader->total > 0 && !downloader->isFinished) {
			progress += downloader->current/(double)downloader->total;
		}
	}
	return (addedCount > 0)? progress/addedCount: 0;
}

unsigned int DownloadManager::addDownload( const char *url, const char *outputFile, const char *expectedHash, const char *expectedETag ) {
	if ( url == nullptr || outputFile == nullptr )
		return 0;

	downloaders.push_back( new Downloader( url, outputFile, expectedHash, expectedETag ) );
	return ++addedCount;
}

void DownloadManager::terminate( void ) {
	for (auto &downloader : downloaders) {
		if (!downloader->isFinished)
			downloader->wasTerminated = true;
	}
}

void DownloadManager::clear( void ) {
	terminate( );
	for (auto &downloader : downloaders) {
		downloader->assimilate( );
		delete downloader;
	}
	downloaders.clear( );
	addedCount    = 0;
	finishedCount = 0;
	failedCount   = 0;
}

int DownloadManager::checkDownload( unsigned int i ) {
	if (i > addedCount) {
		return -1;
	}
	return downloaders[i-1]->isFinished;
}

const char* DownloadManager::getError( unsigned int i ) {
	if (i > addedCount)
		return "Not a download.";

	auto downloader = downloaders[i-1];
	if (downloader->hasFailed)
		return downloader->errorMessage.c_str( );

	return nullptr;
}

bool DownloadManager::fileWasCached( unsigned int i ) {
	if (i > addedCount)
		return false;
	auto downloader = downloaders[i-1];
	return downloader->isCachedFile;
}

const char* DownloadManager::getETag( unsigned int i ) {
	if (i > addedCount)
		return nullptr;

	auto downloader = downloaders[i-1];
	return downloader->actualETag.c_str( );
}

int DownloadManager::busy( void ) {
	for (const auto &downloader : downloaders) {
		if (downloader->assimilate( )) {
			if (downloader->hasFailed) {
				failedCount++;
			}
			finishedCount++;
		}
	}
	return addedCount - finishedCount;
}

// These are kind of out of place but they are useful. This function
// should probably not be used on multi-gigabyte files, since it tries
// to dump the entire file into an array at once. Perhaps a filesize
// check for switching behavior should be added.
std::string DownloadManager::getFileSHA1( const std::string &filename ) {
	std::string buffer;
	std::fstream inFile( filename, std::ios::in | std::ios::binary | std::ios::ate );
	if (inFile.is_open( )) {
		buffer.resize( inFile.tellg( ) );
		inFile.seekg( 0, std::ios::beg );
		inFile.read( &buffer[0], buffer.size( ) );
		inFile.close( );
		return getStringSHA1( buffer );
	}
	return "";
}

std::string DownloadManager::getStringSHA1( const std::string &string ) {
	SHA1_CTX ctx;
	uint8_t digest[SHA1_DIGEST_SIZE];
	SHA1_Init( &ctx );
	SHA1_Update( &ctx, reinterpret_cast<const uint8_t*>(string.c_str( )), string.size( ) );
	SHA1_Final( &ctx, digest );
	return digestToHex( digest );
}

bool DownloadManager::isInternetConnected() {
#if defined(_WIN32)
	DWORD flags = 0; // don't care about those
	return !!InternetGetConnectedState( &flags, 0 );

#elif defined(__APPLE__)
	struct sockaddr address = {};
	address.sa_len = sizeof(address);
	address.sa_family = AF_INET;

	SCNetworkConnectionFlags flags = 0;
	SCNetworkReachabilityRef target = SCNetworkReachabilityCreateWithAddress( kCFAllocatorDefault, &address );
	if ( target == nullptr )
		return false;

	bool flagsValid = SCNetworkReachabilityGetFlags( target, &flags );
	CFRelease(target);
	return flagsValid && (flags & kSCNetworkReachabilityFlagsReachable) && !(flags & kSCNetworkReachabilityFlagsConnectionRequired);

#else // todo: add linux code?
	return true;

#endif
}

/*
#include <unistd.h> // usleep

int main( int argc, char **argv ) {
	DownloadManager manager;
	unsigned int count = 0;
	count = manager.addDownload( "https://a.real.website", "out1", "b52854d1f79de5ebeebf0160447a09c7a8c2cde4" );
	count = manager.addDownload( "https://a.real.website", "out2", "this isn't a real sha1" );
	count = manager.addDownload( "https://a.real.website", "out3" );
	while (manager.busy( ) > 0) {
		std::cout << "Progress: " << manager.getProgress( ) << std::endl;
		usleep( 10000 );
	}

	for( int i = 1; i < count+1; ++i ) {
		const char *str = manager.getError( i );
		if (str) {
			std::cout << "Download " << i << " error: " << str << std::endl;
		}
	}

	return 0;
}
*/
