# Server setup in data centres

## Introduction
reduce power consumption of idling servers, setup/delayedoff mode

Dispatcher -> multiple servers
inter-arrival time and service time are exponential
M/M/m queue

## Design

1. use master clock that ticks every second
2. calculate the main check points and get result for these states only





## The setup/delayedoff system

### 4 Server states: 
* OFF: server is powered off, cannot process a job
* SETUP: server booting up, cannot process a job, descripted by ```setup_time```
* BUSY: server is processing a job
* DELAYEDOFF: no job in dispatcher after finish processing a job, countdown timer ```delayedoff_time``` until OFF state.

### Dispatcher Rules
* send job to the server with highest countdown timer value
* no servers in DELAYEDOFF, find server in OFF state and power on, job MARKED and enqueue
* no server in DELAYEDOFF or OFF, job UNMARKET and enqueue

When job departures from the server
* no job in queue: server start countdown timer until OFF
* one or more jobs in queue: server take job check MARKED or UNMARKET
		UNMARKED: process the job
		MARKED: find if other UNMARKED job in queue, replace if so
			if no UNMARKET job in queue, process the MARKED job
					turn off SETUP server depending on remaining time
					turn off the server with longest remaining time
					state change from SETUP to OFF immediately

* initial condition: all servers are OFF, dispatcher queue empty

## Input files

* mode_*.txt
random or trace

* para_*.txt

number of servers m
setup time
value of delayedoff time Tc

Eg 1.
3
50
100

* arrival_*.txt


* service_*.txt


## Output files

1. one file containing mean response time : mrt_*.txt
Eg 1.
41.250

2. one file containing departure times: departure_*.txt
Eg 1.
10.000  61.000
20.000  63.000
32.000  66.000
33.000  70.000



## Test scripts

		dp.set_state(1, "OFF");
		dp.set_delayedoff_counter(1, 50);
		dp.set_state(2, "DELAYEDOFF");
		dp.set_delayedoff_counter(2, 30);
		dp.set_state(3, "OFF");
		dp.set_delayedoff_counter(3, 40);
		dp.set_state(4, "BUSY");
		dp.set_delayedoff_counter(4, 40);
		dp.set_state(5, "OFF");
		dp.set_delayedoff_counter(5, 70);
		dp.set_state(6, "OFF");
		dp.set_delayedoff_counter(6, 60);



