#ifndef BADMUTEXC_H
#define BADMUTEXC_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif /*_WIN32*/

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void lock( void );
EXPORT bool try_lock( void );
EXPORT void unlock( void );
EXPORT unsigned int version( void );

#ifdef __cplusplus
}
#endif

#endif /*BADMUTEXC_H*/
