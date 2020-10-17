#ifndef THREAD_MAP_HPP
#define THREAD_MAP_HPP

#include <pthread.h>
#include <sched.h>

#include <string>
#include <map>

class thread_map
{
	public:
	struct thread_info
	{
		std::string cfg_name;
		std::string sys_name;
		cpu_set_t cpuset;
	};
	
	bool add(const std::string& cfg_name,
		const std::string& sys_name,
		const char * core
	)
	{
		bool ret = false;
		
		thread_info new_ti;
		if (_cpu_set_from_str(core, &(new_ti.cpuset)))
		{			
			new_ti.cfg_name = cfg_name;
			new_ti.sys_name = sys_name;
			_tmap[cfg_name] = new_ti;
			ret = true;
		}
		
		return ret;
	} 
	
	bool get(const std::string& cfg_name, thread_info& out) const
	{
		// may not even be public
		
		bool ret = false;
		
		auto ti = _tmap.find(cfg_name);
		if (ti != _tmap.end())
		{
			out = ti->second;
			ret = true;
		}
		
		return ret;
	}
	
	bool map_thread(const char * th_cfg_name) const
	{
		const pid_t self = 0;
		const int max_thread_name_len = 15;
		thread_map::thread_info ti;
		
		// needs granular error handling
		return (get(th_cfg_name, ti)
			&& !pthread_setname_np(pthread_self(),
					ti.sys_name.substr(0, max_thread_name_len).c_str())
			&& !sched_setaffinity(self, sizeof(ti.cpuset), &ti.cpuset)
		);
	}
	
	private:	
	bool _cpu_set_from_str(const char * str, cpu_set_t * out)
	{
		bool ret = false;
		
		// parse more complicated strings here
		int core;
		if (sscanf(str, "%d", &core) == 1)
		{		
			CPU_ZERO(out);
			CPU_SET(core, out);
			ret = true;
		}
		
		return ret;
	}
	
	std::map<std::string, thread_info> _tmap;
};

#endif
