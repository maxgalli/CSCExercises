/*
g++ -o example3 -std=c++17 -pthread -g -Wall -Wextra -Wpedantic -Werror
example3.cpp
*/

#include <iostream>
#include <thread>
#include <unistd.h>

void incrementCounter(int * counter, const unsigned int times) {
  for (unsigned int i = 0; i < times; ++i) {
    usleep(100);
    ++(*counter);
  }
}

void anotherIncrementCounter(int * c, const unsigned int times) {
  for (unsigned int i = 0; i < times; ++i) {
    usleep(100);
    ++(*c);
  }
}

int main() {
  int counter{0};
  // launch two threads to increment the counter
  std::thread increment1{incrementCounter, &counter, 100000};
  std::thread increment2{anotherIncrementCounter, &counter, 1000000000};
  increment1.join();
  increment2.join();
  std::cout << "Expected value:   1000100000" << "\n";
  std::cout << "Calculated value: " << counter << "\n";
  return 0;
}
