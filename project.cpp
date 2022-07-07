#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <queue>
#include "process.h"

class CPU {
	private:
		std::priority_queue<Process> queue; //queue of processes (wait state)
	public:
		Process* current = NULL; //currently running process
		int context = 0; //counts remaining time of context switch

	void push_back(Process &p){
		p.tau = 0;
		queue.push(p);
	}

	void push(Process &p, int priority){
		p.tau = priority;
		queue.push(p);
	}

	void pop_front(){
		return queue.pop();
	}

	const Process& top(){
		return queue.top();
	}

	void printQueue() {
		printf("[Q:");
		bool empty = true;
		std::priority_queue<Process> temp;
		while (queue.size() > 0){
			Process p = queue.top();
			printf(" %c", p.ID);
			empty = false;
			queue.pop();
			temp.push(p);
		}
		queue = temp;
		if (empty) {printf(" empty]\n");}
		else {printf("]\n");}
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

void FCFS(Process* processes, int n, int cs) {
	//initialize
	int time = 0;
	CPU cpu;
	cpu.context = 0;
	bool inSwitch = true; //true when CPU is ready to take another process
	int alive = n; //counter for how many processes are still alive
	
	//start
	printTime(time);
	printf("Simulator started for FCFS ");
	cpu.printQueue();
	
	//loop
	while (alive > 0 || cpu.context > 0) {
		//printf("context: %d\n", cpu.context);
		if (cpu.context > 0) {
			cpu.context--;
		}
		
		//process things
		for (int i = 0; i < n; i++) {
			//printf("context: %d\n", cpu.context);
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
				if (cpu.current == NULL && *p == cpu.top()) { //no current process, run next process in queue
					if (inSwitch) {
						cpu.context += cs/2;
						inSwitch = false;
					}
					//printf("context: %d\n", cpu.context);
					if (cpu.context == 0) {
						p->inQueue = false;
						cpu.current = p;
						cpu.pop_front();
						p->inCPU = true;
						p->CPUTime = 0;
						inSwitch = true;
						
						printTime(time);
						printf("Process %c started using the CPU for %dms burst ", p->ID, p->CPUBursts[p->step]);
						cpu.printQueue();
					}/*
					else {
						//printf("here\n");
						cpu.context--;
						continue;
					}*/
				}
				p->waitTime++;
			}
			
			//check CPU time
			if (p->inCPU) {
				if (p->CPUTime == p->CPUBursts[p->step]) { //CPU use done
					//printf("context: %d\n", cpu.context);
					if (p->step == p->CPUBursts.size()-1) {
						cpu.current = NULL;
						p->inCPU = false;
						p->inQueue = false;
						p->inIO = false;
						alive--;
						cpu.context += cs/2;
						printTime(time);
						printf("Process %c terminated ", p->ID);
						cpu.printQueue();
						continue;
					}
					
					if (cpu.context == 0) {
						cpu.current = NULL;
						p->inCPU = false;
						p->inIO = true;
						int next = time + (p->IOBursts)[p->step] + (cs/2);
						p->nextArr = next;
						cpu.context += cs/2;
					
						printTime(time);
						printf("Process %c completed a CPU burst; %ld bursts to go ", p->ID, (p->CPUBursts.size())-(p->step)-1);
						cpu.printQueue();
						printTime(time);
						printf("Process %c switching out of CPU; will block on I/O until time %dms ", p->ID, next);
						cpu.printQueue();
					}
					/*else {
						cpu.context--;
						continue;
					}*/
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

void SJF(Process * processes, int n, int cs, double alpha, double lambda){
	
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
	
	//display processes
	int tau_init = int(ceil(1/lambda));
	for (int i = 0; i < n; i++) {
		printf("Process %c: arrival time %dms; tau %dms; %ld CPU bursts:\n", p[i].ID, p[i].arrival, tau_init, p[i].CPUBursts.size());
		for (int j = 0; j < p[i].CPUBursts.size(); j++) {
			if (j != p[i].CPUBursts.size() - 1) {
				printf("--> CPU burst %dms --> I/O burst %dms\n", p[i].CPUBursts[j], p[i].IOBursts[j]);
			}
			else {
				printf("--> CPU burst %dms\n", p[i].CPUBursts[j]);
			}
		}
	}
	
	printf("\n");
	
	//do FCFS
	resetAll(p, n);
	FCFS(p, n, cs);
	
	//do SJF
	resetAll(p, n);
	
	//do SRT
	resetAll(p, n);
	
	//do RR
	resetAll(p, n);
	
	//cleanup
	// for (int i = 0; i < n; i++) {
	// 	delete[] p[i].CPUBursts;
	// 	delete[] p[i].IOBursts;
	// }
	delete[] p;
}