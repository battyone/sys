#include "sys.config.h"
#include "sys.dir.h"

#include <cstring>
#include <cstdlib>

#if defined(SYS_WIN32)
static ::DIR* opendir(const char* path)
{
	char filespec[MAX_PATH];
	std::strcpy(filespec, path);
	std::strcat(filespec, "\\");

	::DIR* entry = static_cast<::DIR *>(std::malloc(sizeof(::DIR)));
	if (entry != nullptr)
	{
		entry->firsttime = TRUE;
		entry->search = ::FindFirstFileA(filespec, &entry->win32_find_data);

		if (entry->search == INVALID_HANDLE_VALUE)
		{
			std::strcat(filespec, "\\*.*");
			entry->search = ::FindFirstFileA(filespec, &entry->win32_find_data);
			if (entry->search == INVALID_HANDLE_VALUE)
				entry = (std::free(entry), nullptr);
		}
	}

	return entry;
}

static struct ::dirent* readdir(::DIR* entry)
{
	if (entry == nullptr)
		return nullptr;

	if (!entry->firsttime)
	{
		int status = ::FindNextFileA(entry->search, &entry->win32_find_data);
		if (status == 0)
			return nullptr;
	}

	entry->firsttime = FALSE;
	std::strcpy(entry->fileinfo.d_name, entry->win32_find_data.cFileName);

	return &entry->fileinfo;
}

static void closedir(::DIR* entry)
{
	::FindClose(entry->search);
	std::free(entry);
}
#endif

sys::dir::dir(const char* name)
	: dirp_(::opendir(name))
{
}

sys::dir::~dir(void)
{
	if (dirp_)
		::closedir(dirp_);
}

bool sys::dir::advance(std::string& dname)
{
	const struct ::dirent* entp = ::readdir(dirp_);
	if (entp == nullptr)
		return false;
	if (!std::strcmp(entp->d_name, ".") || !std::strcmp(entp->d_name, ".."))
		return advance(dname);
	dname = entp->d_name;
	return true;
}