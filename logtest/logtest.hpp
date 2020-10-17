#ifndef LOGTEST_HPP
#define LOGTEST_HPP

#include <atomic>
#include <thread>
#include <queue>
#include <cstdio>
#include <chrono>
#include <iostream>

#include <unistd.h> // usleep()

class logtest
{
	public:
	logtest(FILE * where) :
		_where(where)
	{
		_thread_start();
	}
	
	~logtest()
	{
		_terminate();
	}
	
	private:
	enum log_item_type {
		NONE = 0, CSTRING, DOUBLE, INT, CHAR
	};	
	
	public:
	struct log_item {
		union log_item_u {
			char * str;
			double dbl;
			int integ;
			char ch;
		} u_item;
		log_item_type tag;
	};
	
	inline log_item make_item_cstr(const char * reachable)
	{
		log_item item;
		item.u_item.str = (char *)reachable;
		item.tag = CSTRING;
		return item;
	}
	
	inline log_item make_item_double(double val)
	{
		log_item item;
		item.u_item.dbl = val;
		item.tag = DOUBLE;
		return item;
	}
	
	inline log_item make_item_int(int val)
	{
		log_item item;
		item.u_item.integ = val;
		item.tag = INT;
		return item;
	}
	
	inline log_item make_item_char(char ch)
	{
		log_item item;
		item.u_item.ch = ch;
		item.tag = CHAR;
		return item;
	}
	
	void enq_log_item(const log_item& item)
	{
		_qlock.lock();
		_msg_q.push(item);
		_qlock.unlock();
	}
		
	private:
	
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
				if (!_lock.exchange(true, std::memory_order_acquire))
					break;
				
				// spin on a read operation -> no cache trashing
				while (_lock.load(std::memory_order_relaxed))
					continue;
			}
		}
	  
		void unlock()
		{_lock.store(false, std::memory_order_release);}
		
		private:
		std::atomic<bool> _lock; // should check is_lock_free()
	};
#endif

	void _terminate()
	{
		bool wait = true;
		while (wait)
		{
			_qlock.lock();
			wait = !_msg_q.empty();
			_qlock.unlock();
			
			if (wait)
				usleep(7*1000);
		}
	
		_work = false;
		_log_thread.join();
	}
	
	void _log_thread_loop()
	{
		auto start = std::chrono::steady_clock::now();
		
		_work = true;
		while (_work)
		{
			size_t q_size = 0;
			log_item item = {};
			item.tag = NONE;
			
			_qlock.lock();
			if ((q_size = _msg_q.size()))
			{
				item = _msg_q.front();
				_msg_q.pop();
			}
			_qlock.unlock();
			
			switch (item.tag)
			{
				case NONE:
					break;
				case CSTRING:
					fputs(item.u_item.str, _where);
					break;
				case DOUBLE:
					fprintf(_where, "%.2f", item.u_item.dbl);
					break;
				case INT:
					fprintf(_where, "%d", item.u_item.integ);
					break;
				case CHAR:
					fputc(item.u_item.ch, _where);
					break;
				default:
					fprintf(stderr, "!!! item tag error; tag = %d !!!\n",
						item.tag
					);
					break;
			}
			
			if (q_size == 1 || !q_size)
				usleep(5*1000);
		}
		
		auto end = std::chrono::steady_clock::now();
		
		auto nanos =
			std::chrono::duration_cast
				<std::chrono::nanoseconds>(end - start);

		auto micros =
			std::chrono::duration_cast
				<std::chrono::microseconds>(end - start);

		auto mills =
			std::chrono::duration_cast
				<std::chrono::milliseconds>(end - start);

		std::clog << "logger time   : " << nanos.count()
			<< " nanos" << std::endl;
		std::clog << "logger time   : " << micros.count()
			<< " micros" << std::endl;
		std::clog << "logger time   : " << mills.count()
			<< " millis" << std::endl;
		std::clog << std::endl;
	}
	
	void _thread_start()
	{
		_log_thread = std::thread{&logtest::_log_thread_loop, this};
	}
	
	std::thread _log_thread;
	std::queue<log_item> _msg_q;
	FILE * _where;
	spinlock _qlock;
	bool _work;
};

#endif
