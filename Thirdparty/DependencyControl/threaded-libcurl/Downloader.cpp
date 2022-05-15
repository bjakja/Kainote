#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "DebugLog.h"
#include "Downloader.hpp"

static size_t curlWriteCallback( char *buffer, size_t size, size_t nitems, void *userdata ) {
	DEBUG_LOG( "Write callback." );
	return static_cast<Downloader*>(userdata)->writeCallback( buffer, size*nitems );
}

static int curlProgressCallback( void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow ) {
	return static_cast<Downloader*>(userdata)->progressCallback( dltotal, dlnow );
}

static size_t curlHeaderCallback( char *buffer, size_t size, size_t nitems, void *userdata ) {
	return static_cast<Downloader*>(userdata)->headerCallback( buffer, size*nitems );
}

std::string digestToHex( uint8_t digest[SHA1_DIGEST_SIZE] ) {
	char hash[41];
	for( unsigned int offset = 0; offset < SHA1_DIGEST_SIZE; offset++ ) {
		snprintf( hash + 2*offset, 3, "%02x", digest[offset] );
	}
	DEBUG_LOG( "Calculated download sha1sum: " << std::string(hash) );
	return std::string(hash);
}

Downloader::Downloader( const char *url, const char *outputFile, const char *expectedHash, const char *expectedETag )
// url and outputFile are checked to be nonnull by DownloadManager.
: url(url), outputFile(outputFile) {
	if ( expectedHash != nullptr ) {
		this->expectedHash = std::string( expectedHash );
		hasExpectedHash = true;
		SHA1_Init( &sha1ctx );
	}
	if ( expectedETag != nullptr ) {
		this->expectedETag = std::string( expectedETag );
		hasExpectedETag = true;
	}

	thread = std::thread( &Downloader::process, this );
}

int Downloader::progressCallback( curl_off_t dltotal, curl_off_t dlnow ) {
	current = dlnow;
	total = dltotal;
	return wasTerminated? !CURLE_OK: CURLE_OK;
}

size_t Downloader::writeCallback( const char *buffer, size_t size ) {
	outputBuffer.append( buffer, size );
	if (hasExpectedHash) {
		// This seems to calculate the sha1 correctly but buffer gets
		// corrupted somehow. Is it the cast? Is it the function?
		// SHA1_Update just memcpy's from the buffer, so it shouldn't be to
		// blame. Easiest fix is to append before hashing.
		SHA1_Update( &sha1ctx, reinterpret_cast<const uint8_t*>(buffer), size );
	}
	return size;
}

size_t Downloader::headerCallback( const char *buffer, size_t size ) {
	// This is currently the only condition where we treat the file as
	// cached and proceed. Perhaps we should also manually compare etags?
	if (strncmp( buffer, "HTTP/1.1 304 Not Modified", 25 ) == 0) {
		DEBUG_LOG( "304 not modified got." );
		isCachedFile = true;
	}
	if (strncmp( buffer, "ETag: ", 6 ) == 0) {
		DEBUG_LOG( "An etag header was found" );
		// cut off an extra three chars because buffer is CRLF terminated
		// and the ETag is in quotation marks.
		actualETag.append( buffer + 7, size - 10 );
		DEBUG_LOG( "ETag: " << actualETag );
	}
	return size;
}

void Downloader::finalize( void ) {
	DEBUG_LOG( "Finalize." );
	if (wasTerminated) {
		DEBUG_LOG( "Download was terminated." );
		errorMessage = "Download was terminated.";
		hasFailed = true;
		return;
	}

	if (isCachedFile)
		return;

	if (hasExpectedHash) {
		uint8_t digest[SHA1_DIGEST_SIZE];
		SHA1_Final( &sha1ctx, digest );
		DEBUG_LOG( "Finalizer sha1sum: " << expectedHash );
		auto result = digestToHex( digest );
		if ( result != expectedHash ) {
			errorMessage = "Hash mismatch. Got " + result + ", expected " + expectedHash;
			hasFailed = true;
			return;
		}
	}
	DEBUG_LOG( "Download not cached. Writing file." );
	std::fstream outStream( outputFile, std::ios::out | std::ios::binary );
	if (outStream.fail( )) {
		errorMessage = "Couldn't open output file: " + outputFile;
		hasFailed = true;
		return;
	}

	outStream << outputBuffer;
	outStream.close( );
	DEBUG_LOG( "We're done here." );
}

void Downloader::process( void ) {
	DEBUG_LOG( "Thread has been started." );
	char curlError[CURL_ERROR_SIZE];
	CURL *curl = curl_easy_init( );
	struct curl_slist *slist = NULL;
	if ( curl == nullptr ) {
		errorMessage = "Could not initialize curl.";
		hasFailed = true;
		goto exit;
	}

	if (hasExpectedETag) {
		char header[256];
		snprintf( header, 256, "If-None-Match: \"%s\"", expectedETag.c_str( ) );
		slist = curl_slist_append( slist, header );
		if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_HTTPHEADER, slist )) {
			errorMessage = "Could not set http headers.";
			goto fail;
		}
		if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_HEADERDATA, this )) {
			errorMessage = "Could not set header callback userdata.";
			goto fail;
		}
		if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback ) ) {
			errorMessage = "Could not set header callback function.";
			goto fail;
		}
	}

	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_WRITEDATA, this )) {
		errorMessage = "Could not set write callback userdata.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlWriteCallback )) {
		errorMessage = "Could not set write callback function.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 0 )) {
		errorMessage = "Could not enable progress callback????";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_XFERINFODATA, this )) {
		errorMessage = "Could not set progress callback userdata.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback )) {
		errorMessage = "Could not set progress callback function.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_FAILONERROR, 1 ) ) {
		errorMessage = "Could not fail on error.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_URL, url.c_str( ) )) {
		errorMessage = "Could not set fetch url.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 ) ) {
		errorMessage = "Could not set redirect following.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_ERRORBUFFER, curlError )) {
		errorMessage = "Could not set error buffer.";
		goto fail;
	}
	if (CURLE_OK != curl_easy_setopt( curl, CURLOPT_ACCEPT_ENCODING, "" )) {
		errorMessage = "Could not set Accept-Encoding header.";
		goto fail;
	}

	DEBUG_LOG( "Running curl_easy_perform." );
	switch (curl_easy_perform( curl )) {
	case CURLE_OK:
		break;

	case CURLE_ABORTED_BY_CALLBACK:
		errorMessage = "User aborted.";
		goto fail;

	case CURLE_WRITE_ERROR:
		errorMessage = "A write error occurred.";
		goto fail;

	default:
		errorMessage = std::string( curlError );
		goto fail;
	}

	finalize( );
	goto cleanup;

fail:
	hasFailed = true;
cleanup:
	curl_slist_free_all( slist );
	curl_easy_cleanup( curl );
exit:
	isFinished = true;
	return;
}

bool Downloader::assimilate( void ) {
	if ((!isFinished && !wasTerminated) || threadHasJoined) {
		return false;
	}
	thread.join( );
	threadHasJoined = true;
	return true;
}
