/*
g++ -o example1 -std=c++17 -pthread -g -Wall -Wextra -Wpedantic -Werror
example1.cpp
*/
#include <atomic>
#include <iostream>
#include <thread>

void incrementCounter(std::atomic<int> * counter, const unsigned int times) {
  for (unsigned int i = 0; i < times; ++i) {
    ++(*counter);
  }
}

int main() {
  std::atomic<int> counter{0};
  // launch two threads to increment the counter
  std::thread increment1{incrementCounter, &counter, 100000};
  std::thread increment2{incrementCounter, &counter, 100000};
  increment1.join();
  increment2.join();
  std::cout << counter << "\n";
  return 0;
}
