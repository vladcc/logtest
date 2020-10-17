#include "logtest.hpp"
#include "thread_map.hpp"

#include <iostream>



void log_line(logtest& logger, const char * str)
{
	logger.enq_log_item(logger.make_item_cstr("logging string: "));
	logger.enq_log_item(logger.make_item_cstr(str));
	logger.enq_log_item(logger.make_item_char('\n'));
}

void log_double(logtest& logger, double val)
{
	logger.enq_log_item(logger.make_item_cstr("logging double: "));
	logger.enq_log_item(logger.make_item_double(val));
	logger.enq_log_item(logger.make_item_char('\n'));
}

void log_int(logtest& logger, int val)
{
	logger.enq_log_item(logger.make_item_cstr("logging int: "));
	logger.enq_log_item(logger.make_item_int(val));
	logger.enq_log_item(logger.make_item_char('\n'));
}

void run(int loops, const char * log_th_name, const thread_map& tm)
{
	logtest logger(stdout, log_th_name, tm);
	
	auto start = std::chrono::steady_clock::now();
	
	for (int i = 0; i < loops; ++i)
	{
		log_line(logger, "this is a string");
		log_double(logger, 5.4);
		log_int(logger, 77);
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
			
	std::clog << "main time   : " << nanos.count()
		<< " nanos" << std::endl;
	std::clog << "main time   : " << micros.count()
		<< " micros" << std::endl;
	std::clog << "main time   : " << mills.count()
		<< " millis" << std::endl;
	std::clog << std::endl;
}

void map_myself(const char * my_name, const thread_map& tmap)
{
	if (!tmap.map_thread(my_name))
		fprintf(stderr, "warning: can't map thread '%s'\n", my_name);
}

int main(int argc, char * argv[]) 
{
	if (argc != 4)
	{
		std::cerr << "Use: " << argv[0] << " <how-many-loops> "
			<< "<core-main-thread> <core-logger-thread>" << std::endl;
		return -1;
	}
	
	int loops = 0;
	if (sscanf(argv[1], "%d", &loops) != 1)
	{
		std::cerr << "error :" << argv[1] << " not a number" << std::endl;
		return -1;
	}
	
	static const char main_th_cfg_name[] = "main-thread";
	static const char log_th_cfg_name[] = "out-of-thread-logger";
	static const char log_th_sys_name[] = "oot-log-poc";
	
	thread_map tmap;
	if (!tmap.add(main_th_cfg_name, main_th_cfg_name, argv[2]))
	{
		fprintf(stderr, "warning: can't add thread '%s' to core '%s'\n",
			main_th_cfg_name, argv[2]
		);
	}	
	
	if (!tmap.add(log_th_cfg_name, log_th_sys_name, argv[3]))
	{
		fprintf(stderr, "warning: can't add thread '%s' to core '%s'\n",
			log_th_cfg_name, argv[3]
		);
	}
	
	map_myself(main_th_cfg_name, tmap);
	run(loops, log_th_cfg_name, tmap);
}
