#ifndef __SYS_CONFIG__
#define __SYS_CONFIG__

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)
#define SYS_WIN_NATIVE
#endif

#if defined(__CYGWIN__)
#define SYS_CYGWIN
#endif

#if defined(SYS_WIN_NATIVE) || defined(SYS_CYGWIN)
#define SYS_WIN32
#endif

#if !defined(SYS_WIN_NATIVE)
#define SYS_POSIX
#endif

#if defined(_MSC_VER)
#define SYS_MSVC
#if !defined(_DEBUG) && !defined(__SYS_INLINE__)
#define __SYS_INLINE__
#endif
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(__GNUC__)
#define SYS_GCC
#if __GNUC__ < 4
#define SYS_GCC3
#endif
#if !defined (__SYS_INLINE__)
#define __SYS_INLINE__
#endif
#endif

#if defined(__linux)
#define SYS_HAVE_PROC_SELF_EXE
#elif defined(__sun)
#define SYS_HAVE_PROC_SELF_PATH_AOUT
#undef SYS_HAVE_GETEXECNAME
#elif defined(__hpux)
#define SYS_HAVE_DLGETNAME
#undef SYS_HAVE_PSTAT_GETPATHNAME
#elif defined(__FreeBSD__)
#define SYS_HAVE_KERN_PROC_PATHNAME
#undef SYS_HAVE_PROC_CURPROC_FILE
#elif defined(__NetBSD__)
#define SYS_HAVE_PROC_CURPROC_EXE
#elif defined(__DragonFly__)
#define SYS_HAVE_PROC_CURPROC_FILE
#elif defined(__OpenBSD__)
#define SYS_HAVE_KERN_PROC_ARGV
#elif defined(__APPLE__)
#define SYS_HAVE_NSGETEXECUTABLEPATH
#undef SYS_HAVE_PROC_PIDPATH
#elif defined(__QNXNTO__)
#define SYS_HAVE_PROC_SELF_EXEFILE
#endif

#if defined(SYS_LACKS_INLINE_FUNCTIONS) && !defined(SYS_NO_INLINE)
#define SYS_NO_INLINE
#endif

#if defined(SYS_NO_INLINE)
#undef __SYS_INLINE__
#endif

#if defined(__SYS_INLINE__)
#define SYS_INLINE inline
#else
#define SYS_INLINE
#endif

#if defined(SYS_HAVE_GETEXECNAME)
#include <stdlib.h>
#endif

#if defined(SYS_HAVE_NSGETEXECUTABLEPATH)
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#if defined(SYS_HAVE_PROC_PIDPATH)
#include <libproc.h>
#endif

#if defined(SYS_HAVE_DLGETNAME)
#include <dlfcn.h> 
#endif

#if defined(SYS_HAVE_PSTAT_GETPATHNAME)
#include <sys/pstat.h>
#include <limits.h>
#endif

#if defined(SYS_WIN32)
#undef SYS_WIN32
#include <windows.h>
#define SYS_WIN32
#if defined(SYS_CYGWIN)
#include <sys/cygwin.h>
#endif
#else
#if !defined(SYS_LACKS_SYS_TYPES_H)
#include <sys/types.h>
#endif
#if !defined(SYS_LACKS_SYS_STAT_H)
#include <sys/stat.h>
#endif
#if !defined(SYS_LACKS_SYS_SYSCTL_H)
#include <sys/sysctl.h>
#endif
#if !defined(SYS_LACKS_UNISTD_H)
#include <unistd.h>
#endif
#endif

#endif

