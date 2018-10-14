#include "sys.config.h"
#include "sys.path.h"

#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>

#if defined(SYS_WIN32)
const char sys::path::separator = '/';
const char sys::path::preferred_separator = '\\';
const char* const sys::path::separators = "/\\";
const char* sys::path::separator_string = "/";
const char* sys::path::preferred_separator_string = "\\";
const char sys::path::path_separator = ';';
#else
const char sys::path::separator = '/';
const char sys::path::preferred_separator = '/';
const char* const sys::path::separators = "/";
const char* sys::path::separator_string = "/";
const char* sys::path::preferred_separator_string = "/";
const char sys::path::path_separator = ':';
#endif

const sys::path sys::path::initial_path(current_path());

sys::path::path(void)
{
}

sys::path::path(const char* pathname)
	: pathname_(pathname)
{
}

sys::path::path(const std::string& pathname)
	: pathname_(pathname)
{
}

sys::path::path(const char* first, const char* last)
	: pathname_(first, last)
{
}

sys::path::path(const sys::path& other)
	: pathname_(other.pathname_)
{
}

void sys::path::clear(void)
{
	pathname_.clear();
}

sys::path& sys::path::make_preferred(void)
{
#if defined(SYS_WIN32)
	std::replace(pathname_.begin(), pathname_.end(), '/', '\\');
#endif
	return *this;
}

sys::path& sys::path::make_absolute(const sys::path& base)
{
	pathname_.assign(absolute(base).c_str());
	return *this;
}

sys::path& sys::path::make_canonical(const sys::path& base)
{
	pathname_.assign(canonical(base).c_str());
	return *this;
}

sys::path& sys::path::remove_filename(void)
{
	pathname_.erase(parent_path_end());
	return *this;
}

sys::path& sys::path::remove_trailing_separator(void)
{
	if (pathname_.empty() && is_separator(pathname_[pathname_.size() - 1]))
		pathname_.erase(pathname_.size() - 1);
	return *this;
}

sys::path& sys::path::assign(const char* str)
{
	pathname_.assign(str == nullptr ? "" : str);
	return *this;
}

sys::path& sys::path::assign(const std::string& str)
{
	return assign(str.c_str());
}

sys::path& sys::path::assign(const sys::path& p)
{
	return assign(p.pathname_);
}

sys::path& sys::path::append(const char* str)
{
	if (str == nullptr || *str == '\0')
		return *this;
	if (str >= pathname_.data() &&
		str < pathname_.data() + pathname_.size())
	{
		path rhs(str);
		if (!is_separator(rhs.pathname_[0]))
			append_separator_if_needed();
		pathname_ += rhs.pathname_;
	}
	else
	{
		if (!is_separator(*str))
			append_separator_if_needed();
		pathname_ += str;
	}
	return *this;
}

sys::path& sys::path::append(const std::string& str)
{
	return append(str.c_str());
}

sys::path& sys::path::append(const sys::path& p)
{
	if (p.empty())
		return *this;
	if (this == &p)
	{
		path rhs(p);
		if (!is_separator(rhs.pathname_[0]))
			append_separator_if_needed();
		pathname_ += rhs.pathname_;
	}
	else
	{
		if (!is_separator(p.pathname_[0]))
			append_separator_if_needed();
		pathname_ += p.pathname_;
	}
	return *this;
}

bool sys::path::equal(const char* rhs) const
{
	return pathname_.compare(rhs) == 0;
}

bool sys::path::equal(const std::string& rhs) const
{
	return pathname_.compare(rhs) == 0;
}

bool sys::path::equal(const sys::path& rhs) const
{
	return pathname_.compare(rhs.pathname_) == 0;
}

sys::path& sys::path::operator=(const char* str)
{
	return assign(str);
}

sys::path& sys::path::operator=(const std::string& str)
{
	return assign(str);
}

sys::path& sys::path::operator=(const sys::path& p)
{
	return assign(p);
}

sys::path& sys::path::operator+=(const char* str)
{
	return append(str);
}

sys::path& sys::path::operator+=(const std::string& str)
{
	return append(str);
}

sys::path& sys::path::operator+=(const sys::path& p)
{
	return append(p);
}

bool sys::path::operator==(const char* rhs) const
{
	return equal(rhs);
}

bool sys::path::operator==(const std::string& rhs) const
{
	return equal(rhs);
}

bool sys::path::operator==(const sys::path& rhs) const
{
	return equal(rhs);
}

bool sys::path::operator!=(const char* rhs) const
{
	return !equal(rhs);
}

bool sys::path::operator!=(const std::string& rhs) const
{
	return !equal(rhs);
}

bool sys::path::operator!=(const sys::path& rhs) const
{
	return !equal(rhs);
}

sys::path sys::path::root_path(void) const
{
	path temp(root_name());
	if (!root_directory().empty())
		temp.pathname_ += root_directory().c_str();
	return temp;
}

sys::path sys::path::root_name(void) const
{
	iterator itr(begin());

	return (itr.pos_ != pathname_.size() && (
			(itr.element_.pathname_.size() > 1 &&
				is_separator(itr.element_.pathname_[0]) &&
				is_separator(itr.element_.pathname_[1]))
#if defined(SYS_WIN32)
			|| itr.element_.pathname_[itr.element_.pathname_.size() - 1] == ':'
#endif
			))
		? itr.element_
		: path();
}

sys::path sys::path::root_directory(void) const
{
	std::string::size_type pos(root_directory_start(pathname_, pathname_.size()));
	return pos == std::string::npos
		? path()
		: path(pathname_.c_str() + pos, pathname_.c_str() + pos + 1);
}

sys::path sys::path::relative_path() const
{
	iterator itr(begin());

	while (itr.pos_ != pathname_.size() && 
		(is_separator(itr.element_.pathname_[0])
#if defined(SYS_WIN32)
		|| itr.element_.pathname_[itr.element_.pathname_.size() - 1] == ':'
#endif
		))
		++itr;

	return path(pathname_.c_str() + itr.pos_);
}

sys::path sys::path::parent_path() const
{
	std::string::size_type end_pos(parent_path_end());
	return end_pos == std::string::npos ?
		path() : path(pathname_.c_str(), pathname_.c_str() + end_pos);
}

sys::path sys::path::filename() const
{
	std::string::size_type pos(filename_pos(pathname_, pathname_.size()));
	return (pathname_.size() && pos &&
			is_separator(pathname_[pos]) && !is_root_separator(pathname_, pos)) ?
		"." : path(pathname_.c_str() + pos);
}

sys::path sys::path::stem() const
{
	path name(filename());
	if (name.pathname_ == "." || name.pathname_ == "..")
		return name;
	std::string::size_type pos(name.pathname_.rfind('.'));
	return pos == std::string::npos ?
		name : path(name.pathname_.c_str(), name.pathname_.c_str() + pos);
}

sys::path sys::path::extension() const
{
	path name(filename());
	if (name.pathname_ == "." || name.pathname_ == "..")
		return path();
	std::string::size_type pos(name.pathname_.rfind('.'));
	return pos == std::string::npos ?
		path() : path(name.pathname_.c_str() + pos);
}

sys::path sys::path::current_path(void)
{
#if defined(SYS_WIN32)
	DWORD size = ::GetCurrentDirectoryA(0, NULL);
	if (size == 0)
		size = 1;
	std::vector<char> buf(size);
	::GetCurrentDirectoryA(size, buf.data());
	return path(buf.data());
#else
	for (long size = 128;; size *= 2)
	{
		std::vector<char> buf(size);
		if (::getcwd(buf.data(), static_cast<std::size_t>(size)) != 0)
			return path(buf.data());
	}
#endif
}

sys::path sys::path::executable_path(void)
{
	path exec_path;
#if defined(SYS_WIN32)
	std::vector<char> buf;
	DWORD copied = 0;
	do
	{
		buf.resize(buf.size() + MAX_PATH);
		copied = ::GetModuleFileNameA(0,
			buf.data(), static_cast<DWORD>(buf.size()));
	} while (copied >= buf.size());
	buf.resize(copied);
	exec_path.assign(buf.data());
#elif defined(SYS_HAVE_PROC_SELF_EXE)
	bool err(false);
	path self_exe(read_symlink("/proc/self/exe", err));
	if (!err)
		exec_path.assign(self_exe);
#elif defined(SYS_HAVE_PROC_SELF_PATH_AOUT)
	bool err(false);
	path self_aout(read_symlink("/proc/self/path/a.out", err));
	if (!err)
		exec_path.assign(self_aout);
#elif defined(SYS_HAVE_GETEXECNAME)
	const char* execname = ::getexecname();
	if (execname != nullptr)
	{
		exec_path.assign(execname);
		exec_path.make_canonical(initial_path);
	}
#elif defined(SYS_HAVE_DLGETNAME)
	struct ::load_module_desc desc;
	if (::dlget(-2, &desc, sizeof(desc)) != nullptr)
		exec_path.assign(::dlgetname(&desc, sizeof(desc), nullptr, 0, 0));
#elif defined(SYS_HAVE_PSTAT_GETPATHNAME)
	char buf[PATH_MAX];
	struct ::pst_status stat;
	if (::pstat_getproc(&stat, sizeof(stat), 0, ::getpid()) != -1)
	{
		if (::pstat_getpathname(buf, sizeof(buf), &stat.pst_fid_text) != -1)
			exec_path.assign(buf);
	}
#elif defined(SYS_HAVE_PROC_CURPROC_EXE)
	bool err(false);
	path curproc_exe(read_symlink("/proc/curproc/exe", err));
	if (!err)
		exec_path.assign(curproc_exe);
#elif defined(SYS_HAVE_PROC_CURPROC_FILE)
	bool err(false);
	path curproc_file(read_symlink("/proc/curproc/file", err));
	if (!err)
		exec_path.assign(curproc_file);
#elif defined(SYS_HAVE_NSGETEXECUTABLEPATH)
	std::size_t size = PATH_MAX;
	std::vector<char> buf(size);
	if (::_NSGetExecutablePath(buf.data(), &size) == -1)
	{
		buf.resize(size);
		if (::_NSGetExecutablePath(buf.data(), &size) == -1)
			return exec_path;
	}
	exec_path.assign(buf.data());
	exec_path.make_canonical(initial_path);
#elif defined(SYS_HAVE_PROC_PIDPATH)
	char buf[PROC_PIDPATHINFO_MAXSIZE];
	if (::proc_pidpath(::getpid(), buf, sizeof(buf)) > 0)
		exec_path.assign(buf.data());
#elif defined(SYS_HAVE_PROC_SELF_EXEFILE)
	std::ifstream ifs("/proc/self/exefile");
	std::string exefile;
	std::getline(ifs, exefile);
	if (!ifs.fail() && !exefile.empty())
		exec_path.assign(exefile);
#elif defined(SYS_HAVE_KERN_PROC_PATHNAME)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	for (;;)
	{
		std::size_t size;
		if (::sysctl(mib, 4, nullptr, &size, NULL, 0) < 0)
			break;
		std::vector<char> buf(size);
		if (::sysctl(mib, 4, buf.data(), &size, NULL, 0) < 0)
			break;
		exec_path.assign(buf.data());
		exec_path.make_canonical(initial_path);
		break;
	}
#elif defined(SYS_HAVE_KERN_PROC_ARGV)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC_ARGS;
	mib[2] = ::getpid();
	mib[3] = KERN_PROC_ARGV;

	for (;;)
	{
		std::size_t size;
		if (::sysctl(mib, 4, nullptr, &size, NULL, 0) < 0)
			break;
		std::vector<char*> buf(size / sizeof(char*));
		if (::sysctl(mib, 4, buf.data(), &size, NULL, 0) < 0)
			break;
		path procname(buf.data()[0]);
		exec_path.assign(procname.canonical(initial_path));
		if (!exec_path.empty())
			break;

		const std::string paths(std::getenv("PATH"));
		if (paths.empty())
			break;

		std::size_t prev = 0;
		std::size_t pos = paths.find(path_separator);
		while (pos != std::string::npos)
		{
			std::string s(paths.substr(prev, pos - prev));
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch); }));
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch); }).base(), s.end());
			exec_path.assign(procname.canonical(s));
			if (!exec_path.empty())
				break;
			prev = pos + 1;
			pos = paths.find(path_separator, prev);
		}
		if (pos == std::string::npos)
		{
			std::string s(paths.substr(prev));
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch); }));
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch); }).base(), s.end());
			exec_path.assign(procname.canonical(s));
		}
		break;
	}
#else
	return initial_path;
#endif
	return exec_path.remove_filename();
}

sys::path sys::path::absolute(const path& base) const
{
	path abs_base(base.is_absolute() ? base : absolute(base));
	if (empty())
		return abs_base;
	
	path root_name(root_name());
	path base_root_name(abs_base.root_name());
	path root_directory(root_directory());

	if (!root_name.empty())
	{
		if (root_directory.empty())
		{
			root_name.append(abs_base.root_directory());
			return root_name;
		}
	}
	else if (!root_directory.empty())
	{
#if !defined(SYS_WIN32)
		if (base_root_name.empty())
			return *this;
#endif
		base_root_name.append(*this);
		return base_root_name;
	}
	else
	{
		abs_base.append(*this);
		return abs_base;
	}
	return *this;
}

sys::path sys::path::canonical(const path& base) const
{
	path source(is_absolute() ? *this : absolute(base));
	path root(source.root_path());
	path result;

	bool err(false);
	file_type_t filetype(symlink_status(source, err));
	if (err || filetype == sys::file_not_found)
		return result;

	bool scan(true);
	while (scan)
	{
		scan = false;
		result.clear();
		for (path::iterator itr = source.begin(); itr != source.end(); ++itr)
		{
			if ((*itr).pathname_ == ".")
				continue;
			if ((*itr).pathname_ == "..")
			{
				if (result != root)
					result.remove_filename();
				continue;
			}

			result.append(*itr);

			bool err(false);
			bool is_sym(is_symlink(symlink_status(result, err)));
			if (err)
				return path();

			if (is_sym)
			{
				path link(read_symlink(result, err));
				if (err)
					return path();

				if (link.is_absolute())
				{
					for (++itr; itr != source.end(); ++itr)
						link.append(*itr);
					source = link;
				}
				else
				{
					path new_source(result);
					new_source.append(link);
					for (++itr; itr != source.end(); ++itr)
						new_source.append(*itr);
					source = new_source;
				}
				scan = true;
				break;
			}
		}
	}
	return result;
}

bool sys::path::empty(void) const
{
	return pathname_.empty();
}

bool sys::path::has_root_path(void) const
{
	return has_root_directory() || has_root_name();
}

bool sys::path::has_root_name(void) const
{
	return !root_name().empty();
}

bool sys::path::has_root_directory(void) const
{
	return !root_directory().empty();
}

bool sys::path::has_relative_path(void) const
{
	return !relative_path().empty();
}

bool sys::path::has_parent_path(void) const
{
	return !parent_path().empty();
}

bool sys::path::has_filename(void) const
{
	return !filename().empty();
}

bool sys::path::has_stem(void) const
{
	return !stem().empty();
}

bool sys::path::has_extension(void) const
{
	return !extension().empty();
}

bool sys::path::is_relative(void) const
{
	return !is_absolute();
}

bool sys::path::is_absolute(void) const
{
#if defined(SYS_WIN32)
	return has_root_name() && has_root_directory();
#else
	return has_root_directory();
#endif
}

const std::string& sys::path::native(void) const
{
	return pathname_;
}

const char* sys::path::c_str(void) const
{
	return pathname_.c_str();
}

std::string::size_type sys::path::size(void) const
{
	return pathname_.size();
}

sys::file_type_t sys::path::status(void) const
{
	bool err;
	return symlink_status(*this, err);
}

sys::path::iterator sys::path::begin() const
{
	iterator itr;
	itr.path_ptr_ = this;
	std::string::size_type size;
	first_element(pathname_, itr.pos_, size);
	itr.element_.pathname_ = pathname_.substr(itr.pos_, size);
	if (itr.element_.pathname_ == preferred_separator_string)
		itr.element_.pathname_ = separator_string;
	return itr;
}

sys::path::iterator sys::path::end() const
{
	iterator itr;
	itr.path_ptr_ = this;
	itr.pos_ = pathname_.size();
	return itr;
}

bool sys::path::exists(void) const
{
	return status() == directory_file;
}

bool sys::path::create(void) const
{
#if defined(SYS_WIN32)
	return ::CreateDirectoryA(c_str(), 0) != 0;
#else
	return ::mkdir(c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
#endif
}

bool sys::path::create_all(void) const
{
	path filename(filename());

	if (filename.native().size() == 1 && filename.native()[0] == '.')
		return parent_path().create_all();

	if (filename.native().size() == 2 &&
		filename.native()[0] == '.' && filename.native()[1] == '.')
		return parent_path().create_all();

	if (status() == directory_file)
		return true;

	path parent = parent_path();
	if (!parent.empty())
	{
		if (parent.status() == file_not_found)
		{
			if (!parent.create_all())
				return false;
		}
	}

	return create();
}

bool sys::path::remove(void) const
{
#if defined(SYS_WIN32)
	return ::RemoveDirectoryA(c_str()) != 0;
#else
	return ::rmdir(c_str()) == 0;
#endif
}

std::string::size_type sys::path::parent_path_end(void) const
{
	std::string::size_type end_pos(filename_pos(pathname_, pathname_.size()));
	bool was_separator(pathname_.size() && is_separator(pathname_[end_pos]));
	std::string::size_type root_pos(root_directory_start(pathname_, end_pos));

	while (end_pos > 0 && end_pos - 1 != root_pos &&
		is_separator(pathname_[end_pos - 1]))
		--end_pos;

	return (end_pos == 1 && root_pos == 0 && was_separator) ?
		std::string::npos : end_pos;
}

std::string::size_type sys::path::append_separator_if_needed(void)
{
	if (!pathname_.empty() &&
#if defined(SYS_WIN32)
		pathname_[pathname_.size() - 1] != ':' &&
#endif
		!is_separator(pathname_[pathname_.size() - 1]))
	{
		std::string::size_type tmp(pathname_.size());
		pathname_ += preferred_separator;
		return tmp;
	}
	return 0;
}

void sys::path::first_element(const std::string& src,
	std::string::size_type& pos, std::string::size_type& size)
{
	pos = 0;
	size = 0;
	if (src.empty())
		return;

	std::string::size_type len(src.size());
	std::string::size_type cur(0);

	if (len >= 2 && is_separator(src[0]) && is_separator(src[1]) &&
		(size == 2 || !is_separator(src[2])))
	{
		cur += 2;
		size += 2;
	}
	else if (is_separator(src[0]))
	{
		++size;
		while (cur + 1 < len && is_separator(src[cur + 1]))
		{
			++cur;
			++pos;
		}
		return;
	}

	while (cur < len
#if defined(SYS_WIN32)
		&& src[cur] != ':'
#endif
		&& !is_separator(src[cur]))
	{
		++cur;
		++size;
	}

#if defined(SYS_WIN32)
	if (cur == len)
		return;
	if (src[cur] == ':')
		++size;
#endif
}

bool sys::path::is_separator(const char& c)
{
	return c == separator
#if defined(SYS_WIN32)
		|| c == preferred_separator
#endif
		;
}

bool sys::path::is_root_separator(const std::string& str,
	std::string::size_type pos)
{
	while (pos > 0 && is_separator(str[pos - 1]))
		--pos;
	if (pos == 0)
		return true;
#if defined(SYS_WIN32)
	if (pos == 2 && std::isalpha(static_cast<unsigned char>(str[0])) &&
			str[1] == ':')
		return true;
#endif
	if (pos < 3 || !is_separator(str[0]) || !is_separator(str[1]))
		return false;
	return str.find_first_of(separators, 2) == pos;
}

std::string::size_type sys::path::root_directory_start(
	const std::string& str, std::string::size_type size)
{
#if defined(SYS_WIN32)
	if (size > 2 && str[1] == ':' && is_separator(str[2]))
		return 2;
#endif

	if (size == 2 && is_separator(str[0]) && is_separator(str[1]))
		return std::string::npos;

#if defined(SYS_WIN32)
	if (size > 4 && is_separator(str[0]) && is_separator(str[1]) &&
		str[2] == '?' && is_separator(str[3]))
	{
		std::string::size_type pos(str.find_first_of(separators, 4));
		return pos < size ? pos : std::string::npos;
	}
#endif

	if (size > 3 && is_separator(str[0]) &&
		is_separator(str[1]) && !is_separator(str[2]))
	{
		std::string::size_type pos(str.find_first_of(separators, 2));
		return pos < size ? pos : std::string::npos;
	}

	if (size > 0 && is_separator(str[0]))
		return 0;

	return std::string::npos;
}

std::string::size_type sys::path::filename_pos(const std::string& str,
	std::string::size_type end_pos)
{
	if (end_pos == 2 && is_separator(str[0]) && is_separator(str[1]))
		return 0;

	if (end_pos && is_separator(str[end_pos - 1]))
		return end_pos - 1;

	std::string::size_type pos(str.find_last_of(separators, end_pos - 1));

#if defined(SYS_WIN32)
	if (pos == std::string::npos && end_pos > 1)
		pos = str.find_last_of(':', end_pos - 2);
#endif

	return (pos == std::string::npos || (pos == 1 && is_separator(str[0]))) ?
		0 : pos + 1;
}

sys::path::iterator::iterator(void)
	: element_()
	, path_ptr_(nullptr)
	, pos_(std::string::npos)
{
}

const sys::path& sys::path::iterator::operator*() const
{
	return element_;
}

sys::path::iterator& sys::path::iterator::operator++()
{
	increment(); return *this;
}

sys::path::iterator sys::path::iterator::operator++(int)
{
	iterator tmp(*this); operator++(); return tmp;
}

sys::path::iterator& sys::path::iterator::operator--()
{
	decrement(); return *this;
}

sys::path::iterator sys::path::iterator::operator--(int)
{
	iterator tmp(*this); operator--(); return tmp;
}

bool sys::path::iterator::operator==(const sys::path::iterator& rhs) const
{
	return equal(rhs);
}

bool sys::path::iterator::operator!=(const sys::path::iterator& rhs) const
{
	return !equal(rhs);
}

bool sys::path::iterator::equal(const sys::path::iterator& rhs) const
{
	return path_ptr_ == rhs.path_ptr_ && pos_ == rhs.pos_;
}

void sys::path::iterator::increment(void)
{
	pos_ += element_.pathname_.size();
	if (pos_ == path_ptr_->pathname_.size())
	{
		element_.clear();
		return;
	}

	bool was_net(element_.pathname_.size() > 2 &&
		is_separator(element_.pathname_[0]) &&
		is_separator(element_.pathname_[1]) &&
		!is_separator(element_.pathname_[2]));

	if (is_separator(path_ptr_->pathname_[pos_]))
	{
		if (was_net
#if defined(SYS_WIN32)
			|| element_.pathname_[element_.pathname_.size() - 1] == ':'
#endif
		)
		{
			element_.pathname_ = preferred_separator;
			return;
		}

		while (pos_ != path_ptr_->pathname_.size() &&
				is_separator(path_ptr_->pathname_[pos_]))
			++pos_;

		if (pos_ == path_ptr_->pathname_.size() &&
				!is_root_separator(path_ptr_->pathname_, pos_ - 1))
		{
			--pos_;
			element_.pathname_ = ".";
			return;
		}
	}

	std::string::size_type end_pos(
		path_ptr_->pathname_.find_first_of(separators, pos_));
	if (end_pos == std::string::npos)
		end_pos = path_ptr_->pathname_.size();
	element_.pathname_ = path_ptr_->pathname_.substr(pos_, end_pos - pos_);
}

void sys::path::iterator::decrement(void)
{
	std::string::size_type end_pos(pos_);

	if (pos_ == path_ptr_->pathname_.size() &&
		path_ptr_->pathname_.size() > 1 &&
		is_separator(path_ptr_->pathname_[pos_ - 1]) &&
		!is_root_separator(path_ptr_->pathname_, pos_ - 1)
	)
	{
		--pos_;
		element_.pathname_ = ".";
		return;
	}

	std::string::size_type root_dir_pos(
		root_directory_start(path_ptr_->pathname_, end_pos));

	while (end_pos > 0 && end_pos - 1 != root_dir_pos &&
			is_separator(path_ptr_->pathname_[end_pos - 1]))
		--end_pos;

	pos_ = filename_pos(path_ptr_->pathname_, end_pos);
	element_.pathname_ = path_ptr_->pathname_.substr(pos_, end_pos - pos_);
	if (element_.pathname_ == preferred_separator_string)
		element_.pathname_ = separator_string;
}
