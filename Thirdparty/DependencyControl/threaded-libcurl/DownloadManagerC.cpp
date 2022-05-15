#include <string>
#include "DownloadManager.hpp"
#include "DownloadManagerC.h"

// Avoid symbol mangling.
extern "C" {
	EXPORT CDlM *CDlM_new( void ) {
		return reinterpret_cast<CDlM*>(new DownloadManager);
	}

	EXPORT uint CDlM_addDownload( CDlM *mgr, const char *url, const char *outputFile, const char *expectedHash, const char *expectedETag ) {
		if (mgr == nullptr)
			return 0;
		return reinterpret_cast<DownloadManager*>(mgr)->addDownload( url, outputFile, expectedHash, expectedETag );
	}

	EXPORT double CDlM_progress( CDlM *mgr ) {
		if (mgr == nullptr)
			return 0;
		return reinterpret_cast<DownloadManager*>(mgr)->getProgress( );
	}

	EXPORT int CDlM_busy( CDlM *mgr ) {
		if (mgr == nullptr)
			return 0;
		return reinterpret_cast<DownloadManager*>(mgr)->busy( );
	}

	EXPORT int CDlM_checkDownload( CDlM *mgr, uint i ) {
		if (mgr == nullptr)
			return -1;
		return reinterpret_cast<DownloadManager*>(mgr)->checkDownload( i );
	}

	EXPORT const char* CDlM_getError( CDlM *mgr, uint i ) {
		if (mgr == nullptr)
			return nullptr;
		return reinterpret_cast<DownloadManager*>(mgr)->getError( i );
	}

	EXPORT bool CDlM_fileWasCached( CDlM *mgr, uint i ) {
		if (mgr == nullptr)
			return false;
		return reinterpret_cast<DownloadManager*>(mgr)->fileWasCached( i );
	}

	EXPORT const char* CDlM_getETag( CDlM *mgr, uint i ) {
		if (mgr == nullptr)
			return nullptr;
		auto result = reinterpret_cast<DownloadManager*>(mgr)->getETag( i );
		if ( result == nullptr || result[0] == '\0' )
			return nullptr;
		return result;
	}

	EXPORT void CDlM_terminate( CDlM *mgr ) {
		if (mgr == nullptr)
			return;
		reinterpret_cast<DownloadManager*>(mgr)->terminate( );
	}

	EXPORT void CDlM_clear( CDlM *mgr ) {
		if (mgr == nullptr)
			return;
		reinterpret_cast<DownloadManager*>(mgr)->clear( );
	}

	EXPORT const char* CDlM_getFileSHA1( const char *filename ) {
		if (filename == nullptr)
			return nullptr;

		auto result = DownloadManager::getFileSHA1( std::string( filename ) );
		if (result == "")
			return nullptr;

		return result.c_str( );
	}

	EXPORT const char* CDlM_getStringSHA1( const char *string ) {
		if (string == nullptr)
			return nullptr;

		auto result = DownloadManager::getStringSHA1( std::string( string ) );
		if (result == "")
			return nullptr;

		return result.c_str( );
	}

	EXPORT uint CDlM_version( void ) {
		return DownloadManager::version;
	}

	EXPORT void CDlM_freeDM( CDlM* mgr ) {
		if (mgr == nullptr)
			return;

		delete reinterpret_cast<DownloadManager*>(mgr);
	}

	EXPORT bool CDlM_isInternetConnected() {
		return DownloadManager::isInternetConnected();
	}
}

/*
#include <cstdio>
#include <unistd.h> // usleep

int main( int argc, char **argv ) {
	CDlM *manager = newDM( );
	unsigned int count = 0;
	count = addDownload( manager, "https://a.real.website", "out1", "b52854d1f79de5ebeebf0160447a09c7a8c2cde4", NULL );
	count = addDownload( manager, "https://a.real.website", "out2", "this isn't a real sha1", NULL );
	count = addDownload( manager, "https://a.real.website", "out3", NULL, NULL );
	while (busy( manager ) > 0) {
		printf( "Progress: %g\n", progress( manager ) );
		usleep( 10000 );
	}

	for( int i = 1; i < count+1; ++i ) {
		const char *str = getError( manager, i );
		if (str) {
			printf( "Download %d error: %s\n", i, str );
		}
	}

	return 0;
}
*/
