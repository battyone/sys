#ifndef __SYS_NONCOPYABLE__
#define __SYS_NONCOPYABLE__

namespace sys
{
	class noncopyable
	{
	protected:
		constexpr noncopyable() = default;
		~noncopyable() = default;
	protected:
		noncopyable(const noncopyable&) = delete;
		noncopyable(noncopyable&&) = delete;
	protected:
		noncopyable & operator=(const noncopyable&) = delete;
		noncopyable& operator=(noncopyable&&) = delete;
	};
}

#endif

