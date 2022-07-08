#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <queue>
#include <deque>
#include "process.h"

class CPU {
	private:
		std::deque<Process> queue; //queue of processes
		std::priority_queue<Process> pqueue; //priority queue of processes (wait state)
	public:
		Process* current = NULL; //currently running process
		Process* switching = NULL; //process currently being swapped in/out
		int context = 0; //counts remaining time of context switch

	void push_back(Process &p){
		p.tau = 0;
		queue.push_back(p);
	}

	void push(Process &p, int priority){
		p.tau = priority;
		pqueue.push(p);
	}

	void pop_front(){
		return queue.pop_front();
	}
	
	const Process& front() {
		return queue.front();
	}
	
	const Process& top(){
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
		std::priority_queue<Process> temp;
		while (pqueue.size() > 0){
			Process p = pqueue.top();
			printf(" %c", p.ID);
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
};

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
void FCFS(Process* processes, int n, int cs) {
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n; //counter for how many processes are still alive
	
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
			}
			else { //switching out of CPU
				a->inIO = true;
				cpu.switching = NULL;
			}
		}
		
		//process things
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			
			//check arrival time / I/O time
			if (time == p->nextArr) {
				cpu.push_back(*p);
				p->inQueue = true;
				
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
				}
				p->waitTime++;
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
					
					printTime(time);
					printf("Process %c completed a CPU burst; %ld bursts to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
					cpu.printQueue();
					printTime(time);
					printf("Process %c switching out of CPU; will block on I/O until time %dms ", p->ID, next);
					cpu.printQueue();
				}
				p->CPUTime++;
			}
		}
		
		time++;
	}
	time--;
	
	//end
	printTime(time);
	printf("Simulator ended for FCFS ");
	cpu.printQueue();
}

/***********************************************************/
void SJF(Process * processes, int n, int cs, double alpha, double lambda){
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	//int alive = n; //counter for how many processes are still alive

	printTime(time);
	printf("Simulator started for SJF ");
	cpu.printQueue();

	
}

/***********************************************************/


/***********************************************************/
void RR(Process* processes, int n, int cs, int slice) {
	//start
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	int alive = n;
	
	printTime(time);
	printf("Simulator started for RR with time slice %dms ", slice);
	cpu.printQueue();
	
	//loop
	while (alive > 0 || cpu.context > 0) {
		assert(cpu.current == NULL || cpu.switching == NULL);
		
		if (cpu.context > 0) {
			cpu.context--;
		}
		if (cpu.context == 0 && cpu.switching != NULL) {
			Process* a = cpu.switching;
			if (a->swap) { //switching into CPU
				a->inCPU = true;
				cpu.current = a;
				cpu.switching = NULL;
				
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
			}
		}
		
		for (int i = 0; i < n; i++) {
			Process* p = &(processes[i]);
			
			if (time == p->nextArr) {
				cpu.push_back(*p);
				p->inQueue = true;
				
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
				
				p->remaining = p->CPUBursts[p->step]; //first time in queue; remaining time is full time
			}
			
			if (p->inQueue) {
				if (cpu.current == NULL && cpu.switching == NULL && *p == cpu.front()) {
					p->inQueue = false;
					p->swap = true;
					cpu.switching = p;
					cpu.context += cs/2;
					p->CPUTime = 0;
					cpu.pop_front();
				}
				p->waitTime++;
			}
			
			if (p->inCPU) {
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
					
						printTime(time);
						printf("Process %c completed a CPU burst; %ld bursts to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
						cpu.printQueue();
						printTime(time);
						printf("Process %c switching out of CPU; will block on I/O until time %dms ", p->ID, next);
						cpu.printQueue();
						continue;
					}
					else { //time slice ran out
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
						
							printTime(time);
							printf("Time slice expired; process %c preempted with %dms remaining ", p->ID, p->remaining);
							cpu.printQueue();
						}
					}
				}
				p->CPUTime++;
			}
		}
		
		time++;
	}
	time--;
	
	//end
	printTime(time);
	printf("Simulator ended for RR ");
	cpu.printQueue();
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
	
	
	//build processes
	Process* p = build(n, seed, lambda, bound);
	/*
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
	*/
	//printf("\n");
	
	//do FCFS
	resetAll(p, n);
	//FCFS(p, n, cs);
	//printf("\n");
	
	//do SJF
	resetAll(p, n);
	
	//printf("\n");
	
	//do SRT
	resetAll(p, n);
	
	//printf("\n");
	
	//do RR
	resetAll(p, n);
	RR(p, n, cs, slice);
	
	//cleanup
	// for (int i = 0; i < n; i++) {
	// 	delete[] p[i].CPUBursts;
	// 	delete[] p[i].IOBursts;
	// }
	delete[] p;
}