#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include <iomanip>
#include <random>
/*
reduce power consumption of idling servers, setup/delayedoff mode

Dispatcher -> multiple servers
inter-arrival time and service time are exponential
M/M/m queue
*/


/*
problem: float percision, unable to start job after finish

accumulate float error during time addition

*/

#define DEBUG
#ifdef DEBUG
#define debug_cout(x)  std::cout << x
#else
#define debug_cout(x)
#endif 

class Job;
class Server;
class Dispatcher;


// global vector for critical time point
static std::vector<float> clock_v;


// overload operator for vector
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v){
	for(size_t i = 0; i < v.size(); ++i)
		os << v[i] << " ";
	os << "\n";
	return os;
}

bool equal_float(float f1, float f2){
	float epsilon = 0.01;
	if(fabs(f1 - f2) < epsilon)
		return true;
	return false;
}

// get the decimal of float
int decimals_float(float n){
	std::string s = std::to_string(n);
	s.erase(s.find_last_not_of('0') + 1, std::string::npos);
	std::string t = s.substr(s.find(".")+1);
	return t.length();
}

// get the minimal time resolution step
float get_min_step(std::vector<float> arrival, std::vector<float> service){
	int m_decimal = 0;
	int d;
	for(int i = 0; i < arrival.size(); ++i){
		d = decimals_float(arrival[i]);
		if(d > m_decimal)
			m_decimal =d;
	}
	for(int i = 0; i < service.size(); ++i){
		d = decimals_float(service[i]);
		if(d > m_decimal)
			m_decimal =d;
	}
	return std::pow(0.1, m_decimal);
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


class Server{
public:
	Server(int id, float t_setup, float t_delay, float clk)
		:id(id), state("OFF"), 
		setup_time(t_setup), setup_exp(-1),
		delayedoff_time(t_delay), delayedoff_exp(-1),
		current_job(nullptr), service_exp(-1),
		m_clock(clk)
	{
		if(state=="DELAYEDOFF"){
			priority=1;
		}else if(state=="OFF"){
			priority=2;
		}else if(state=="SETUP"){
			priority=3;
		}else if(state=="BUSY"){
			priority=4;
		}else{
			std::cerr << "Undefined Server state\n";
		}
	}
	~Server(){}

	// set server state
	void set_state(std::string s){
		state = s;
		if(s=="DELAYEDOFF"){
			priority=1;
			setup_exp = -1;
			service_exp = -1;
		}else if(s=="OFF"){
			priority=2;
			setup_exp = -1;
			delayedoff_exp = -1;
			service_exp = -1;
		}else if(s=="SETUP"){
			priority=3;
			delayedoff_exp = -1;
			service_exp = -1;

		}else if(s=="BUSY"){
			priority=4;
			setup_exp = -1;
			delayedoff_exp = -1;
		}else{
			std::cerr << "Undefined Server state\n";
		}
	}


	void poweron(){
		debug_cout( "Server "<< id << " poweron\n");
		set_state("SETUP");
		setup_exp = m_clock + setup_time;

		clock_v.push_back(setup_exp); // global
	}

	
	void process_job(Job* job){
		current_job = job;
		service_exp = m_clock + job->service_time;
		clock_v.push_back(service_exp); // global
		set_state("BUSY");
	}


	// update master clock and server state
	void update_clock(float clk){
		// setup operation
		if(equal_float(setup_exp, clk)){
			set_state("DELAYEDOFF");
			delayedoff_exp = clk + delayedoff_time;
			clock_v.push_back(delayedoff_exp); // gloal
			debug_cout("Server " << id << " starts, delayedoff\n");
		}
				
		// delayoff operation
		if(equal_float(delayedoff_exp, clk)){
			set_state("OFF");
			debug_cout("Server " << id << " off\n");
		}

		// busy operation
		if(equal_float(service_exp, clk)){
			set_state("DELAYEDOFF");
			delayedoff_exp = clk + delayedoff_time;
			clock_v.push_back(delayedoff_exp); // gloal

			current_job->departure_time = clk;

			debug_cout("Server " << id << " job finish, delayedoff\n");
		}

		debug_cout("Server " << id << " " << state << "\n");


		m_clock = clk;
	}



	friend class Dispatcher;

private:
	int id;
	//OFF(default), SETUP, BUSY, DELAYEDOFF
	std::string state;
	int priority;  // priority depend on state

	
	const float setup_time;
	float setup_exp;

	const float delayedoff_time;
	float delayedoff_exp;

	Job* current_job;
	float service_exp;

	// master clock
	float m_clock;
};


class Dispatcher{
public:
	Dispatcher(int m, float t_setup, float t_delay, float clk)
		:m_clock(clk)
	{
		for(int i = 0;i < m; ++i){
			Server *s = new Server(i+1, t_setup, t_delay, clk);
			servers.push_back(s);
		}
	}
	~Dispatcher(){}



	// methods for testing only
	void set_state(int id, std::string s){
		servers[id-1]->set_state(s);
	}


	// dispatch jobs on arrival
	Job* job_arrive(float t_arrival, float t_service){
		debug_cout("Job arrive at: " << t_arrival << " with service time: " << t_service << "\n");
		Job *job = new Job(t_arrival, t_service);
		
		// compare servers on condition, most suitable one appares first
		auto compare = [](const Server *a, const Server *b){
			if(!(a->state=="DELAYEDOFF" && b->state=="DELAYEDOFF")){
				return a->priority < b->priority;
			}
			return a->delayedoff_exp > b->delayedoff_exp;
		};
		std::sort(servers.begin(), servers.end(), compare);	

		// test only
		for(int i=0; i < servers.size(); ++i){
			std::cout << servers[i]->id << " ";
		}
		std::cout << "\n";

		// operations depend on server state
		assign_job_to_servers(0, servers, job);

		return job;
	}

	// assign job to servers according to priority
	void assign_job_to_servers(int id, std::vector<Server*> &servers, Job* job){
		assert(id < servers.size());
		if(servers[id]->state=="DELAYEDOFF"){
			Job* job_assign = nullptr;
			int job_assign_id = -1;
			for(int i = 0; i < queue.size(); ++i){
				if(queue[i]->server_id==servers[id]->id){
					job_assign = queue[i];
					job_assign_id = i;
				}
			}
			if(job_assign!=nullptr && job_assign_id != -1){
				servers[id]->process_job(job_assign);
				queue.erase(queue.begin() + job_assign_id);
				//queue.push_back(job);
				assign_job_to_servers(id+1, servers, job);

				debug_cout( "Server " << servers[id]->id << " process job "
					<< "(" <<job_assign->arrival_time << ", "<< job_assign->service_time
					<< ", " <<job_assign->mark<< ", sid: " << job_assign->server_id <<")\n");
			}else{
				servers[id]->process_job(job);
				debug_cout( "Server " << servers[id]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");
			}

		}else if(servers[id]->state=="OFF"){
			servers[id]->poweron();
			job->marked();
			job->server_id = servers[id]->id;
			queue.push_back(job);
		}else if(servers[id]->state=="SETUP" || servers[id]->state=="BUSY"){
			queue.push_back(job);
		}

	}

	// process job in queue
	void process_queue(){
		if(queue.empty())
			return;

		Job *job = queue.front();
		// compare servers on condition, most suitable one appares first
		auto compare = [](const Server *a, const Server *b){
			if(!(a->state=="DELAYEDOFF" && b->state=="DELAYEDOFF")){
				return a->priority < b->priority;
			}
			return a->delayedoff_exp > b->delayedoff_exp;
		};
		std::sort(servers.begin(), servers.end(), compare);
	

		if(job->mark==0 && servers[0]->state=="DELAYEDOFF"){
			servers[0]->process_job(job);
			queue.erase (queue.begin());

			debug_cout( "Server " << servers[0]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");

		}else if(job->mark==1 && servers[0]->state=="DELAYEDOFF"){
			if(job->server_id==servers[0]->id){
				servers[0]->process_job(job);
				queue.erase(queue.begin());

				debug_cout( "Server " << servers[0]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");
			}else{
				//find if other UNMARKED job in queue, replace if so
				int i;
				for(i=0; i < queue.size(); ++i){
					if(queue[i]->mark==false)
						break;
				}
				// unmarked job found
				if(i!=queue.size()){
					servers[0]->process_job(job);
					queue.erase(queue.begin());
					queue[i]->marked();
					
					
					debug_cout( "Server " << servers[0]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");
				}
				// no other unmarked job found 
				else{
					for(int i=0; i < servers.size(); ++i){
						if(servers[i]->id == job->server_id){
							assert(servers[i]->state=="SETUP");
							servers[i]->set_state("OFF");
							debug_cout( "Turn off setup server "
								       	<< servers[i]->id << "\n");
						}
					}

					servers[0]->process_job(job);
					queue.erase(queue.begin());

					debug_cout( "Server " << servers[0]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");
				}
			}
		}
	}

	// update master clock
	void update_clock(float clk){
		m_clock = clk;
		debug_cout("*********** Dispatcher master clock: " << m_clock << "\n");

		for(auto it: queue)
			debug_cout( "(" <<it->arrival_time << ", "<< it->service_time
				<< ", " <<it->mark<< ", sid: " << it->server_id <<")\n");

		
		for(int i= 0; i < servers.size(); ++i){
			servers[i]->update_clock(clk);
		}
	}

	// terminate condition check
	bool terminate(float t_last_in){
		if(m_clock < t_last_in)
			return false;
		for(int i= 0; i < servers.size(); ++i){
			if(servers[i]->state!="OFF")
				return false;
		}
		return true;
	}



private:
	std::vector<Server*> servers;
	std::vector<Job*> queue;

	// master clock
	float m_clock;
};


std::vector<Job*> simulate(std::string mode, std::vector<float> arrival, std::vector<float> service,
	       	int m, float setup_time,
	       	float delayedoff_time, float time_end)
{
	std::vector<Job*> all_jobs;

	if(mode=="random"){
		debug_cout("Simulation in random mode: \n");
		assert(arrival.size()==1);
		assert(service.size()==1);

		float lambda = arrival[0];
		float mu = service[0];

		std::random_device rd;
		std::mt19937 gen(rd());

		// inter-arrival probability distribution
		std::exponential_distribution<float> d_arrival(lambda);

		// service time probability distribution
		std::exponential_distribution<float> d_service(mu);

		// generate simulation data
		std::vector<float> r_arrival;
		std::vector<float> r_service;

		float at;
		float st;
		while(at < time_end){
			st = 0;
			for(int i=0; i < 3; ++i){
				st += d_service(gen);
			}
			at += 1/d_arrival(gen);

			st = roundf(st * 10) / 10;
			at = roundf(at * 10) / 10;

			r_arrival.push_back(at);
			r_service.push_back(st);
		}

		std::cout << "arrival time: \n";
		for(auto it: r_arrival)
			std::cout << it << "\n";
		std::cout << "service time: \n";
		for(auto it: r_service)
			std::cout << it << "\n";
		std::cout << "\n";

		float m_clock = 0;
		clock_v.push_back(0.0);
		for(int i = 0; i < r_arrival.size(); ++i){
			clock_v.push_back(r_arrival[i]);
		}

		// start dispatcher with server
		Dispatcher dp(m, setup_time, delayedoff_time, m_clock);


		// main loop in execution
		int i = 0;
		int j = 0;
		while(i < clock_v.size()){
			dp.update_clock(m_clock);
			
			std::sort(clock_v.begin(), clock_v.end());

			// job enter system at arrival time
			if(m_clock == r_arrival[j]){
				all_jobs.push_back(dp.job_arrive(r_arrival[j], r_service[j]));
				j++;
			}
			// process queued job if available
			dp.process_queue();
			std::sort(clock_v.begin(), clock_v.end());

			i++;
			m_clock = clock_v[i];
		}
		// simulation finished
		debug_cout( "Simulation summary: \n");
		float sum = 0;
		for(int i = 0; i < all_jobs.size(); ++i){
			std::cout <<std::fixed << std::setprecision(3) 
				<< all_jobs[i]->get_arrival() << " " 
				<< all_jobs[i]->get_departure() << "\n";
			sum += (all_jobs[i]->get_departure() - all_jobs[i]->get_arrival());
		}
		std::cout  <<std::fixed << std::setprecision(3)
			<< "mrt: "<<sum / (float)all_jobs.size() << "\n";

	}else if(mode=="trace"){
		debug_cout("Simulation in trace mode: \n");
		float m_clock = 0;
		clock_v.push_back(0.0);
		for(int i = 0; i < arrival.size(); ++i){
			clock_v.push_back(arrival[i]);
		}

		// start dispatcher with server
		Dispatcher dp(m, setup_time, delayedoff_time, m_clock);


		// main loop in execution
		int i = 0;
		int j = 0;
		while(i < clock_v.size()){
			dp.update_clock(m_clock);
			
			std::sort(clock_v.begin(), clock_v.end());
			clock_v.erase( std::unique(clock_v.begin(), clock_v.end() ), clock_v.end());
			// job enter system at arrival time
			if(m_clock == arrival[j]){
				all_jobs.push_back(dp.job_arrive(arrival[j], service[j]));
				j++;
			}
			// process queued job if available
			dp.process_queue();
			std::sort(clock_v.begin(), clock_v.end());
			clock_v.erase( std::unique(clock_v.begin(), clock_v.end() ), clock_v.end());
			i++;
			m_clock = clock_v[i];
		}
		// simulation finished
		debug_cout( "Simulation summary: \n");
		float sum = 0;
		for(int i = 0; i < all_jobs.size(); ++i){
			std::cout <<std::fixed << std::setprecision(3) 
				<< all_jobs[i]->get_arrival() << " " 
				<< all_jobs[i]->get_departure() << "\n";
			sum += (all_jobs[i]->get_departure() - all_jobs[i]->get_arrival());
		}
		std::cout  <<std::fixed << std::setprecision(3)
			<< "mrt: "<<sum / (float)all_jobs.size() << "\n";
	
	}else{
		std::cerr << "unavilable mode\n";
	}
	return all_jobs;
}

/*
Useage: 
make
./simulation seqnum

*/

int main(int argc, char *argv[]){
	assert(argc==2);
	int seqnum = atoi(argv[1]);

	std::string n_mode = "mode_" + std::to_string(seqnum) + ".txt";
	std::string n_para = "para_"+ std::to_string(seqnum) + ".txt";
	std::string n_arrival = "arrival_"+ std::to_string(seqnum) + ".txt";
	std::string n_service = "service_"+ std::to_string(seqnum) + ".txt";
	
	// setup files to read
	std::ifstream f_mode(n_mode);
	std::ifstream f_para(n_para);
	std::ifstream f_arrival(n_arrival);
	std::ifstream f_service(n_service);

	// setup containers for arguments
	std::string mode;

	int n_server;
	float setup_time;
	float delayedoff_time;

	std::vector<float> arrival;

	std::vector<float> service;

	float time_end;

	// read arguments from files
	f_mode >> mode;
	
	f_para >> n_server;
	f_para >> setup_time;
	f_para >> delayedoff_time;

	float a;
	while(f_arrival >> a){
		arrival.push_back(a);
	}
	
	while(f_service >> a){
		service.push_back(a);
	}

	time_end = 10; // only relevent in random mode

	// close files
	f_mode.close();
	f_para.close();
	f_arrival.close();
	f_service.close();
	
	// check arguemnt value
	debug_cout( "Mode: " << mode << "\n");
	debug_cout( "N server: " << n_server << "\n");
	debug_cout( "setup: " << setup_time << "\n");
	debug_cout( "delayed off: " << delayedoff_time << "\n");
	debug_cout( "arrival: "<< arrival);
	debug_cout( "service: " << service);

	std::vector<Job* > finished_jobs;
	finished_jobs = simulate(mode, arrival, service, n_server, setup_time,
	       			delayedoff_time, time_end);


	// write simulation results to file
	std::string n_mrt = "mrt_" + std::to_string(seqnum) + ".txt";
	std::string n_departure = "departure_"+ std::to_string(seqnum) + ".txt";
	
	std::ofstream f_mrt(n_mrt);
	std::ofstream f_departure(n_departure);
	

	debug_cout( "Write results to file... \n");
	float sum = 0;
	for(int i = 0; i < finished_jobs.size(); ++i){
		f_departure <<std::fixed << std::setprecision(3) 
			<< finished_jobs[i]->get_arrival() << " " 
			<< finished_jobs[i]->get_departure() << "\n";
		sum += (finished_jobs[i]->get_departure() - finished_jobs[i]->get_arrival());
	}
	f_mrt << std::fixed << std::setprecision(3) << sum / (float)finished_jobs.size() << "\n";

	// close files
	f_mrt.close();
	f_departure.close();

}
