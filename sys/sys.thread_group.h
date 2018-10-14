#ifndef __SYS_THREAD_GROUP__
#define __SYS_THREAD_GROUP__

#include <list>
#include <memory>
#include <thread>
#include <shared_mutex>

#include "sys.noncopyable.h"

namespace sys
{
	class thread_group : public noncopyable
	{
		std::list<std::thread*> threads_;
		mutable std::shared_mutex mutex_;
	public:
		thread_group();
		virtual ~thread_group();
	public:
		bool contains(const std::thread* t) const;
		std::size_t size() const;
	public:
		template<class Function>
		std::thread* create(Function&& f);
	public:
		void add(std::thread* t);
		void remove(std::thread* t);
	public:
		void join_all(void);
	};

	template<class Function>
	std::thread* thread_group::create(Function&& f)
	{
		std::lock_guard<std::shared_mutex> guard(mutex_);
		std::unique_ptr<std::thread> t(new std::thread(f));
		threads_.push_back(t.get());
		return t.release();
	}
}

#endif
