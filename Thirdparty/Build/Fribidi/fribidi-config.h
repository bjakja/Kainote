/* Hand-maintained stand-in for the fribidi-config.h that fribidi's own
   configure/meson run would generate. Kept in sync with the fribidi version
   pinned in Thirdparty/dependencies.json.

   Values mirror lib/fribidi-config.h.in from fribidi 1.0.16, with
   FRIBIDI_INTERFACE_VERSION taken from meson.build (interface_version = 4).

   FRIBIDI_BUILT_WITH_MSVC stays undefined and Fribidi.vcxproj defines
   FRIBIDI_ENTRY to nothing, so the library builds statically with no
   dllimport/dllexport decoration. */
#ifndef FRIBIDI_CONFIG_H
#define FRIBIDI_CONFIG_H

#define FRIBIDI "fribidi"
#define FRIBIDI_NAME "GNU FriBidi"
#define FRIBIDI_BUGREPORT "https://github.com/fribidi/fribidi/issues/new"

#define FRIBIDI_VERSION "1.0.16"
#define FRIBIDI_MAJOR_VERSION 1
#define FRIBIDI_MINOR_VERSION 0
#define FRIBIDI_MICRO_VERSION 16
#define FRIBIDI_INTERFACE_VERSION 4
#define FRIBIDI_INTERFACE_VERSION_STRING "4"

/* The size of a `int', as computed by sizeof. */
#define FRIBIDI_SIZEOF_INT 4

/* Define if fribidi was built with MSVC */
#undef FRIBIDI_BUILT_WITH_MSVC

/* lib/common.h expects these from the autoconf-generated config.h, which this
   build does not produce. They are declared here rather than in a config.h next
   to this file on purpose: Libass.vcxproj puts Thirdparty\build\Fribidi on its
   include path ahead of Thirdparty\Build\libass, so a config.h here would shadow
   libass's own config.h and silently turn off CONFIG_ASM and friends.

   fribidi-config.h is reached early enough for these to take effect, because
   common.h includes fribidi-common.h (which includes this file) before it tests
   any of them. Without HAVE_STRINGIZE, common.h is a hard #error; without the
   other two, malloc/free/memcpy are used without a declaration. */
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define STDC_HEADERS 1
#define HAVE_STRINGIZE 1

#endif /* FRIBIDI_CONFIG_H */
