#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
using namespace std;

class ThreadPool {
public:
ThreadPool(size_t num_threads);
~ThreadPool();
void enqueue(function<void()> task);
void stopAndWait();
};