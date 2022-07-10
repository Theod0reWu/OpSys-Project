#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <queue>
#include <vector>
#include "process.h"

Process::Process(){
	tau = 0;
}

Process::Process(char ID, int arrival, int tau){
	this->ID = ID;
	this->tau = tau;
	this->arrival = arrival;
	this->nextArr = arrival;
}

bool Process::operator<(const Process& b) const {
	return this->tau < b.tau;
}

//reset all values (except burst data)
void Process::reset() {
	this->waitTime = 0;
	this->CPUTime = 0;
	this->IOTime = 0;
	this->nextArr = this->arrival;
	this->step = 0;
	this->inCPU = false;
	this->inQueue = false;
	this->inIO = false;
}

bool Process::operator==(Process const& other) const {
	return this->ID == other.ID;
}

int Process::getCurrentCPUBurst() const {
	return CPUBursts[step];
}

int Process::getCurrentIOBurst() const {
	return IOBursts[step];
}

//assumes seed is set
double next_exp(double lambda, int bound) {
	while (true) {
		double num = -log(drand48()) / lambda;
		if (num < bound) {return num;}
	}
}

//builds all the process for the seed
Process* build(int n, int seed, double lambda, int bound) {
	char id = 'A';
	Process* processes = new Process[n];
	srand48(seed);
	for (int i = 0; i < n; i++) {
		Process p(id, int(floor(next_exp(lambda, bound))));
		id++;
		
		int bursts = int(ceil(drand48()*100));
		for (int j = 0; j < bursts-1; j++) {
			p.CPUBursts.push_back(ceil(next_exp(lambda, bound)));
			p.IOBursts.push_back(ceil(next_exp(lambda, bound))*10);
		}
		p.CPUBursts.push_back(int(ceil(next_exp(lambda, bound))));
		
		processes[i] = p;
	}
	return processes;
}