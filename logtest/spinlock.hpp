#ifndef SPINLOCK_HPP
#define SPINLOCK_HPP

#include <atomic>

#ifdef TRASH_CACHE
	// see https://www.boost.org/doc/libs/1_56_0/boost/smart_ptr/detail/spinlock_std_atomic.hpp
	// see https://gist.github.com/vertextao/9a9077720c15fec89ed1f3fd91c9e91a
	class spinlock
	{
		public:
			spinlock() { m_lock.clear(); }
			spinlock(const spinlock&) = delete;
			~spinlock() = default;

			void lock()
			{
				while (m_lock.test_and_set(std::memory_order_acquire));
			}
			void unlock()
			{
				m_lock.clear(std::memory_order_release);
			}
		private:
			std::atomic_flag m_lock;
	};
#else
	class spinlock
	{
		// see https://rigtorp.se/spinlock/
		public:
		spinlock() : _lock(false) {}
		
		void lock()
		{
			for (;;)
			{
				// try to acquire once
#ifdef READ_EXCH
				if (!_lock.load(std::memory_order_relaxed) &&
					!_lock.exchange(true, std::memory_order_acquire))
#else
				if (!_lock.exchange(true, std::memory_order_acquire))
#endif
					break;
				
				// spin on a read operation -> no cache trashing
				while (_lock.load(std::memory_order_relaxed))
					continue;//__builtin_ia32_pause(); // <-- needed for HT
			}
		}
	  
		void unlock()
		{_lock.store(false, std::memory_order_release);}
		
		private:
		std::atomic<bool> _lock; // should check is_lock_free()
	};
#endif

#endif
