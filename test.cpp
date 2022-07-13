#include <iostream>
#include <cmath>
#include <queue>
#include <vector>
#include "process.h"

void printPQueue(std::priority_queue<Process*, std::vector<Process*>, Compare>& pqueue) {
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

int main(int argc, char const *argv[])
{
	std::priority_queue<Process*, std::vector<Process*>, Compare> pqueue;
	for (int i = 0; i < 4; ++i){
		Process *p = new Process('A' + i, 100);
		p->CPUBursts.push_back(100);
		pqueue.push(p);
	}
	printPQueue(pqueue);
	return 0;
}