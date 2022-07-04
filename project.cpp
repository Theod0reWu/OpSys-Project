#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

class Process {
	public:
		char ID;
		int waitTime = 0; //counts time in queue
		int IOTime = 0; //counts time in I/O
		int contextSwitches = 0; //counts # of context switches
		int arrival; //arrival time
		int noBursts; //# of bursts
		int* CPUBursts; //holds list of CPU burst times
		int* IOBursts; //holds list of I/O burst times
		int step = 0; //step is index of CPU or I/O burst list (reset when necesssary)
		bool place = false; //place = true if doing CPU things, false otherwise
		
		//reset all values (except burst data)
		void reset() {
			this->waitTime = 0;
			this->IOTime = 0;
			this->contextSwitches = 0;
			this->step = 0;
			this->place = false;
		}
};

class CPU {
	public:
		Process current; //currently running process
		Process* queue; //queue of processes
};

void fetch(char** args, int& n, int& seed, double& lambda, int& bound, int& cs, int& alpha, int& slice) {
	//use arguments as buffers
	n = atoi(args[1]);
	seed = atoi(args[2]);
	lambda = strtod(args[3], nullptr);
	bound = atoi(args[4]);
	cs = atoi(args[5]);
	alpha = atoi(args[6]);
	slice = atoi(args[7]);
}

//assumes seed is set
double next_exp(double lambda, int bound) {
	while (true) {
		double num = -log(drand48()) / lambda;
		if (num < bound) {return num;}
	}
}

Process* build(int n, int seed, double lambda, int bound) {
	char id = 'A';
	Process* processes = new Process[n];
	srand48(seed);
	for (int i = 0; i < n; i++) {
		Process p;
		
		p.ID = id;
		id++;
		
		p.arrival = int(floor(next_exp(lambda, bound)));
		
		int bursts = int(ceil(drand48()*100));
		p.noBursts = bursts;
		p.CPUBursts = new int[bursts];
		p.IOBursts = new int[bursts-1];
		for (int j = 0; j < bursts-1; j++) {
			p.CPUBursts[j] = ceil(next_exp(lambda, bound));
			p.IOBursts[j] = ceil(next_exp(lambda, bound))*10;
		}
		p.CPUBursts[bursts-1] = int(ceil(next_exp(lambda, bound)));
		
		processes[i] = p;
	}
	return processes;
}

//reset all process variables
void resetAll(Process* p, int n) {
	for (int i = 0; i < n; i++) {
		printf("here: %c\n", p[i].ID);
		p[i].reset();
	}
}

int main(int argc, char** argv) {
	//error handling
	
	//fetch args
	int n = 0, seed = 0, bound = 0, cs = 0, alpha = 0, slice = 0;
	double lambda = 0;
	fetch(argv, n, seed, lambda, bound, cs, alpha, slice);
	//printf("%d\n", seed);
	
	//validation error handling
	
	//build processes
	Process* p = build(n, seed, lambda, bound);
	
	//display processes
	int tau_init = int(ceil(1/lambda));
	for (int i = 0; i < n; i++) {
		printf("Process %c: arrival time %dms; tau %dms; %d CPU bursts:\n", p[i].ID, p[i].arrival, tau_init, p[i].noBursts);
		for (int j = 0; j < p[i].noBursts; j++) {
			if (j != p[i].noBursts - 1) {
				printf("--> CPU burst %dms --> I/O burst %dms\n", p[i].CPUBursts[j], p[i].IOBursts[j]);
			}
			else {
				printf("--> CPU burst %dms\n", p[i].CPUBursts[j]);
			}
		}
	}
	
	//do FCFS
	resetAll(p, n);
	
	//do SJF
	resetAll(p, n);
	
	//do SRT
	resetAll(p, n);
	
	//do RR
	resetAll(p, n);
	
	//output
	printf("hello\n");
	
	//cleanup
	for (int i = 0; i < n; i++) {
		delete[] p[i].CPUBursts;
		delete[] p[i].IOBursts;
	}
	delete[] p;
}