/*
g++ -o customersOpt customersOpt.cpp -std=c++14
*/
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

// This program *was badly designed and implemented on purpose*.
// The goal is to test performance profiling tools and understand
// some most common performance degradation patterns.

// The main issue linked to the performance of this program is the number of
// copies. We can mitigate it with:
// - Removing the vector (of vectors), using a deque instead
// - Making gCustomersAlphabetic a view of Customerv (use pointers)
// - Use emplace_back

class Customer {
public:
  Customer(unsigned int id, const std::string & name,
           const std::string & company, const std::string & city,
           const std::string & phone)
      : m_id(id), m_name(name), m_company(company), m_city(city),
        m_phone(phone) {
    nCostumerCtions += 1;
  };

  Customer(const Customer & obj)
      : m_id(obj.m_id), m_name(obj.m_name), m_company(obj.m_company),
        m_city(obj.m_city), m_phone(obj.m_phone) {
    nCostumerCopies++;
  };

  void Print() {
    std::cout << "Customer id : " << m_id << "\n"
              << " o name " << m_name << "\n"
              << " o company: " << m_company << "\n"
              << " o city: " << m_city << "\n"
              << " o phone: " << m_phone << "\n";
  }

  const char * getName() { return m_name.c_str(); };

  static void PrintCopyStats() {
    std::cout << "Constructor called " << nCostumerCtions << " times.\n"
              << "Copy c.ctor called " << nCostumerCopies << " times.\n";
  }

private:
  unsigned int m_id;
  std::string m_name;
  std::string m_company;
  std::string m_city;
  std::string m_phone;
  static unsigned int nCostumerCopies;
  static unsigned int nCostumerCtions;
};

unsigned int Customer::nCostumerCopies = 0;
unsigned int Customer::nCostumerCtions = 0;

using Customerv = std::deque<Customer>;
using CustomerPtrv = std::deque<Customer *>;
using CustomerPtrvv = std::deque<CustomerPtrv>;

Customerv gCustomers;
CustomerPtrvv gCustomersAlphabetic(26);

void fillCustomerData(const std::string & line,
                      std::vector<std::string> & data) {
  char buf[1000];
  strcpy(buf, line.c_str());
  char * p = strtok(buf, "|");
  int dataIndex = 0;
  while (p) {
    data[dataIndex++] = p;
    p = strtok(NULL, "|");
  }
}

int readCustomersData(const std::string filename) {

  std::ifstream iFile(filename);
  if (!iFile.is_open()) {
    std::cerr << "Error opening " << filename << "\n";
    return -1;
  }

  std::string line;
  std::vector<std::string> data(4);
  unsigned int id = 0;
  while (!iFile.eof()) {
    std::getline(iFile, line);
    fillCustomerData(line, data);
    gCustomers.emplace_back(id++, data[0], data[1], data[2], data[3]);
  }
  std::cout << "Customers data read in.\n";
  return 0;
}

void fillCustomerDataAlphabetic() {

  const char * name;
  const char offset = 65;
  for (auto & customer : gCustomers) {
    name = customer.getName();
    char initial = name[0];
    initial -= offset;
    gCustomersAlphabetic[(int)initial].emplace_back(&customer);
  }
}

int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cout << "Missing input file.\n"
              << "Usage: " << argv[0] << " inputData.txt\n";
    return 1;
  }

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  readCustomersData(argv[1]);
  fillCustomerDataAlphabetic();

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

  Customer::PrintCopyStats();
}
