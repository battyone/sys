#ifndef __SYS_DIR__
#define __SYS_DIR__

#include "sys.config.h"

#include <string>

#if defined(SYS_WIN32)
struct dirent
{
	char d_name[MAX_PATH];
};

typedef struct
{
	HANDLE search;
	WIN32_FIND_DATAA win32_find_data;
	BOOL firsttime;
	struct dirent fileinfo;
} DIR;
#endif

namespace sys
{
	class dir
	{
		::DIR* dirp_;
	public:
		dir(const char* name);
		virtual ~dir(void);
	public:
		bool advance(std::string& dname);
	};
}

#endif

