/* Example program that introduces to task based parallelism
g++ hard.cpp -o hard -std=c++17 -fgnu-tm -pthread
*/
// The headers suggest what pieces you may need :)
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

//------------------------------------------------------------------------------
// A possible implementation of a bounded thread safe queue using tm

// The methods should be implemented. Choose the technology you prefer to make
// the queue thread safe

template <class T> class TsStack {
public:
  TsStack(size_t queueSize){};
  //---------
  bool isFull(){};           // Must be thread safe
                             //--------
  void push(const T & item); // Must be thread safe
                             //---------
  void pop(T & item);        // Must be thread safe

private:
  std::vector<T> m_items;
};

// Some useful globals
using Action = std::function<void()>;
using TsActionPtrQueue = TsStack<Action *>;
TsActionPtrQueue * gActionsQueue = new TsActionPtrQueue(1000);
TsActionPtrQueue * gMsgQueue = new TsActionPtrQueue(100);

//------------------------------------------------------------------------------
// Print messages in a thread safe way
// Find a solution for this functionality

//------------------------------------------------------------------------------
// A dummy function which just spends time crunching CPU

void doWork(float deltaTf) {
  // Implement this number crunching function using the functionalities in
  // std::chrono
}

//------------------------------------------------------------------------------
// Small dummy class representing a mail item. It has an Id and a state
// Finish the class
class mailItem {
public:
  enum class State : char {
    kStart,
    kFolded,
    kStuffed,
    kSealed,
    kAddressed,
    kStamped,
    kMailed
  };

  mailItem();

  mailItem(size_t id);

  size_t getId();

  State getState();

  void setState(State state);

private:
  size_t m_id;
  State m_state;
};

TsStack<mailItem> * gSentMailItemsQueue;
//------------------------------------------------------------------------------
// Mini functions that represent the actions applicable to a mail item

// The function that brings the state of the item to kMailed, puts the mail item
// in a thread safe queue of finished items. You can monitor that to understand
// when there is no more work to do and join the worker threads.

// The implementation of the functions to evolve between intermediate states
// is missing. Note that each function should push into the work queue the
// work item corresponding to the evolution to the next state.

//------------------------------------------------------------------------------
// Pull work items from a work queue and stop when necessary
// It can sleep.
void pullWork(TsActionPtrQueue * workQueue, std::function<bool()> stopCondition,
              bool sleep) {

  Action * action = nullptr;
  // while you obtain a work item from the queue OR there is no more work to do
  // Execute the work item
  // If requested, take a break sleeping for 10 milliseconds
}

//------------------------------------------------------------------------------

std::vector<size_t> gCompletedMailItemIDs;

int main(int argc, char ** argv) {

  // Get the arguments from command line and notify start
  // Parse args
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <mail items> <n working threads>\n";
    return 1;
  }

  int nItems = std::stoi(argv[1]);
  int nThreads = std::stoi(argv[2]);

  if (nItems * nThreads == 0) {
    std::cerr << "Invalid input parameter(s) value(s)\n";
    return 1;
  }

  // create queue for the sent mail items. It should be thread safe

  std::cout << "Starting with " << nItems << " items and " << nThreads
            << " threads\n";
  //-------------------------------------------------------

  // Here the real orchestration starts!

  // We need a common signal when to stop
  bool workEnded = false;

  // Condition to stop pulling work from queue
  auto stopPullingWork([] {
    return /*How can you know that all mail items have been processed?*/;
  });

  // Launch worker threads
  // This is a simplified thread pool. Use a loop and std::threads
  // For i in number of workers, start a thread with the pullWork function and
  // the stop condition as arguments.

  // Create the mail items (this thread owns them)

  // Pump the work into the work queue
  std::cout << "Adding initial work items to queue...\n";
  Action * action;
  for (auto & mailItem : mailItems) {
    action = new Action(std::bind(Fold, mailItem));
    gActionsQueue->push(action);
  }

  // transform the main thread in a worker
  // Here you could use the main thread as worker, not to waste it.

  // Join threads

  // clean up new'ed objects

  std::cout << "Work finished, threads joined\n";
}
