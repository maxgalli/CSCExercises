/* Example program that introduces to task based parallelism
 g++ easy.cpp -o easy -std=c++17 -fgnu-tm -pthread
*/
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

//------------------------------------------------------------------------------
// A possible implementation of a bounded thread safe queue using tm

template <class T> class TsStack {
public:
  TsStack(size_t queueSize)
      : m_maxSize(queueSize), m_currentSize(0), m_items(m_maxSize){};
  //---------
  size_t getNitems() {
    bool assigned = false;
    size_t currentSize = 0;
    while (!assigned) {
      currentSize = m_currentSize;
      assigned = true;
    }
    return currentSize;
  }
  //---------
  bool isFull() { return getNitems() == m_maxSize; };
  //--------
  void push(const T & item) {
    while (!try_push(item))
      ;
  };
  //---------
  bool try_push(const T & item) {
    bool success = false;
    if (m_currentSize < m_maxSize) {
      m_items[m_currentSize] = (item);
      m_currentSize++;
      success = true;
    }
    return success;
  };

  //---------
  void pop(T & item) {
    while (!try_pop(item))
      ;
  };
  //---------
  bool try_pop(T & item) {
    bool success = false;
    if (m_currentSize > 0) {
      m_currentSize--;
      item = m_items[m_currentSize];
      success = true;
    }
    return success;
  };

  //--------
  void dump() { // Non thread safe: here for debugging
    for (int i = 0; i < m_currentSize; ++i) {
      std::cout << "Item " << i << " " << m_items[i] << "\n";
    }
  }

private:
  const size_t m_maxSize;
  size_t m_currentSize;
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

void Mail(mailItem & item) {
  doWork(.7);
  item.setState(mailItem::State::kMailed);
  tsPrint("Mail item " + std::to_string(item.getId()) + " was sent.",
          std::this_thread::get_id());
  gSentMailItemsQueue->push(item);
}

void Stamp(mailItem & item) {
  doWork(.05);
  item.setState(mailItem::State::kStamped);
  Action * work = new Action(std::bind(Mail, item));
  tsPrint("Mail item " + std::to_string(item.getId()) + " was stamped.",
          std::this_thread::get_id());
  gActionsQueue->push(work);
}

void Address(mailItem & item) {
  doWork(.5);
  item.setState(mailItem::State::kAddressed);
  Action * work = new Action(std::bind(Stamp, item));
  tsPrint("Mail item " + std::to_string(item.getId()) + " was addressed.",
          std::this_thread::get_id());
  gActionsQueue->push(work);
}

void Seal(mailItem & item) {
  doWork(.24);
  item.setState(mailItem::State::kSealed);
  Action * work = new Action(std::bind(Address, item));
  tsPrint("Mail item " + std::to_string(item.getId()) + " was sealed.",
          std::this_thread::get_id());
  gActionsQueue->push(work);
}

void Stuff(mailItem & item) {
  doWork(.1);
  item.setState(mailItem::State::kStuffed);
  Action * work = new Action(std::bind(Seal, item));
  tsPrint("Mail item " + std::to_string(item.getId()) + " was stuffed.",
          std::this_thread::get_id());
  gActionsQueue->push(work);
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
  std::vector<mailItem> mailItems;
  mailItems.reserve(nItems);
  for (int i = 0; i < nItems; ++i)
    mailItems.emplace_back(i);

  // Pump the work into the work queue
  std::cout << "Adding initial work items to queue...\n";
  Action * action;
  for (auto & mailItem : mailItems) {
    action = new Action(std::bind(Fold, mailItem));
    gActionsQueue->push(action);
  }

  // transform the main thread in a worker
  pullWork(gActionsQueue, stopPullingWork, false);

  // Join threads
  for (auto & thr : workerThreads) // 1 thread is the main thread :)
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
