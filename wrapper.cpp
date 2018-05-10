#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "simulation.hpp"

/*
Useage: 
make
./wrapper

*/

void run(int seqnum){
	bool reproducible = true;

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
	float time_end;

	std::vector<float> arrival;

	std::vector<float> service;

	// read arguments from files
	f_mode >> mode;
	
	f_para >> n_server;
	f_para >> setup_time;
	f_para >> delayedoff_time;

	if(mode=="random"){
		f_para >> time_end;
	}

	float a;
	while(f_arrival >> a){
		arrival.push_back(a);
	}
	
	while(f_service >> a){
		service.push_back(a);
	}

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

	// run simulation function, results store in finished_jobs
	std::vector<Job* > finished_jobs;
	finished_jobs = simulate(mode, arrival, service, n_server, setup_time,
	       			delayedoff_time, time_end, true);


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



int main(int argc, char *argv[]){
	std::string n_test = "num_tests.txt";
	
	// setup files to read
	std::ifstream f_test(n_test);
	int n;
	f_test >> n;
 
	for(int i=1; i <= n; ++i){
		run(i);
	}
	
	// close file
	f_test.close();

}
