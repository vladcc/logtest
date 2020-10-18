#ifndef LOGTEST_HPP
#define LOGTEST_HPP


#include "thread_map.hpp"
#include "spin_queue.hpp"

#include <thread>
#include <cstdio>
#include <chrono>
#include <iostream>

#include <unistd.h> // usleep()

class logtest
{
	public:
	logtest(FILE * where, const char * loop_th_name, const thread_map& tm) :
		_where(where)
	{
		_log_thread = std::thread{&logtest::_log_thread_loop,
			this,
			loop_th_name,
			tm
		};
	}
	
	~logtest()
	{
		_terminate();
	}
	
	private:
	enum log_item_type {
		NONE = 0, CSTRING, DOUBLE, INT, CHAR
	};	
	
	
	struct log_item {
		union log_item_u {
			char * str;
			double dbl;
			int integ;
			char ch;
		} u_item;
		log_item_type tag;
	};
	
	public:
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
	
	inline void enq_log_item(const log_item& item)
	{_msg_q.enque(item);}
	
	private:
	
	inline void _sleep_little()
	{
		// while (1) usleep(1); results in ~10% cpu
		usleep(1);
	}
	
	inline void _sleep_more()
	{usleep(1000);}
	
	void _terminate()
	{
		bool wait = true;
		while (wait)
		{
			wait = _msg_q.size();			
			if (wait)
				_sleep_more();
		}
	
		_work = false;
		_log_thread.join();
	}
	
	void _log_bad_map(const char * name)
	{
		enq_log_item(
			make_item_cstr("logger warning: can't map logger thread ")
		);
		enq_log_item(make_item_cstr(name));
		enq_log_item(make_item_char('\n'));
	}
	
	inline log_item _log_item_init()
	{
		log_item li = {};
		li.tag = NONE;
		return li;
	}
		
	void _log_thread_loop(const char * my_name, const thread_map& tmap)
	{
		if (!tmap.map_thread(my_name))
			_log_bad_map(my_name);
		
		// check on core numbers and thread names
		//while (true)
		//	sleep(1);
		
		auto start = std::chrono::steady_clock::now();
		
		_work = true;
		while (_work)
		{
			log_item item = _log_item_init();
			size_t q_size = _msg_q.deque(item);
			
#ifdef REPORT_QSIZE
	fprintf(_where, "queue size: %zu\n", q_size);
#endif			
			
			if (q_size)
			{
				switch (item.tag)
				{
					case NONE: // should never come here
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
					default: // should never come here either
						fprintf(stderr, "!!! item tag error; tag = %d !!!\n",
							item.tag
						);
						break;
				}
				
				
				if (1 == q_size)
					_sleep_little();
			}
			else
				_sleep_little();
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
	
	std::thread _log_thread;
	spin_queue<log_item> _msg_q;
	FILE * _where;
	spinlock _qlock;
	bool _work;
};

#endif
