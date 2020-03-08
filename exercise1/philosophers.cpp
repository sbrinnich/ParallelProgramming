#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <sstream>
#include <mutex>
#include <map>

// Build: g++ -g philosophers.cpp  -o philosophers.exe
// Execute: ./philosophers.exe

// Created deadlock with e.g. parameters 3, 20, 20

bool running = true;
std::vector<std::mutex*> forks;

auto mutexTimeSpentLocks = new std::mutex();  
auto mutexTimeSpentWorkFlow = new std::mutex(); 

auto timeSpentLocks = new std::vector<long long>();
auto timeThreadsExecuted = new std::vector<long long>();  

void addTimeSpentLocks(long long timeSpent){
    mutexTimeSpentLocks->lock();
    timeSpentLocks->push_back(timeSpent); 
    mutexTimeSpentLocks->unlock();
}

void addTimeThreadExecuted(long long duration){
    mutexTimeSpentWorkFlow->lock(); 
    timeThreadsExecuted->push_back(duration);
    mutexTimeSpentWorkFlow->unlock(); 
}

void doPhilosopher(int index, int thinkingTime, int eatingTime) {
    auto timeThreadStart = std::chrono::high_resolution_clock::now(); 
    srand( (unsigned)time(NULL) + index);
    std::stringstream msg;

    int firstSpoonIndex = (index % 2 == 0) ? ((index + 1) % forks.size()) : index;
    int secondSpoonIndex = (index % 2 == 0) ? index : ((index + 1) % forks.size());

    while(running) {
        // Think
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % thinkingTime + 1));
        msg << "Philosopher " << index << " finished thinking." << std::endl; 
        std::cout << msg.str();
        msg.str(std::string());

        //measure time around the lock to get statistics about waiting time of the thread 
        auto timeBeforeLock = std::chrono::high_resolution_clock::now(); 
        
        // Take fork 1
        forks[firstSpoonIndex]->lock();

        auto timeAfterLock = std::chrono::high_resolution_clock::now();  

        auto durationWait = std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLock - timeBeforeLock).count();

        if(durationWait != 0){
            addTimeSpentLocks(durationWait);
        }

        msg << "Philosopher " << index << " took first fork." << std::endl; 
        std::cout << msg.str();
        msg.str(std::string());
        
        timeBeforeLock = std::chrono::high_resolution_clock::now();  

        // Take fork 2
        forks[secondSpoonIndex]->lock();

        timeAfterLock = std::chrono::high_resolution_clock::now();  

        durationWait = std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLock - timeBeforeLock).count();

        if(durationWait != 0){
            addTimeSpentLocks(durationWait);
        } 
        
        msg << "Philosopher " << index << " took second fork." << std::endl; 
        std::cout << msg.str();
        msg.str(std::string());

        // Eat
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % eatingTime + 1));
        msg << "Philosopher " << index << " finished eating." << std::endl;
        std::cout << msg.str();
        msg.str(std::string());

        // Put forks down
        forks[secondSpoonIndex]->unlock();
        forks[firstSpoonIndex]->unlock();
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeThreadStart).count();
    addTimeThreadExecuted(duration);
}

void doStatistics(){
    long long maxWait = 0; 
    long long sumWait = 0; 
    for(std::size_t i = 0; i < timeSpentLocks->size(); i++){
        long long duration = timeSpentLocks->back();
        maxWait = (duration > maxWait) ? duration : maxWait; 
        sumWait += duration; 
        timeSpentLocks->pop_back(); 
    }

    auto avarageWaitTime = sumWait/timeSpentLocks->size(); 

    long long sumDuration = 0; 
    for(std::size_t i = 0; i < timeThreadsExecuted->size(); i++){
        long long duration = timeThreadsExecuted->back();
        sumDuration += duration; 
    }

    std::cout << "The highest waiting time for an locked fork was: " << maxWait << " ms" << std::endl; 
    std::cout << "The avarage waiting tiem over all threads and waiting blocks: " << avarageWaitTime << " ms" << std::endl; 
    std::cout << "The total waiting time for all threads: " << sumWait << " ms" << std::endl;
    std::cout << "The total execution time for all threads: " << sumDuration << " ms" << std::endl;
    
    std::cout << "The ratio of waiting time to execution time over all threads: " << (float)sumWait/sumDuration << std::endl; 
}

int main() {
    std::cout << "Welcome to Dining Philosophers!\n";

    // Read inputs
    int n;
    int thinkingTime;
    int eatingTime;

    std::cout << "Please input the number of philosophers at the table: ";
    std::cin >> n;
    std::cout << "Please input the maximal thinking time of the philosophers: ";
    std::cin >> thinkingTime;
    std::cout << "Please input the maximal eating time of the philosophers: ";
    std::cin >> eatingTime;

    std::vector<std::thread> philosophers;

    // Create forks
    forks = std::vector<std::mutex*>(n);
    for(int i = 0; i < n; i++) {
        forks[i] = new std::mutex();
    }

    // Create philosophers
    for(int i = 0; i < n; i++) {
        std::cout << "Creating philosopher " << i << std::endl;
        philosophers.emplace(philosophers.begin(), std::thread(doPhilosopher, i, thinkingTime, eatingTime));
    }

    // Wait for input to shutdown
    char input;
    std::cin >> input;

    running = false;

    std::cout << "Shutting down..." << std::endl;;

    for (std::thread & philosopher : philosophers) {
	    if (philosopher.joinable()) philosopher.join();
    }

    doStatistics(); 
    
    for(int i = 0; i < n; i++) {
        delete forks[i];
    }

    return 0;
}