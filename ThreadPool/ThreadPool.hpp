#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

template<typename dataStruct> class  ThreadPool{
private:
	int numberOfThreads;
	int freeThreads;
	std::vector<std::thread> threads;
	std::mutex resourcesMutex;
	std::condition_variable newJobCv; //Not the cv that we send to a university:P
	bool timeToDie;
	std::queue<dataStruct> resourcesForEachThread;

public:
	ThreadPool(int numOfThreads, void (*func)(dataStruct)):numberOfThreads(numOfThreads),timeToDie(false),freeThreads(0){
		threads.resize(numberOfThreads);
		for(int i=0; i<numOfThreads; i++){
			threads[i] = std::thread(&ThreadPool::genericThreadFunc, this, func);
		}
	};

	~ThreadPool(){
		{
    		std::unique_lock<std::mutex> mutexLock(resourcesMutex);
    		timeToDie = true;
    		newJobCv.notify_one();
		}
		for(unsigned int i=0; i<threads.size(); i++){
			threads[i].join();
		}

	};

	void testAwakeNThreads(int n){
		for(int i=0; i<n; i++){
    		useNewThread(i);	
		}
	}

	void useNewThread(dataStruct data){
		{
    		std::unique_lock<std::mutex> mutexLock(resourcesMutex);
    		resourcesForEachThread.push(data);
    		newJobCv.notify_all();
		};
	}

	void genericThreadFunc(void (*f)(dataStruct)){
		while(true){
			dataStruct dataForFunc;
	    	{
	    		std::unique_lock<std::mutex> mutexLock(resourcesMutex);
	    		freeThreads++;
	    		if(resourcesForEachThread.empty()){
		 			if(newJobCv.wait_for(mutexLock, std::chrono::milliseconds(100)) == std::cv_status::timeout){
						if(timeToDie == true){
							std::cout << "Thread terminating: " << std::this_thread::get_id() << std::endl;
							break;
						}else{
							continue;
						}
					}else if(resourcesForEachThread.empty()){
						continue;
					}
				}
				freeThreads--;
				dataForFunc = resourcesForEachThread.front();
				resourcesForEachThread.pop();
			}
			std::cout << "new job arrived to: " << std::this_thread::get_id() << std::endl;
			f(dataForFunc);
		}
	}
};
#endif //THREADPOOL_HPP_