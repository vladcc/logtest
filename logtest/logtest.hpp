#ifndef LOGTEST_HPP
#define LOGTEST_HPP

#include "spinlock.hpp"
#include "thread_map.hpp"

#include <thread>
#include <queue>
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
	
	void enq_log_item(const log_item& item)
	{
		_qlock.lock();
		_msg_q.push(item);
		_qlock.unlock();
	}
	
	private:
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
	
	void _log_bad_map(const char * name)
	{
		enq_log_item(
			make_item_cstr("logger warning: can't map logger thread ")
		);
		enq_log_item(make_item_cstr(name));
		enq_log_item(make_item_char('\n'));
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
			
			if (!q_size || q_size == 1)
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
	
	std::thread _log_thread;
	std::queue<log_item> _msg_q;
	FILE * _where;
	spinlock _qlock;
	bool _work;
};

#endif
