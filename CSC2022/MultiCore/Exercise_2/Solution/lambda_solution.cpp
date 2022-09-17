/*
g++ -o lambda_solution -std=c++17 -pthread -g -Wall -Wextra -Wpedantic -Werror
lambda_solution.cpp
*/
#include <iostream>
#include <thread>

int main() {
  int counter{0};

  // create lambda to increment the counter
  auto incrementCounter = [&counter](const unsigned int times) {
    for (unsigned int i = 0; i < times; ++i) {
      ++counter;
    }
  };
  // launch a thread to increment the counter
  std::thread increment(incrementCounter, 100000);
  increment.join();
  std::cout << counter << "\n";

  return 0;
}
