#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
/*
reduce power consumption of idling servers, setup/delayedoff mode

Dispatcher -> multiple servers
inter-arrival time and service time are exponential
M/M/m queue
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


// overload operator for vector
template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> v){
	for(auto it: v)
		os << it << " ";
	os << "\n";
	return os;
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
		setup_time(t_setup), setup_counter(-1),
		delayedoff_time(t_delay), delayedoff_counter(-1),
		current_job(nullptr), service_counter(-1),
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

	// set server state
	void set_state(std::string s){
		state = s;
		if(s=="DELAYEDOFF"){
			priority=1;
			setup_counter = -1;
			service_counter = -1;
		}else if(s=="OFF"){
			priority=2;
			setup_counter = -1;
			delayedoff_counter = -1;
			service_counter = -1;
		}else if(s=="SETUP"){
			priority=3;
			delayedoff_counter = -1;
			service_counter = -1;

		}else if(s=="BUSY"){
			priority=4;
			setup_counter = -1;
			delayedoff_counter = -1;
		}else{
			std::cerr << "Undefined Server state\n";
		}
	}


	void poweron(){
		debug_cout( "Server "<< id << " poweron\n");
		set_state("SETUP");
		setup_counter = setup_time;
	}

	
	void process_job(Job* job){
		current_job = job;
		service_counter = job->service_time;
		set_state("BUSY");
	}


	// update master clock and server state
	void update_clock(float clk){
		float step = clk - m_clock;
		// setup operation
		if(setup_counter > 0){
			setup_counter -= step;
		}
		if(setup_counter==0){
			set_state("DELAYEDOFF");
			delayedoff_counter = delayedoff_time;

			debug_cout("Server " << id << " starts, delayedoff\n");
		}
				
		// delayoff operation
		if(delayedoff_counter > 0){
			delayedoff_counter -= step;
		}
		if(delayedoff_counter==0){
			set_state("OFF");
			debug_cout("Server " << id << " off\n");
		}

		// busy operation
		if(service_counter > 0){
			service_counter -= step;
		}
		if(service_counter==0){
			set_state("DELAYEDOFF");
			delayedoff_counter = delayedoff_time;

			current_job->departure_time = clk;

			debug_cout("Server " << id << " job finish, delayedoff\n");
		}

		debug_cout("Server " << id << " " << state << " sc: " << setup_counter 
				<< " dc: " << delayedoff_counter << "\n");


		m_clock = clk;
	}



	friend class Dispatcher;

private:
	int id;
	//OFF(default), SETUP, BUSY, DELAYEDOFF
	std::string state;
	int priority;  // priority depend on state

	
	const float setup_time;
	float setup_counter;

	const float delayedoff_time;
	float delayedoff_counter;

	Job* current_job;
	float service_counter;

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

	// methods for testing only
	void set_state(int id, std::string s){
		servers[id-1]->set_state(s);
	}

	void set_delayedoff_counter(int id, float c){
		servers[id-1]->delayedoff_counter = c;
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
			return a->delayedoff_counter > b->delayedoff_counter;
		};
		std::sort(servers.begin(), servers.end(), compare);	

		// test only
		for(int i=0; i < servers.size(); ++i){
			std::cout << servers[i]->id << " ";
		}
		std::cout << "\n";

		// operations depend on server state
		if(servers[0]->state=="DELAYEDOFF"){
			servers[0]->process_job(job);

		}else if(servers[0]->state=="OFF"){
			servers[0]->poweron();
			job->marked();
			job->server_id = servers[0]->id;
			queue.push_back(job);
		}else if(servers[0]->state=="SETUP" || servers[0]->state=="BUSY"){
			queue.push_back(job);
		}

		
		return job;
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
			return a->delayedoff_counter > b->delayedoff_counter;
		};
		std::sort(servers.begin(), servers.end(), compare);
	

		if(job->mark==0 && servers[0]->state=="DELAYEDOFF"){
			servers[0]->process_job(job);
			queue.erase (queue.begin());
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
				if(i!=queue.size()-1){
					job = queue[i];
					servers[0]->process_job(job);
					queue.erase(queue.begin() + i);
					
					debug_cout( "Server " << servers[0]->id << " process job "
					<< "(" <<job->arrival_time << ", "<< job->service_time
					<< ", " <<job->mark<< ", sid: " << job->server_id <<")\n");
				}else{
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

	
	}else if(mode=="trace"){
		debug_cout("Simulation in trace mode: \n");

		float m_clock = 0;
		float step = get_min_step(arrival, service);
		debug_cout( "Time step: " << step <<"\n");

		// start dispatcher with server
		Dispatcher dp(m, setup_time, delayedoff_time, m_clock);

		// main loop in execution
		int j = 0;
		while(!dp.terminate(arrival.back())){
			dp.update_clock(m_clock);
			
			// job enter system at arrival time
			if(m_clock==arrival[j]){
				all_jobs.push_back(dp.job_arrive(arrival[j], service[j]));
				j++;
			}

			// process queued job if available
			dp.process_queue();

			m_clock+=step;
		}
		// simulation finished
		debug_cout( "Simulation summary: \n");
		float sum = 0;
		for(int i = 0; i < all_jobs.size(); ++i){
			std::cout << all_jobs[i]->get_arrival() << " " 
				<< all_jobs[i]->get_departure() << "\n";
			sum += (all_jobs[i]->get_departure() - all_jobs[i]->get_arrival());
		}
		std::cout << "mrt: "<<sum / (float)all_jobs.size() << "\n";


	
	}else{
		std::cerr << "unavilable mode\n";
	}
	return all_jobs;
}


int main(int argc, char *argv[]){
	int seqnum = 1;

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

	time_end = 0; // only relevent in random mode

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
		f_departure << finished_jobs[i]->get_arrival() << " " 
			<< finished_jobs[i]->get_departure() << "\n";
		sum += (finished_jobs[i]->get_departure() - finished_jobs[i]->get_arrival());
	}
	f_mrt <<sum / (float)finished_jobs.size() << "\n";

	// close files
	f_mrt.close();
	f_departure.close();
}
