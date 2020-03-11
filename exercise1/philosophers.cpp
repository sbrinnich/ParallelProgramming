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

std::mutex* mutexTimeSpentLocks = new std::mutex();  
std::mutex* mutexTimeSpentWorkFlow = new std::mutex(); 

std::vector<long long>* timeSpentLocks = new std::vector<long long>();
long long timeThreadsExecuted = 0;  

void addTimeSpentLocks(std::vector<long long>* timeSpent){
    mutexTimeSpentLocks->lock();
    for(auto const& value: *timeSpent) {
        timeSpentLocks->push_back(value);
    }
    mutexTimeSpentLocks->unlock();
}

void addTimeThreadExecuted(long long duration){
    mutexTimeSpentWorkFlow->lock(); 
    timeThreadsExecuted += duration;
    mutexTimeSpentWorkFlow->unlock(); 
}

void doPhilosopher(int index, int thinkingTime, int eatingTime) {
    auto timeThreadStart = std::chrono::high_resolution_clock::now(); 
    std::vector<long long>* waitingTime = new std::vector<long long>();
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

        waitingTime->push_back(std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLock - timeBeforeLock).count());

        msg << "Philosopher " << index << " took first fork." << std::endl; 
        std::cout << msg.str();
        msg.str(std::string());
        
        timeBeforeLock = std::chrono::high_resolution_clock::now();  

        // Take fork 2
        forks[secondSpoonIndex]->lock();

        timeAfterLock = std::chrono::high_resolution_clock::now();  

        waitingTime->push_back(std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLock - timeBeforeLock).count());
        
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
    addTimeSpentLocks(waitingTime);
    delete waitingTime;
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

    auto avarageWait = sumWait/timeSpentLocks->size();

    std::cout << "The highest waiting time for a locked fork: " << maxWait << " ms" << std::endl;
    std::cout << "The average waiting time for a fork: " << avarageWait << " ms" << std::endl;
    std::cout << "The total waiting time for all threads: " << sumWait << " ms" << std::endl;
    std::cout << "The total execution time for all threads: " << timeThreadsExecuted << " ms" << std::endl;
    
    std::cout << "The percentage of waiting time of the total execution time over all threads: " << (float)sumWait/timeThreadsExecuted << std::endl; 
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

    // Output statistics of run
    doStatistics(); 
    
    // Clear memory
    for(int i = 0; i < n; i++) {
        delete forks[i];
    }

    delete mutexTimeSpentLocks;
    delete mutexTimeSpentWorkFlow;
    delete timeSpentLocks;

    return 0;
}