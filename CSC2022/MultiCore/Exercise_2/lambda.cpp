/*
g++ -o lambda -std=c++17 -pthread -g -Wall -Wextra -Wpedantic -Werror lambda.cpp
*/
#include <iostream>
#include <thread>

void incrementCounter(int * counter, const unsigned int times) {
  for (unsigned int i = 0; i < times; ++i) {
    ++(*counter);
  }
}

int main() {
  int counter{0};
  // launch a thread to increment the counter
  std::thread increment(incrementCounter, &counter, 100000);
  increment.join();
  std::cout << counter << "\n";

  return 0;
}
