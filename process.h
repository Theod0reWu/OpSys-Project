class Process {
	public:
		char ID;
		int waitTime = 0; //counts time in queue
		int CPUTime = 0; //counts time in CPU
		int arrival; //arrival time
		int nextArr; //next arrival time
		std::vector<int> CPUBursts;
		std::vector<int> IOBursts;

		int step = 0; //step is index of CPU or I/O burst list (reset when necesssary)
		bool inCPU = false; //true if doing CPU things, false otherwise
		bool inQueue = false; //true if in queue, false otherwise
		bool inIO = false; //true if in I/O, false otherwise

		int tau;
		int remaining; //counts remaining time on current CPU burst
		bool swap = true; //true if swapping into CPU, false if leaving CPU
		bool preempt = false; //true if current swap is a preemption
		
		bool turn = false; //true when 
		int turnaround = 0;

		Process();
		Process(char ID, int arrival, int tau = 0);
		bool operator<(const Process& b) const;
		bool operator>(const Process& b) const;

		bool lessThan(const Process& b) const;
		void reset();
		bool operator==(Process const& other) const;

		int getCurrentCPUBurst() const;
		int getCurrentIOBurst() const;
};

class Compare {
public:
	bool operator()(Process* a, Process* b){
		return (*a) < (*b);
	}
};

class CompareAgain {
public:
	bool operator()(Process* a, Process* b){
		return a->lessThan(*b);
	}
};

double next_exp(double lambda, int bound);
Process* build(int n, int seed, double lambda, int bound);
