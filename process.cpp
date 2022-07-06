class Process {
	public:
		char ID;
		int waitTime = 0; //counts time in queue
		int IOTime = 0; //counts time in I/O
		int arrival; //arrival time
		int noBursts; //# of bursts
		int* CPUBursts; //holds list of CPU burst times
		int* IOBursts; //holds list of I/O burst times
		int step = 0; //step is index of CPU or I/O burst list (reset when necesssary)
		bool inCPU = false; //true if doing CPU things, false otherwise
		bool inQueue = false; //true if in queue, false otherwise
		bool inIO = false; //true if in I/O, false otherwise
		
		//reset all values (except burst data)
		void reset() {
			this->waitTime = 0;
			this->IOTime = 0;
			this->step = 0;
			this->inCPU = false;
			this->inQueue = false;
			this->inIO = false;
		}
};