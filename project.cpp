#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <queue>
#include <deque>
#include <iostream>
#include <fstream>
#include "process.h"

class CPU {
	private:
		std::deque<Process> queue; //queue of processes
		std::priority_queue<Process*, std::vector<Process*>, Compare> pqueue; //priority queue of processes (wait state)
	public:
		Process* current = NULL; //currently running process
		Process* switching = NULL; //process currently being swapped in/out
		int context = 0; //counts remaining time of context switch

	void push_back(Process &p){
		p.tau = 0;
		queue.push_back(p);
	}

	void push(Process* p, int priority){
		p->tau = priority;
		pqueue.push(p);
	}

	void push(Process* p) {
		pqueue.push(p);
	}

	void pop_front(){
		return queue.pop_front();
	}

	void pop(){
		return pqueue.pop();
	}
	
	const Process& front() {
		return queue.front();
	}
	
	Process * top(){
		return pqueue.top();
	}

	void printQueue() {
		printf("[Q:");
		bool empty = true;
		for (auto i = queue.begin(); i != queue.end(); i++) {
			printf(" %c", (*i).ID);
			empty = false;
		}
		if (empty) {printf(" empty]\n");}
		else {printf("]\n");}
	}

	void printPQueue() {
		printf("[Q:");
		bool empty = true;
		std::priority_queue<Process*, std::vector<Process*>, Compare> temp;
		while (pqueue.size() > 0){
			Process * p = pqueue.top();
			printf(" %c", p->ID);
			empty = false;
			pqueue.pop();
			temp.push(p);
		}
		pqueue = temp;
		if (empty) {printf(" empty]\n");}
		else {printf("]\n");}
	}
	
	bool empty() {
		return queue.empty();
	}

	bool size() {
		return pqueue.size();
	}
};

double ceilTo3(double n){
	n *= 1000;
	n = ceil(n);
	return n/1000;
}

double totalBurstTime(Process* processes, int n){
	double total = 0;
	for (int i = 0; i < n; ++i){
		for (int e = 0; e < int(processes[i].CPUBursts.size()); ++e){
			total += processes[i].CPUBursts[e];
		}
	}
	return total;
}

double totalBursts(Process* processes, int n){
	double total = 0;
	for (int i = 0; i < n; ++i){
		total += processes[i].CPUBursts.size();
	}
	return total;
}

double avgCPUBurstTime(Process* processes, int n) {
	double total = totalBurstTime(processes, n);
	return ceilTo3(total / totalBursts(processes, n)); 
}

double CPUutil(Process* processes, int n, int total_time){
	double total = totalBurstTime(processes, n);
	return ceilTo3(total / total_time * 100);
}



void fetch(char** args, int& n, int& seed, double& lambda, int& bound, int& cs, double& alpha, int& slice) {
	//use arguments as buffers
	n = atoi(args[1]);
	seed = atoi(args[2]);
	lambda = strtod(args[3], nullptr);
	bound = atoi(args[4]);
	cs = atoi(args[5]);
	alpha = strtod(args[6], nullptr);
	slice = atoi(args[7]);
}

//reset all process variables
void resetAll(Process* p, int n) {
	for (int i = 0; i < n; i++) {
		//printf("here: %c\n", p[i].ID);
		p[i].reset();
	}
}

void printTime(int t) {
	printf("time %dms: ", t);
}

/***********************************************************/
void FCFS(Process* processes, int n, int cs, std::ostream& file) {
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n; //counter for how many processes are still alive
	int contextSwitches = 0;
	double totalWait = 0; //total wait time
	int waitCount = 0; //number of distinct wait times
	double turnarounds = 0; //total turnaround time
	
	//start
	printTime(time);
	printf("Simulator started for FCFS ");
	cpu.printQueue();
	
	//loop
	while (alive > 0 || cpu.context > 0) {
		//printf("context: %d, switch = %d, alive = %d\n", cpu.context, inSwitch, alive);
		assert(cpu.current == NULL || cpu.switching == NULL);
		
		//CPU things
		if (cpu.context > 0) {
			cpu.context--;
		}
		if (cpu.context == 0 && cpu.switching != NULL) {
			Process* a = cpu.switching;
			if (a->swap) { //switching into CPU
				a->inCPU = true;
				cpu.current = a;
				cpu.switching = NULL;
				cpu.pop_front();
				
				printTime(time);
				printf("Process %c started using the CPU for %dms burst ", a->ID, a->CPUBursts[a->step]);
				cpu.printQueue();
				contextSwitches++;
			}
			else { //switching out of CPU
				a->inIO = true;
				cpu.switching = NULL;
				a->turn = false;
			}
		}
		
		//process things
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			
			//check arrival time / I/O time
			if (time == p->nextArr) {
				cpu.push_back(*p);
				p->inQueue = true;
				p->turn = true;
				
				if (p->inIO) {
					p->inIO = false;
					p->step++;
					printTime(time);
					printf("Process %c completed I/O; added to ready queue ", p->ID);
					cpu.printQueue();
				}
				else {
					printTime(time);
					printf("Process %c arrived; added to ready queue ", p->ID);
					cpu.printQueue();
				}
			}
			
			//check wait time
			if (p->inQueue) {
				if (cpu.current == NULL && cpu.switching == NULL && *p == cpu.front()) { //no current process, run next process in queue
					p->inQueue = false;
					p->swap = true;
					cpu.context += cs/2;
					cpu.switching = p;
					p->CPUTime = 0;
					totalWait += p->waitTime;
					waitCount++;
					p->waitTime = 0;
				}
				else {p->waitTime++;}
			}
			
			//check CPU time
			if (p->inCPU) {
				if (p->CPUTime == p->CPUBursts[p->step]) { //CPU use done
					cpu.current = NULL;
					cpu.switching = p;
					cpu.context += cs/2;
					p->swap = false;
					p->inCPU = false;
					
					if (p->step == int(p->CPUBursts.size()-1)) {
						p->inQueue = false;
						p->inIO = false;
						alive--;
						
						printTime(time);
						printf("Process %c terminated ", p->ID);
						cpu.printQueue();
						continue;
					}
					
					//swap out to IO
					int next = time + (p->IOBursts)[p->step] + (cs/2);
					p->nextArr = next;
					
					if ((p->CPUBursts.size())-(p->step)-1 == 1) {
							printTime(time);
							printf("Process %c completed a CPU burst; %ld burst to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
							cpu.printQueue();
						}
						else {
							printTime(time);
							printf("Process %c completed a CPU burst; %ld bursts to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
							cpu.printQueue();
					}
					printTime(time);
					printf("Process %c switching out of CPU; will block on I/O until time %dms ", p->ID, next);
					cpu.printQueue();
				}
				p->CPUTime++;
			}
		}
		
		//turnaround counter
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (p->turn) {
				p->turnaround++;
			}
			else {
				if (p->turnaround) {std::cout << "here: " << p->turnaround << std::endl;}
				turnarounds += p->turnaround;
				p->turnaround = 0;
			}
		}
		
		time++;
	}
	time--;
	
	//end
	printTime(time);
	printf("Simulator ended for FCFS ");
	cpu.printQueue();
	
	//output
	file << "Algorithm FCFS\n";
	file << "-- average CPU burst time: " << avgCPUBurstTime(processes, n) << " ms\n";
	file << "-- average wait time: ";
	if (totalWait == 0){
		file << "0.000 ms\n";
	} 
	else {
		char buffer[100];
		sprintf(buffer,"%.3lfms\n", totalWait / waitCount);
		file << buffer;
	}
	file << "-- average turnaround time: " << ceilTo3(turnarounds / contextSwitches) << " ms\n";
	file << "-- total number of context switches: " << contextSwitches << "\n";
	file << "-- total number of preemptions: 0\n";
	file << "-- CPU utilization: " << CPUutil(processes, n, time) << "%\n";
}

/***********************************************************/
void SJF(Process * processes, int n, int cs, double alpha, double lambda, std::ofstream& file){
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n; //counter for how many processes are still alive
	int tau_init = int(ceil(1/lambda));

	int total_waitTime = 0;
	int context_switches = 0;

	printTime(time);
	printf("Simulator started for SJF ");
	cpu.printPQueue();
/*
If different types of events occur at the same time, simulate these events using the following order:
(a) CPU burst completion; (b) process starts using the CPU; (c) I/O burst completions (i.e., back
to the ready queue); and (d) new process arrivals.

*/
	while (alive > 0){
		if (cpu.current != NULL && cpu.current->remaining > 0 && cpu.context == 0){
			//cpu burst moving along
			cpu.current->remaining--;
		} else if (cpu.current != NULL && cpu.context == 0) {
/*
time 242ms: Process A (tau 100ms) completed a CPU burst; 13 bursts to go [Q: empty]
time 242ms: Recalculated tau for process A: old tau 100ms; new tau 154ms [Q: empty]
time 242ms: Process A switching out of CPU; will block on I/O until time 584ms [Q: empty]
*/
			//cpu burst completion and context switch if possible
			Process * current = cpu.current;
			cpu.current = NULL;
			current->inCPU = false;
			current->inQueue = false;
			//printf("%d < %d \n", current->step, current->CPUBursts.size());
			if (0 < current->CPUBursts.size() - current->step - 1){
				std::string grammar = "bursts";
				if (current->CPUBursts.size() - current->step - 1 == 1){
					grammar = "burst";
				}
				printTime(time);
				printf("Process %c (tau %dms) completed a CPU burst; %ld %s to go ", current->ID, current->tau, current->CPUBursts.size() - current->step - 1, grammar.c_str());
				cpu.printPQueue();

				int old_tau = current->tau;
				current->tau = ceil(alpha * current->getCurrentCPUBurst() + (1-alpha) * old_tau);

				printTime(time);
				printf("Recalculated tau for process %c: old tau %dms; new tau %dms ", current->ID, old_tau, current->tau);
				cpu.printPQueue();

				//added to io Burst
				current->inIO = true;
				//time remaining to get IOBurst and context switch out of this process
				current->remaining = current->getCurrentIOBurst() + cs / 2; 

				printTime(time);
				printf("Process %c switching out of CPU; will block on I/O until time %dms ", current->ID, time + cs / 2 + current->getCurrentIOBurst());
				cpu.printPQueue();

			} else {
				//process terminated
				printTime(time);
				printf("Process %c terminated ", current->ID);
				cpu.printPQueue();
				alive--;
			}

			cpu.context = cs / 2+1; //time for the process to leave the cpu (add one cause it is instantly subtracted)
		}

		//process starts using the cpu
		if (cpu.current == NULL){
			//either get process from ready queue or do nothing
			if (cpu.context > 0) {
				cpu.context--;
			} else if (cpu.context == 0 && cpu.size() > 0) {
				//move new process in
				cpu.current = cpu.top();
				cpu.pop();
				
				//wait for context switch before announcing 
				cpu.current->remaining = cpu.current->getCurrentCPUBurst() - 1;
				cpu.context = cs / 2 - 1;
				//std::cout << time << std::endl;
			} 
		} else if (cpu.context > 0) {
			cpu.context--;
			//announce arrival of process
			if (cpu.context == 0){
				printTime(time);
				printf("Process %c (tau %dms) started using the CPU for %dms burst ", cpu.current->ID, cpu.current->tau, cpu.current->getCurrentCPUBurst());
				cpu.printPQueue();

				context_switches++;
			}
		}

		//IOburst completions
		for (int i = 0; i < n; ++i){
			if (processes[i].inIO){
				if (processes[i].remaining == 0){
					processes[i].inIO = false;
					processes[i].inQueue = true;
					processes[i].step += 1;
					cpu.push(processes+i);

					printTime(time);
					printf("Process %c (tau %dms) completed I/O; added to ready queue ", processes[i].ID, processes[i].tau);
					cpu.printPQueue();
					total_waitTime--;
				} else {
					processes[i].remaining--;
				}
			}
		}

		
		//new process arrivals
		for (int i = 0; i < n; ++i){
			if (processes[i].arrival == time){
				cpu.push(processes+i, tau_init);
				processes[i].inQueue = true;
				printTime(time);
				printf("Process %c (tau %dms) arrived; added to ready queue ", processes[i].ID, tau_init);
				cpu.printPQueue();
				total_waitTime--;
			}
		}

		total_waitTime += cpu.size();

		++time;
		//printf("%d %d %p \n", time, cpu.context, cpu.current);
	}
	time = time + cs /2 - 1;
	printTime(time);
	printf("Simulator ended for SJF ");
	cpu.printQueue();

	file << "Algorithm SJF\n";
	file << "-- average CPU burst time: " << avgCPUBurstTime(processes, n) << " ms\n";
	file << "-- average wait time: ";
	if (total_waitTime == 0){
		file << "0.000 ms\n";
	} else {
		char buffer[100];
		sprintf(buffer,"%.3lf ms\n", double(total_waitTime) / totalBursts(processes, n));
		file << buffer;
	}
	file << "-- average turnaround time: " << ceilTo3((total_waitTime + totalBurstTime(processes, n) + context_switches * cs) / totalBursts(processes, n)) << " ms\n";
	file << "-- total number of context switches: " << context_switches << "\n";
	file << "-- total number of preemptions: " << 0 << "\n";
	file << "-- CPU utilization: " << CPUutil(processes, n, time) << "%\n";
}

/***********************************************************/

//this method should handle a newProcess preempting the current process. 
void preempt(CPU& cpu, int cs, int time, bool io = true, bool alreadyArrived = false){
	std::string finished = " completed I/O;";
	std::string preemptGrammar = "preempting";
	if (!io){
		finished = " arrived;";
	}

	if (alreadyArrived){
		preemptGrammar = "will preempt";
		finished = "";
	}

	printTime(time);
	printf("Process %c (tau %dms)%s %s %c ", cpu.top()->ID, cpu.top()->tau, finished.c_str(), preemptGrammar.c_str(), cpu.current->ID);
	cpu.printPQueue();

	//place the current process into the queue
	cpu.push(cpu.current);
	//take on the new process
	cpu.current = cpu.top();
	cpu.pop();
	//context switch
	cpu.context = cs;
}

void SRT(Process * processes, int n, int cs, double alpha, double lambda){
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n; //counter for how many processes are still alive
	int tau_init = int(ceil(1/lambda));
	int check_preempt = -1;

	printTime(time);
	printf("Simulator started for SRT ");
	cpu.printPQueue();
/*
If different types of events occur at the same time, simulate these events using the following order:
(a) CPU burst completion; (b) process starts using the CPU; (c) I/O burst completions (i.e., back
to the ready queue); and (d) new process arrivals.

*/
	while (alive > 0){
		if (cpu.current != NULL && cpu.current->remaining > 0 && cpu.context == 0){
			//cpu burst moving along
			cpu.current->remaining--;
		} else if (cpu.current != NULL && cpu.context == 0) {

			//cpu burst completion and context switch if possible
			Process * current = cpu.current;
			cpu.current = NULL;
			//printf("%d < %d \n", current->step, current->CPUBursts.size());
			if (0 < current->CPUBursts.size() - current->step - 1){
				std::string grammar = "bursts";
				if (current->CPUBursts.size() - current->step - 1 == 1){
					grammar = "burst";
				}
				printTime(time);
				printf("Process %c (tau %dms) completed a CPU burst; %ld %s to go ", current->ID, current->tau, current->CPUBursts.size() - current->step - 1, grammar.c_str());
				cpu.printPQueue();

				int old_tau = current->tau;
				current->tau = ceil(alpha * current->getCurrentCPUBurst() + (1-alpha) * old_tau);

				printTime(time);
				printf("Recalculated tau for process %c: old tau %dms; new tau %dms ", current->ID, old_tau, current->tau);
				cpu.printPQueue();

				//added to io Burst
				current->inIO = true;
				//time remaining to get IOBurst and context switch out of this process
				current->remaining = current->getCurrentIOBurst() + cs / 2; 

				printTime(time);
				printf("Process %c switching out of CPU; will block on I/O until time %dms ", current->ID, time + cs / 2 + current->getCurrentIOBurst());
				cpu.printPQueue();

			} else {
				//process terminated
				printTime(time);
				printf("Process %c terminated ", current->ID);
				cpu.printPQueue();
				alive--;
			}

			cpu.context = cs / 2+1; //time for the process to leave the cpu (add one cause it is instantly subtracted)
		}

		//process starts using the cpu
		if (cpu.current == NULL){
			//either get process from ready queue or do nothing
			if (cpu.context > 0) {
				cpu.context--;
			} else if (cpu.context == 0 && cpu.size() > 0) {
				//move new process in
				cpu.current = cpu.top();
				cpu.pop();
				
				//wait for context switch before announcing 
				//cpu.current->remaining = cpu.current->getCurrentCPUBurst() - 1;
				cpu.context = cs / 2 - 1;
			} 
		} else if (cpu.context > 0) {
			cpu.context--;
			//announce arrival of process
			if (cpu.context == 0){
				printTime(time);
				if (cpu.current->remaining == cpu.current->getCurrentCPUBurst() - 1){
					printf("Process %c (tau %dms) started using the CPU for %dms burst ", cpu.current->ID, cpu.current->tau, cpu.current->getCurrentCPUBurst());
				} else {
					printf("Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst ", cpu.current->ID, cpu.current->tau, cpu.current->remaining + 1, cpu.current->getCurrentCPUBurst());
				}
				cpu.printPQueue();

				if (check_preempt != -1){
					check_preempt = -1;
					preempt(cpu, cs, time, check_preempt, true);
				}
			}
		}

		//IOburst completions
		for (int i = 0; i < n; ++i){
			if (processes[i].inIO){
				if (processes[i].remaining == 0){
					processes[i].inIO = false;
					processes[i].step += 1;
					processes[i].remaining = processes[i].getCurrentCPUBurst() - 1;
					cpu.push(processes+i);

					//can preempt here
					//only preempts if there is a process in the CPU
					if (cpu.current != NULL && processes[i].tau < cpu.current->tau - (cpu.current->getCurrentCPUBurst() - cpu.current->remaining - 1)){
						if (cpu.context == 0){
							preempt(cpu, cs, time);
						} else {
							printTime(time);
							printf("Process %c (tau %dms) completed I/O; added to ready queue ", processes[i].ID, processes[i].tau);
							cpu.printPQueue();
							check_preempt = 1;
						}
					} else {
						printTime(time);
						printf("Process %c (tau %dms) completed I/O; added to ready queue ", processes[i].ID, processes[i].tau);
						cpu.printPQueue();
					}
				} else {
					processes[i].remaining--;
				}
			}
		}

		
		//new process arrivals
		for (int i = 0; i < n; ++i){
			if (processes[i].arrival == time){
				cpu.push(processes+i, tau_init);
				processes[i].inQueue = true;
				processes[i].remaining = processes[i].getCurrentCPUBurst() - 1;

				//can preempt here
				if (cpu.current != NULL && processes[i].tau < cpu.current->tau - (cpu.current->getCurrentCPUBurst() - cpu.current->remaining - 1)){
					if (cpu.context == 0){
						preempt(cpu, cs, time);
					} else {
						printTime(time);
						printf("Process %c (tau %dms) arrived; added to ready queue ", processes[i].ID, processes[i].tau);
						cpu.printPQueue();
						check_preempt = 0;
					}
				} else {
					printTime(time);
					printf("Process %c (tau %dms) arrived; added to ready queue ", processes[i].ID, tau_init);
					cpu.printPQueue();
				}
			}
		}

		++time;
		//printf("%d %d %p \n", time, cpu.context, cpu.current);
	}

	printTime(time + cs /2 - 1);
	printf("Simulator ended for SRT ");
	cpu.printQueue();
}

/***********************************************************/
void RR(Process* processes, int n, int cs, int slice, std::ofstream& file) {
	//start
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n;
	int contextSwitches = 0;
	double totalWait = 0;
	int waitCount = 0;
	double turnarounds = 0;
	int preemptions = 0;
	
	printTime(time);
	printf("Simulator started for RR with time slice %dms ", slice);
	cpu.printQueue();
	
	//loop
	while (alive > 0 || cpu.context > 0) {
		assert(cpu.current == NULL || cpu.switching == NULL);
		
		//CPU burst completion
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (p->inCPU) {
				p->CPUTime++;
				if ((p->CPUTime != 0 && p->CPUTime % slice == 0) || p->CPUTime == p->remaining) {//CPU time reaches a multiple of the slice or is done
					if (p->CPUTime == p->remaining || !cpu.empty()) { //if not a skipped preemption
						cpu.current = NULL;
						cpu.switching = p;
						cpu.context += cs/2;
						p->swap = false;
						p->inCPU = false;
					}
					
					if (p->CPUTime == p->remaining) { //CPU burst done
						p->preempt = false;
						
						if (p->step == int(p->CPUBursts.size()-1)) { //termination
							p->inQueue = false;
							p->inIO = false;
							alive--;
							
							printTime(time);
							printf("Process %c terminated ", p->ID);
							cpu.printQueue();
							continue;
						}
						
						//send to I/O
						int next = time + (p->IOBursts)[p->step] + (cs/2);
						p->nextArr = next;
						
						if ((p->CPUBursts.size())-(p->step)-1 == 1) {
							printTime(time);
							printf("Process %c completed a CPU burst; %ld burst to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
							cpu.printQueue();
						}
						else {
							printTime(time);
							printf("Process %c completed a CPU burst; %ld bursts to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
							cpu.printQueue();
						}
						printTime(time);
						printf("Process %c switching out of CPU; will block on I/O until time %dms ", p->ID, next);
						cpu.printQueue();
						continue;
					}
					else { //time slice ran out
						//printf("here: CPU time = %d, remaining = %d\n", p->CPUTime, p->remaining);
						p->remaining -= p->CPUTime;
						p->CPUTime = 0;
						if (cpu.empty()) { //no preemption
							p->preempt = false;
							printTime(time);
							printf("Time slice expired; no preemption because ready queue is empty ");
							cpu.printQueue();
						}
						else { //preempt here
							p->preempt = true;
							preemptions++;
						
							printTime(time);
							printf("Time slice expired; process %c preempted with %dms remaining ", p->ID, p->remaining);
							cpu.printQueue();
						}
					}
				}
				
			}
		}
		
		//start using CPU
		if (cpu.context == 0 && cpu.switching != NULL) {
			Process* a = cpu.switching;
			if (a->swap) { //switching into CPU
				a->inCPU = true;
				cpu.current = a;
				cpu.switching = NULL;
				contextSwitches++;
				
				printTime(time);
				if (a->remaining < (a->CPUBursts)[a->step]) {
					printf("Process %c started using the CPU for remaining %dms of %dms burst ", a->ID, a->remaining, a->CPUBursts[a->step]);
				}
				else {
					printf("Process %c started using the CPU for %dms burst ", a->ID, a->CPUBursts[a->step]);
				}
				cpu.printQueue();
			}
			else if (a->preempt) { //preemption
				a->inQueue = true;
				cpu.push_back(*a);
				cpu.switching = NULL;
			}
			else { //switching out to IO
				a->inIO = true;
				cpu.switching = NULL;
				a->turn = false;
			}
		}
		
		//IO burst completion
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (time == p->nextArr && p->inIO) {
				cpu.push_back(*p);
				p->inQueue = true;
				p->inIO = false;
				p->step++;
				printTime(time);
				printf("Process %c completed I/O; added to ready queue ", p->ID);
				cpu.printQueue();
				p->remaining = p->CPUBursts[p->step];
				p->turn = true;
			}
		}
		
		//new process arrivals
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (time == p->nextArr && !(p->inIO) && !(p->inCPU) && !(p->inQueue)) {
				cpu.push_back(*p);
				p->inQueue = true;
				printTime(time);
				printf("Process %c arrived; added to ready queue ", p->ID);
				cpu.printQueue();
				p->remaining = p->CPUBursts[p->step];
				p->turn = true;
			}
		}
		
		//process goes into CPU
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (p->inQueue) {
				if (cpu.current == NULL && cpu.switching == NULL && *p == cpu.front()) {
					p->inQueue = false;
					p->swap = true;
					cpu.switching = p;
					cpu.context += cs/2;
					p->CPUTime = 0;
					cpu.pop_front();
					
					totalWait += p->waitTime;
					waitCount++;
					p->waitTime = 0;
				}
				else {p->waitTime++;}
			}
		}
		
		if (cpu.context > 0) {
			cpu.context--;
		}
		
		//count turnaround time
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			if (p->turn) {
				p->turnaround++;
			}
			else {
				if (p->turnaround) {std::cout << "here: " << p->turnaround << std::endl;}
				turnarounds += p->turnaround;
				p->turnaround = 0;
			}
		}
		time++;
	}
	for (int i = 0; i < n; i++) {
		Process* p = &(processes[i]);
		turnarounds += p->turnaround;
		p->turnaround = 0;
	}
	
	//end
	printTime(time);
	printf("Simulator ended for RR ");
	cpu.printQueue();
	
	//output
	file << "Algorithm RR\n";
	file << "-- average CPU burst time: " << avgCPUBurstTime(processes, n) << " ms\n";
	file << "-- average wait time: ";
	if (totalWait == 0){
		file << "0.000 ms\n";
	} 
	else {
		char buffer[100];
		sprintf(buffer,"%.3lfms\n", totalWait / waitCount);
		file << buffer;
	}
	file << "-- average turnaround time: " << ceilTo3(turnarounds / totalBursts(processes, n)) << " ms\n";
	file << "-- total number of context switches: " << contextSwitches << "\n";
	file << "-- total number of preemptions: " << preemptions << "\n";
	file << "-- CPU utilization: " << CPUutil(processes, n, time) << "%\n";
}

int main(int argc, char** argv) {
	//error handling
	if (argc != 8){
		fprintf(stderr, "Insufficient Arguments, please ensure there are 7 arguments of the right format\n");
		return 1; 
	}
	
	//fetch args
	int n = 0, seed = 0, bound = 0, cs = 0, slice = 0;
	double lambda = 0, alpha = 0;
	fetch(argv, n, seed, lambda, bound, cs, alpha, slice);
	//printf("%d\n", seed);
	
	//validation error handling
	
	//open file
	std::ofstream file;
	file.open("simout.txt");

	
	//build processes
	Process* p = build(n, seed, lambda, bound);
	
	//display processes
	int tau_init = int(ceil(1/lambda));
	for (int i = 0; i < n; i++) {
		printf("Process %c: arrival time %dms; tau %dms; %ld CPU bursts:\n", p[i].ID, p[i].arrival, tau_init, p[i].CPUBursts.size());
		for (int j = 0; j < int(p[i].CPUBursts.size()); j++) {
			if (j != int(p[i].CPUBursts.size() - 1)) {
				printf("--> CPU burst %dms --> I/O burst %dms\n", p[i].CPUBursts[j], p[i].IOBursts[j]);
			}
			else {
				printf("--> CPU burst %dms\n", p[i].CPUBursts[j]);
			}
		}
	}
	
	//printf("\n");
	
	//do FCFS
	resetAll(p, n);
	FCFS(p, n, cs, file);
	printf("\n");
	
	//do SJF
	resetAll(p, n);
	SJF(p, n, cs, alpha, lambda, file);
	
	//printf("\n");
	
	//do SRT
	resetAll(p, n);
	SRT(p, n, cs, alpha, lambda);
	
	//printf("\n");
	
	//do RR
	resetAll(p, n);
	RR(p, n, cs, slice, file);
	
	//cleanup
	// for (int i = 0; i < n; i++) {
	// 	delete[] p[i].CPUBursts;
	// 	delete[] p[i].IOBursts;
	// }
	delete[] p;

	file.close();
}