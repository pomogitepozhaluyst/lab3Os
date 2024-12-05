#include <iostream>
#include<time.h>
#include "my_thread.hpp"
#include "my_shmem.hpp"
#include "lab2.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#endif


using namespace cplib;
#define MEMO_TEXT "Init memo"
int copyId = 0;
bool t1 = false;

std::string nameProg;
struct my_data
{
	my_data() {
		memset(memo,0,sizeof(memo));
        memcpy(memo,MEMO_TEXT,strlen(MEMO_TEXT));
		counter= 0 ;
		pid = -999;
	}
	void printf(FILE* file) {
		fprintf(file, "counter: %d\n\n", counter);
		fflush(file);
	}

	char        memo[256];
	int         counter= 0;
	double      pid = -999;
};

int getPid(){
#ifdef WIN32
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

class Time{
	public:

#ifdef WIN32

	long getTime(){
		union{
			long long ns100;
			FILETIME ft;
		} now;
		GetSystemTimeAsFileTime(&(now.ft));
		return (long)((now.ns100 / 10LL));
	}

	std::string getLocalTime(){
		SYSTEMTIME st;
 	GetLocalTime(&st);
		return (std::to_string(st.wYear) + " " + std::to_string(st.wMonth) + " " + std::to_string(st.wDay) + " " + std::to_string(st.wHour) + ":" + std::to_string(st.wMinute) + ":" + std::to_string(st.wSecond) + ":" + std::to_string(st.wMilliseconds) + "\n").data(); 
	}
#else
	unsigned long getTime(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return 1000000 * tv.tv_sec + tv.tv_usec;
	}

	std::string getLocalTime(){
  char buffer[26];
  int millisec;
  struct tm* tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); 
  if (millisec>=1000) { 
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);

  strftime(buffer, 26, "%Y %m %d %H:%M:%S", tm_info);
		std::string tmp1223 = (std::string)buffer+":"+std::to_string(millisec) + "\n";
		return tmp1223;
	}
#endif


};

class Thread2 : public Thread
{
public:
	Thread2(){}
	virtual int MainStart() {
		return 0;
	}
	virtual void MainQuit() {
	}
	virtual void Main() {
		createProgramm(nameProg+" 1");
	}
	int pid;
};

class Thread3 : public Thread
{
public:
	Thread3(){}
	virtual int MainStart() {
		return 0;
	}
	virtual void MainQuit() {
	}
	virtual void Main() {
		createProgramm(nameProg+" 2");
	}
};


class Thread1 : public Thread
{
public:
	Thread1(){}
	virtual int MainStart() {
 	file = fopen("log.txt", "a");
		return 0;
	}
	virtual void MainQuit() {
 	fclose(file);
	 th2.Stop();
	 th3.Stop();
	 th2.Join();
 	th3.Join();
	}
	virtual void Main() {

		long oldTimeForInc = t.getTime();
		long oldTimeForLocalTime = t.getTime();
		long oldTimeForCreateProgramm1 = t.getTime();
		long oldTimeForCreateProgramm2 = t.getTime();
		long oldTimeForWaitProgramm2 = t.getTime();
		bool isMultiplyx2 = true;

		SharedMem<my_data> shared_data("mymemo");

		while(true){
			CancelPoint();

			if (!copyId && shared_data.Data()->pid == -999){
				shared_data.Lock();
				shared_data.Data()->pid = getPid();
				shared_data.Unlock();
			}


			if ((t.getTime() - oldTimeForLocalTime >= 1000000L) && (shared_data.Data()->pid == getPid())){
				fprintf(file, "pid: %d\n", getPid());
				fprintf(file, "time: %s", t.getLocalTime().c_str());
				fflush(file);
				oldTimeForLocalTime = t.getTime();
				shared_data.Data()->printf(file);
			}
			
			if (t.getTime() - oldTimeForInc >= 300000){

				if (!copyId){
					shared_data.Lock();
					shared_data.Data()->counter++;
					shared_data.Unlock();
				}
				else if (copyId == 1){
					shared_data.Lock();
					shared_data.Data()->counter += 10;
					shared_data.Unlock();
					break;
				}
				else if (copyId == 2){
					if (isMultiplyx2){
						shared_data.Lock();
						shared_data.Data()->counter *= 2;
						shared_data.Unlock();
						isMultiplyx2 = false;
					}
					if (t.getTime() - oldTimeForWaitProgramm2 >= 2000000L){
						shared_data.Lock();
						shared_data.Data()->counter /= 2;
						shared_data.Unlock();
						break;		
					}
				}
				oldTimeForInc = t.getTime();
			}
			if (shared_data.Data()->pid == getPid()){
				if (th2.ThreadState() == STATE_STOPPED && (t.getTime() - oldTimeForCreateProgramm1 >= 3000000L)){
						th2.Start();
						th2.WaitStartup();
						oldTimeForCreateProgramm1 = t.getTime();
				}
				else if (th2.ThreadState() == STATE_RUNNING && (t.getTime() - oldTimeForCreateProgramm1 >= 3000000L)){
					fprintf(file, "copy #1 still work\n\n");
					fflush(file);
				}
				if (th3.ThreadState() == STATE_STOPPED && (t.getTime() - oldTimeForCreateProgramm2 >= 3000000L)){
						th3.Start();
						th3.WaitStartup();
						oldTimeForCreateProgramm2 = t.getTime();
				}
				else if (th3.ThreadState() == STATE_RUNNING && (t.getTime() - oldTimeForCreateProgramm2 >= 3000000L)){
					fprintf(file, "copy #2 still work\n\n");
					fflush(file);
				}
			}
 	}
		t1 = true;
	CancelPoint();

	}
private:
	Time t;
	FILE* file;
	Thread2 th2;
	Thread3 th3;
};

int main(int argc, char** argv){
	nameProg = argv[0];
	FILE* file = fopen("log.txt", "a");
	Time t;
	if (argc == 1){
		copyId = 0;
	}
	else{
		copyId = atoi(argv[1]);
	}

	SharedMem<my_data> shared_data("mymemo");

	#ifdef WIN32
		if (shared_data.Data()->pid == -999){
			shared_data.Lock();
			shared_data.Data()->pid = getPid();
			shared_data.Unlock();
		}
	#else	
		if (kill(shared_data.Data()->pid, 0) != 0) {
			shared_data.Lock();
			shared_data.Data()->pid = getPid();
			shared_data.Data()->counter = 0;
			shared_data.Unlock();
		}
	#endif

		if ((shared_data.Data()->pid == getPid()) || copyId){
			fprintf(file, "pid: %d\ncopy #%d start.\ntime start: %s\n", getPid(), copyId, t.getLocalTime().c_str() );
			fflush(file);
		}

	Thread1 th1;
	
	th1.Start();
	th1.WaitStartup();



 int inc = 0; 

	if (!copyId){
		std::cout << "! Possible commands: i | e | q" << std::endl;
		std::string cmd;
		for (;;) {
			std::cout << "Enter command: ";
			std::cin >> cmd;

			if (cmd == "i" || cmd == "int") {
				std::cin >> inc;
				shared_data.Lock();
				shared_data.Data()->counter = inc;
				shared_data.Unlock();
			} 
			else if (cmd == "e" || cmd == "q" || cmd == "exit" || cmd == "quite") {

				t1 = true;

				break;
			}
			else{
				std::cout << "! Possible commands: i | e | q" << std::endl;
			}
		}
	}

	while(!t1){

	}

 th1.Stop();
 th1.Join();

	if ((shared_data.Data()->pid == getPid()) || copyId){
			fprintf(file, "pid: %d\ncopy #%d exit.\ntime exit: %s\n",getPid(), copyId, t.getLocalTime().c_str());
			fflush(file);
	}

	fclose(file);
	if (!copyId && shared_data.Data()->pid == getPid()){
		shared_data.Lock();
		shared_data.Data()->pid = -999;
		shared_data.Unlock();
	}
	return 0;
}