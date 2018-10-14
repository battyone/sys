#include "sys.config.h"
#include "sys.path.h"

#include <vector>
#include <locale>
#include <codecvt>
#include <cerrno>

#if defined(SYS_WIN32)

#if !defined(REPARSE_DATA_BUFFER_HEADER_SIZE)
#define SYMLINK_FLAG_RELATIVE 1
typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT  ReparseDataLength;
	USHORT  Reserved;
	union {
		struct {
			USHORT  SubstituteNameOffset;
			USHORT  SubstituteNameLength;
			USHORT  PrintNameOffset;
			USHORT  PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT  SubstituteNameOffset;
			USHORT  SubstituteNameLength;
			USHORT  PrintNameOffset;
			USHORT  PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR  DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE \
  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#endif

#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE  ( 16 * 1024 )
#endif

static bool not_found_error(int errval)
{
	return errval == ERROR_FILE_NOT_FOUND
		|| errval == ERROR_PATH_NOT_FOUND
		|| errval == ERROR_INVALID_NAME
		|| errval == ERROR_INVALID_DRIVE
		|| errval == ERROR_NOT_READY
		|| errval == ERROR_INVALID_PARAMETER
		|| errval == ERROR_BAD_PATHNAME
		|| errval == ERROR_BAD_NETPATH;
}

static bool is_reparse_point_a_symlink(const sys::path& p)
{
	HANDLE handle = ::CreateFileA(p.c_str(), FILE_READ_EA,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(handle);
		return false;
	}

	std::vector<char> buf(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	DWORD dwRetLen;
	BOOL result = ::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr,
		0, buf.data(), MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwRetLen, nullptr);
	if (!result)
	{
		::CloseHandle(handle);
		return false;
	}

	return reinterpret_cast<const REPARSE_DATA_BUFFER*>
			(buf.data())->ReparseTag == IO_REPARSE_TAG_SYMLINK ||
		 reinterpret_cast<const REPARSE_DATA_BUFFER*>
			(buf.data())->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT;
}
#endif

sys::path sys::path::read_symlink(const sys::path& p, bool& err)
{
	path symlink_path;
#if defined(SYS_WIN32)
	union info_t
	{
		char buf[REPARSE_DATA_BUFFER_HEADER_SIZE + MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
		REPARSE_DATA_BUFFER rdb;
	} info;

	HANDLE handle = ::CreateFileA(p.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(handle);
		err = true;
		return symlink_path;
	}

	DWORD dwRetLen;
	BOOL result = ::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr,
		0, info.buf, sizeof(info), &dwRetLen, nullptr);
	if (!result)
	{
		::CloseHandle(handle);
		err = true;
		return symlink_path;
	}
	err = false;
	std::wstring pathname(
		static_cast<wchar_t*>(info.rdb.SymbolicLinkReparseBuffer.PathBuffer)
		+ info.rdb.SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t),
		static_cast<wchar_t*>(info.rdb.SymbolicLinkReparseBuffer.PathBuffer)
		+ info.rdb.SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t)
		+ info.rdb.SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t));
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	symlink_path.assign(converter.to_bytes(pathname));
#else
	for (long size = 128;; size *= 2)
	{
		std::vector<char> buf(size);
		long result = ::readlink(p.c_str(), buf.data(),
			static_cast<std::size_t>(size));
		if (result == -1)
		{
			err = true;
			break;
		}
		if (result != size)
		{
			err = false;
			std::string pathname(buf.data(), buf.data() + result);
			symlink_path.assign(pathname);
			break;
		}
	}
#endif
	return symlink_path;
}

bool sys::path::is_symlink(const sys::file_type_t& f)
{
	return f == sys::symlink_file;
}

sys::file_type_t sys::path::symlink_status(const sys::path& p, bool& err)
{
#if defined(SYS_WIN32)
	DWORD attr(::GetFileAttributesA(p.c_str()));
	if (attr == 0xFFFFFFFF)
	{
		int errval(::GetLastError());
		if (not_found_error(errval))
			return sys::file_not_found;
		else if (errval == ERROR_SHARING_VIOLATION)
			return sys::type_unknown;
		err = true;
		return sys::status_error;
	}
	err = false;
	if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
		return is_reparse_point_a_symlink(p) ?
			sys::symlink_file : sys::reparse_file;
	return attr & FILE_ATTRIBUTE_DIRECTORY ?
		sys::directory_file : sys::regular_file;
#else
	struct stat path_stat;
	if (::lstat(p.c_str(), &path_stat) != 0)
	{
		if (errno == ENOENT || errno == ENOTDIR)
			return sys::file_not_found;
		err = true;
		return sys::status_error;
	}
	err = false;
	if (S_ISREG(path_stat.st_mode))
		return sys::regular_file;
	if (S_ISDIR(path_stat.st_mode))
		return sys::directory_file;
	if (S_ISLNK(path_stat.st_mode))
		return sys::symlink_file;
	if (S_ISBLK(path_stat.st_mode))
		return sys::block_file;
	if (S_ISCHR(path_stat.st_mode))
		return sys::character_file;
	if (S_ISFIFO(path_stat.st_mode))
		return sys::fifo_file;
	if (S_ISSOCK(path_stat.st_mode))
		return sys::socket_file;
	return sys::type_unknown;
#endif
}