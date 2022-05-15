#ifndef PRECISETIMERC_H
#define PRECISETIMERC_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif /*_WIN32*/

#ifdef __cplusplus
extern "C" {
#endif

struct CPT;
typedef struct CPT CPT;

EXPORT CPT* startTimer( void );
EXPORT double getDuration( CPT *pt );
EXPORT unsigned int version( void );
EXPORT void freeTimer( CPT *pt );

#ifdef __cplusplus
}
#endif

#endif /*PRECISETIMERC_H*/
