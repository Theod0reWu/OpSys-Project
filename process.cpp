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