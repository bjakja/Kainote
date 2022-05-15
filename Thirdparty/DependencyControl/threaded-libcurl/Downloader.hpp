#pragma once
#include <string>
#include <thread>
#include <curl/curl.h>

#include "sha1.h"

std::string digestToHex( uint8_t digest[SHA1_DIGEST_SIZE] );

class Downloader {
	std::thread thread;
	std::string url,
	            outputFile,
	            expectedETag,
	            expectedHash,
	            outputBuffer;
	SHA1_CTX sha1ctx;
	// because std::string isn't an optional type, we'll manually track
	// whether or not the optional constructor arguments exist.
	bool hasExpectedHash = false,
	     hasExpectedETag = false,
	     threadHasJoined = false;

	void finalize( void );

	public:
		bool wasTerminated = false,
	       isCachedFile  = false,
		     isFinished    = false,
		     hasFailed     = false;
		std::string actualETag,
		            errorMessage;
		curl_off_t current = 0, total = 0;

		// const char pointers are essentially the builtin nullable type so
		// why bother using some ugly optional wrapping std::string?
		Downloader( const char *url, const char *outputFile, const char *expectedHash, const char *expectedETag );
		int progressCallback( curl_off_t dltotal, curl_off_t dlnow );
		size_t writeCallback( const char *buffer, size_t size );
		size_t headerCallback( const char *buffer, size_t size );
		void process( void );
		bool assimilate( void );
};
