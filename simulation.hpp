#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <vector>
#include <string>
#include <cmath>

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

// random number generator
class Random{
public:
	// random number with LGM method, with period 2^31
	Random(unsigned int seed)
		:__x(seed)
	{
		__m = 0x7fffffff;
		__a = 0x41a7;
		__b = 0;
	}

	// generate persudo random int sequence
	unsigned int random(){
		__x = (__a * __x + __b) % __m;
		return __x;
	}
	
	// generate random float within [0, 1]
	float randomfloat(){
		__x = (__a * __x + __b) % __m;
		return (float)__x/__m;
	}
	
	// generate exponential distribution
	float exponential(float lambda){
		return -lambda * log(1 - randomfloat());
	}
private:
	unsigned int __m;
	unsigned int __a;
	unsigned int __b;
	unsigned int __x;
};


class Server;
class Dispatcher;




std::vector<Job*> simulate(std::string mode, std::vector<float> arrival, std::vector<float> service,
	       	int m, float setup_time,
	       	float delayedoff_time, float time_end, bool reproducible);


#endif
