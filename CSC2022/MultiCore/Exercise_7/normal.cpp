/* Example program that introduces to task based parallelism
g++ normal.cpp -o normal -std=c++17 -fgnu-tm -pthread -Wall -Wextra -Wpedantic
-Werror
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
void tsPrint(const std::string & msg, std::thread::id thrId) {
  auto printLambda = [thrId](std::string msgCopy) {
    std::cout << "[" << thrId << "] " << msgCopy << "\n";
  };
  Action * action = new Action((std::bind(printLambda, msg)));
  gMsgQueue->push(action);
}

//------------------------------------------------------------------------------
// A dummy function which just spends time crunching CPU
void doWork(float deltaTf) {
  std::this_thread::sleep_for(std::chrono::duration<float>(deltaTf));
}

//------------------------------------------------------------------------------
// Small dummy class representing a mail item. It has an Id and a state
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

  mailItem() : m_id(0), m_state(State::kStart){};

  mailItem(size_t id) : m_id(id), m_state(State::kStart){};

  size_t getId() { return m_id; };

  State getState() { return m_state; };

  void setState(State state) { m_state = state; };

private:
  size_t m_id;
  State m_state;
};

TsStack<mailItem> * gSentMailItemsQueue;
//------------------------------------------------------------------------------
// Mini functions that represent the actions applicable to a mail item

// The implementation of the functions to evolve between intermediate states
// is missing. Note that each function should push into the work queue the
// work item corresponding to the evolution to the next state.

void Mail(mailItem & item) {
  doWork(.7);
  item.setState(mailItem::State::kMailed);
  tsPrint("Mail item " + std::to_string(item.getId()) + " was sent.",
          std::this_thread::get_id());
  gSentMailItemsQueue->push(item);
}

void Fold(mailItem & item) {
  doWork(.12);
  item.setState(mailItem::State::kFolded);
  tsPrint("Mail item " + std::to_string(item.getId()) + " was folded.",
          std::this_thread::get_id());
  Action * work = new Action(std::bind(Stuff, item));
  gActionsQueue->push(work);
}

//------------------------------------------------------------------------------
// Pull work items from a work queue and stop when necessary
// It can sleep.
void pullWork(TsActionPtrQueue * workQueue, std::function<bool()> stopCondition,
              bool sleep) {

  std::chrono::milliseconds sleepDuration(10);
  Action * action = nullptr;
  tsPrint("Start pulling work", std::this_thread::get_id());
  while (workQueue->try_pop(action) || stopCondition()) {
    if (action) {
      (*action)();
      delete action;
      action = nullptr;
    }
    if (sleep) {
      std::this_thread::sleep_for(sleepDuration);
    }
  }
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

  gSentMailItemsQueue = new TsStack<mailItem>(nItems);

  std::cout << "Starting with " << nItems << " items and " << nThreads
            << " threads\n";
  //-------------------------------------------------------

  // Here the real orchestration starts!

  // We need a common signal when to stop
  bool workEnded = false;

  // Create a svc thread for logging. This one sleeps:
  // we do not need a full core for logging!
  std::thread msgSvcThr(
      pullWork, gMsgQueue, [&workEnded] { return !workEnded; }, true);

  // Condition to stop pulling work from queue
  auto stopPullingWork([] { return !gSentMailItemsQueue->isFull(); });

  // Launch worker threads
  std::vector<std::thread> workerThreads;
  for (int i = 0; i < nThreads - 1; ++i) { // 1 thread is the main thread :)
    workerThreads.emplace_back(pullWork, gActionsQueue, stopPullingWork, false);
    std::cout << "Worker thread " << i << " created\n";
  }

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
  for (auto & thr : workerThreads)
    thr.join();

  // no more work, let's join the logger thread
  workEnded = true;
  msgSvcThr.join();

  // clean up
  delete gActionsQueue;
  delete gMsgQueue;
  delete gSentMailItemsQueue;

  std::cout << "Work finished, threads joined\n";
}
