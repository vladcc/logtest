#include "logtest.hpp"

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
	logger.enq_log_item(logger.make_item_cstr("logging double: "));
	logger.enq_log_item(logger.make_item_int(val));
	logger.enq_log_item(logger.make_item_char('\n'));
}

int main(int argc, char * argv[]) 
{
	if (argc < 2)
	{
		std::cerr << "User: " << argv[0] << " <how-many-loops>" << std::endl;
		return -1;
	}
	
	int loops = 0;
	if (sscanf(argv[1], "%d", &loops) != 1)
	{
		std::cerr << "error :" << argv[1] << " not a number" << std::endl;
		return -1;
	}
	
	logtest logger(stdout);
	
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
