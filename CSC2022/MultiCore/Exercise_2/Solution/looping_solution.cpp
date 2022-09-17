/*
g++ -o looping_solution -std=c++17 -pthread -g -Wall -Wextra -Wpedantic -Werror
looping_solution.cpp
*/
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

class Tool {
public:
  Tool() : aMember{0} {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  Tool(const Tool &) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  void print() const { std::cout << "tool: " << aMember << "\n"; };

private:
  int aMember;
};

void indexLoop(const std::vector<Tool> & tools) {
  for (unsigned int i = 0, size = tools.size(); i < size; ++i) {
    tools[i].print();
  }
}

void iteratorLoop(const std::vector<Tool> & tools) {
  for (auto tool = tools.begin(); tool != tools.end(); ++tool) {
    tool->print();
  }
}

void rangeLoop(const std::vector<Tool> & tools) {
  for (const auto & tool : tools) {
    tool.print();
  }
}

int main() {
  // create a vector of 5 Tools
  std::cout << "Creating tools" << "\n";
  std::vector<Tool> tools(5);

  std::cout << "Start index looping" << "\n";
  indexLoop(tools);
  std::cout << "Start iterator looping" << "\n";
  iteratorLoop(tools);
  std::cout << "Start range looping" << "\n";
  rangeLoop(tools);

  return 0;
}
