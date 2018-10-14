#include "sys.thread_group.h"

sys::thread_group::thread_group()
{
}

sys::thread_group::~thread_group()
{
	for (auto it = threads_.begin(); it != threads_.end(); ++it)
		delete *it;
}

bool sys::thread_group::contains(const std::thread* t) const
{
	std::thread::id id = (t == nullptr) ? 
		std::this_thread::get_id() : t->get_id();
	std::shared_lock<std::shared_mutex> guard(mutex_);
	for (auto it = threads_.begin(); it != threads_.end(); ++it)
	{
		if ((*it)->get_id() == id)
			return true;
	}
	return false;
}

std::size_t sys::thread_group::size() const
{
	std::shared_lock<std::shared_mutex> guard(mutex_);
	return threads_.size();
}

void sys::thread_group::add(std::thread* t)
{
	if (t != nullptr)
	{
		std::lock_guard<std::shared_mutex> guard(mutex_);
		threads_.push_back(t);
	}
}

void sys::thread_group::remove(std::thread* t)
{
	std::lock_guard<std::shared_mutex> guard(mutex_);
	const auto it = std::find(threads_.begin(), threads_.end(), t);
	if (it != threads_.end())
		threads_.erase(it);
}

void sys::thread_group::join_all(void)
{
	std::shared_lock<std::shared_mutex> guard(mutex_);
	for (auto it = threads_.begin(); it != threads_.end(); ++it)
	{
		if ((*it)->joinable())
			(*it)->join();
	}
}