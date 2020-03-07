#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <sstream>
#include <mutex>

// Build: g++ -g philosophers.cpp  -o philosophers.exe
// Execute: ./philosophers.exe

// Created deadlock with e.g. parameters 3, 20, 20

bool running = true;
std::vector<std::mutex*> forks;

void doPhilosopher(int index, int thinkingTime, int eatingTime) {
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

        // Take fork 1
        forks[firstSpoonIndex]->lock();
        
        msg << "Philosopher " << index << " took first fork." << std::endl; 
        std::cout << msg.str();
        msg.str(std::string());
        
        // Take fork 2
        forks[secondSpoonIndex]->lock();
        
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

    for(int i = 0; i < n; i++) {
        delete forks[i];
    }

    return 0;
}