#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cmath>
#include "simulation.hpp"

/*
Useage: 
make
./design

*/

float run(int seqnum, float d_time){
	// setup containers for arguments
	std::string mode;

	int n_server;
	float setup_time;
	float delayedoff_time;
	float time_end;
	bool reproducible = false;

	std::vector<float> arrival;
	std::vector<float> service;

	// read arguments from files
	mode = "random";

	n_server = 5;
	setup_time = 5;
	delayedoff_time = d_time;
	time_end = 500;

	arrival.push_back(0.35);
	service.push_back(1);
	
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
	       			delayedoff_time, time_end, reproducible);

	float sum = 0;
	for(int i = 0; i < finished_jobs.size(); ++i){
		sum += (finished_jobs[i]->get_departure() - finished_jobs[i]->get_arrival());
	}
	return sum / (float)finished_jobs.size();
}



int main(int argc, char *argv[]){
	// each Tc value will run n_test times
	int n_test = 50;
	int n_case = 30;
	float mrt;
	// all test cases of Tc
	std::vector<float> tcv(n_case);
	for(int i=0; i < tcv.size(); ++i){
		tcv[i] = 0.1 * pow(2, i);
	}

	for(int i = 0; i < tcv.size(); ++i){
		mrt = 0;
		for(int j = 0; j < n_test; ++j){
			mrt += run(0, tcv[i]);
		}
		mrt = mrt/n_test;
		std::cout << "Tc: " << tcv[i] << " mean mrt: " << mrt << "\n";
	}
}

