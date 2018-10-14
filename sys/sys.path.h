#ifndef __SYS_PATH__
#define __SYS_PATH__

#include <iterator>
#include <string>

namespace sys
{
	typedef enum
	{
		status_error, file_not_found, regular_file, directory_file,
		symlink_file, block_file, character_file, fifo_file,
		socket_file, reparse_file, type_unknown
	} file_type_t;

	class path
	{
		std::string pathname_;
	public:
		static const char separator;
		static const char preferred_separator;
		static const char* const separators;
		static const char* separator_string;
		static const char* preferred_separator_string;
		static const char path_separator;
	public:
		path(void);
		path(const char* pathname);
		path(const std::string& pathname);
		path(const char* first, const char* last);
		path(const path& other);
	public:
		void clear(void);
		path& make_preferred(void);
		path& make_absolute(const path& base = current_path());
		path& make_canonical(const path& base = current_path());
	public:
		path& remove_filename(void);
		path& remove_trailing_separator(void);
	public:
		path& assign(const char* str);
		path& assign(const std::string& str);
		path& assign(const path& p);
	public:
		path& append(const char* str);
		path& append(const std::string& str);
		path& append(const path& p);
	public:
		bool equal(const char* rhs) const;
		bool equal(const std::string& rhs) const;
		bool equal(const path& rhs) const;
	public:
		path& operator=(const char* str);
		path& operator=(const std::string& str);
		path& operator=(const path& p);
		path& operator+=(const char* str);
		path& operator+=(const std::string& str);
		path& operator+=(const path& p);
	public:
		bool operator==(const char* rhs) const;
		bool operator==(const std::string& rhs) const;
		bool operator==(const path& rhs) const;
		bool operator!=(const char* rhs) const;
		bool operator!=(const std::string& rhs) const;
		bool operator!=(const path& rhs) const;
	public:
		path root_path(void) const;
		path root_name(void) const;
	public:
		path root_directory() const;
		path relative_path() const;
		path parent_path() const;
		path filename() const;
		path stem() const;
		path extension() const;
	public:
		static path current_path(void);
		static path executable_path(void);
	public:
		path absolute(const path& base = current_path()) const;
		path canonical(const path& base = current_path()) const;
	public:
		bool empty(void) const;
		bool has_root_path(void) const;
		bool has_root_name(void) const;
		bool has_root_directory(void) const;
		bool has_relative_path(void) const;
		bool has_parent_path(void) const;
		bool has_filename(void) const;
		bool has_stem(void) const;
		bool has_extension(void) const;
		bool is_relative(void) const;
		bool is_absolute(void) const;
	public:
		const std::string& native(void) const;
		const char* c_str(void) const;
		std::string::size_type size(void) const;
		file_type_t status(void) const;
	public:
		class iterator;
		iterator begin() const;
		iterator end() const;
	public:
		bool exists(void) const;
		bool create(void) const;
		bool create_all(void) const;
		bool remove(void) const;
	private:
		std::string::size_type parent_path_end(void) const;
		std::string::size_type append_separator_if_needed(void);
	private:
		static void first_element(const std::string& src,
			std::string::size_type& pos, std::string::size_type& size);
		static bool is_separator(const char& c);
		static bool is_root_separator(const std::string& str,
			std::string::size_type pos);
		static std::string::size_type root_directory_start(
			const std::string& str, std::string::size_type size);
		static std::string::size_type filename_pos(const std::string& str,
			std::string::size_type end_pos);
	private:
		static bool is_symlink(const file_type_t& f);
		static path read_symlink(const path& p, bool& err);
		static file_type_t symlink_status(const path& p, bool& err);
	private:
		static const path initial_path;
	};

	class path::iterator : public std::iterator<std::input_iterator_tag, path>
	{
		path element_;
		const path* path_ptr_;
		std::string::size_type pos_;
	public:
		iterator(void);
	public:
		const path& operator*() const;
		iterator& operator++();
		iterator  operator++(int);
		iterator& operator--();
		iterator  operator--(int);
		bool operator==(const iterator& rhs) const;
		bool operator!=(const iterator& rhs) const;
	private:
		bool equal(const iterator& rhs) const;
		void increment(void);
		void decrement(void);
	friend class path;
	};
}

#endif

