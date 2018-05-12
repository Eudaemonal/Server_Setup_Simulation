#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <vector>
#include <string>

//#define DEBUG
#ifdef DEBUG
#define debug_cout(x)  std::cout << x
#else
#define debug_cout(x)
#endif 

// overload operator for vector
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v){
	for(size_t i = 0; i < v.size(); ++i)
		os << v[i] << " ";
	os << "\n";
	return os;
}

class Job{
public:
	Job(float t_arrival, float t_service)
		:mark(false), arrival_time(t_arrival), service_time(t_service), server_id(0)
	{}
	~Job(){}

	void marked(){
		mark = true;
	}

	void unmarked(){
		mark = false;
	}

	float get_arrival(){
		return arrival_time;
	}

	float get_departure(){
		return departure_time;
	}

	friend class Server;
	friend class Dispatcher;
private:
	// true: MARKED, false: UNMARKED (default)
	bool mark;
	float arrival_time;
	float service_time;
	float departure_time;

	// assigned server id for setup
	int server_id;
};

class Random;
class Server;
class Dispatcher;




std::vector<Job*> simulate(std::string mode, std::vector<float> arrival, std::vector<float> service,
	       	int m, float setup_time,
	       	float delayedoff_time, float time_end, bool reproducible);


#endif
