#ifndef DOWNLOADMANAGERC_H
#define DOWNLOADMANAGERC_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif /*_WIN32*/

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

struct CDlM;
typedef struct CDlM CDlM;
typedef unsigned int uint;

EXPORT CDlM*       CDlM_new           ( void );
EXPORT uint        CDlM_addDownload   ( CDlM *mgr,              const char *url,
                                   const char *outputFile, const char *expectedHash,
                                   const char *expectedETag );
EXPORT double      CDlM_progress      ( CDlM *mgr );
EXPORT int         CDlM_busy          ( CDlM *mgr );
EXPORT int         CDlM_checkDownload ( CDlM *mgr, uint i );
EXPORT const char* CDlM_getError      ( CDlM *mgr, uint i );
EXPORT bool        CDlM_fileWasCached ( CDlM *mgr, uint i );
EXPORT const char* CDlM_getETag       ( CDlM *mgr, uint i );
EXPORT void        CDlM_terminate     ( CDlM *mgr );
EXPORT void        CDlM_clear         ( CDlM *mgr );
EXPORT const char* CDlM_getFileSHA1   ( const char *filename );
EXPORT const char* CDlM_getStringSHA1 ( const char *string );
EXPORT uint        CDlM_version       ( void );
EXPORT void        CDlM_freeDM        ( CDlM *mgr );
EXPORT bool        CDlM_isInternetConnected( void );
#ifdef __cplusplus
}
#endif

#endif /*DOWNLOADMANAGERC_H*/
