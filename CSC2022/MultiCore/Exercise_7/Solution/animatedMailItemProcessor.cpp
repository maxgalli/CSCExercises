/* Example program that introduces to task based parallelism
g++ animatedMailItemProcessor.cpp -o animatedMailItemProcessor -std=c++17
-fgnu-tm -pthread -lcurses -Wall -Wextra -Wpedantic -Werror
*/
#include "curses.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <random>
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
    while (!assigned)
      __transaction_atomic {
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
    __transaction_atomic {
      if (m_currentSize < m_maxSize) {
        m_items[m_currentSize] = (item);
        m_currentSize++;
        success = true;
      }
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
    __transaction_atomic {
      if (m_currentSize > 0) {
        m_currentSize--;
        item = m_items[m_currentSize];
        success = true;
      }
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

//------------------------------------------------------------------------------
// A dummy function which just spends time crunching CPU
using Duration = std::chrono::duration<float>;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

void doWork(float deltaTf) {
  // Let's add a jitter to have a situation closer to reality
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> d(deltaTf, deltaTf * .4);
  Duration deltaT(std::fabs(d(gen)));
  Duration elapsed_seconds;
  TimePoint start, end;
  std::this_thread::sleep_for(deltaT);
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

class MailMonitor {
public:
  MailMonitor() {
    // initialize the stateCounter to zero everywhere
    m_stateCounter[mailItem::State::kStart] = 0;
    m_stateCounter[mailItem::State::kFolded] = 0;
    m_stateCounter[mailItem::State::kStuffed] = 0;
    m_stateCounter[mailItem::State::kSealed] = 0;
    m_stateCounter[mailItem::State::kAddressed] = 0;
    m_stateCounter[mailItem::State::kStamped] = 0;
    m_stateCounter[mailItem::State::kMailed] = 0;
    initscr();
    clear();
  };
  virtual ~MailMonitor(){};

  void add(mailItem::State state) { m_stateCounter[state]++; };
  void finalize() { endwin(); };
  void register_worker(std::thread::id thread) {
    m_workerAction[thread] = "";
    m_actionCounter[thread] = 0;
  }
  void remove(mailItem::State state) { m_stateCounter[state]--; };
  // The two following methods are in principle not thread safe. Why isn't that
  // a problem here?
  void worker_busy(std::thread::id thread, std::string action) {
    m_workerAction[thread] = action;
    ++m_actionCounter[thread];
  };
  void worker_free(std::thread::id thread) { m_workerAction[thread] = ""; };
  void update() {
    clear();
    // display all queues
    const int x_offset(2);
    move(1, x_offset);
    printw("%10s :", "Start");
    mvhline(1, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kStart]);
    move(2, x_offset);
    printw("%10s :", "Folded");
    mvhline(2, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kFolded]);
    move(3, x_offset);
    printw("%10s :", "Stuffed");
    mvhline(3, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kStuffed]);
    move(4, x_offset);
    printw("%10s :", "Sealed");
    mvhline(4, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kSealed]);
    move(5, x_offset);
    printw("%10s :", "Addressed");
    mvhline(5, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kAddressed]);
    move(6, x_offset);
    printw("%10s :", "Stamped");
    mvhline(6, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kStamped]);
    move(7, x_offset);
    printw("%10s :", "Mailed");
    mvhline(7, x_offset + 14, ACS_BOARD,
            m_stateCounter[mailItem::State::kMailed]);
    // display all workers
    int index(0);
    move(9, x_offset);
    printw("%12s  %10s    %s", "Thread ID", "Action", "Performed actions");
    for (auto action : m_workerAction) {
      move(11 + index, x_offset);
      printw("%12u  %10s", action.first, action.second.c_str());
      ++index;
    }
    index = 0;
    for (auto & counter : m_actionCounter) {
      mvhline(11 + index, x_offset + 28, ACS_DIAMOND, counter.second);
      ++index;
    }
    move(0, 0);
    refresh();
  };

private:
  std::map<std::thread::id, std::string> m_workerAction;
  std::map<mailItem::State, std::atomic<int>> m_stateCounter;
  std::map<std::thread::id, std::atomic<int>> m_actionCounter;
};

MailMonitor * gMailMonitor;
TsStack<mailItem> * gSentMailItemsQueue;

//------------------------------------------------------------------------------
// Mini functions that represent the actions applicable to a mail item

void Mail(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "mailing");
  doWork(.7);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kMailed);
  gSentMailItemsQueue->push(item);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

void Stamp(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "stamping");
  doWork(.05);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kStamped);
  Action * work = new Action(std::bind(Mail, item));
  gActionsQueue->push(work);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

void Address(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "addressing");
  doWork(.5);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kAddressed);
  Action * work = new Action(std::bind(Stamp, item));
  gActionsQueue->push(work);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

void Seal(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "sealing");
  doWork(.24);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kSealed);
  Action * work = new Action(std::bind(Address, item));
  gActionsQueue->push(work);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

void Stuff(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "stuffing");
  doWork(.1);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kStuffed);
  Action * work = new Action(std::bind(Seal, item));
  gActionsQueue->push(work);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

void Fold(mailItem & item) {
  gMailMonitor->worker_busy(std::this_thread::get_id(), "folding");
  doWork(.12);
  gMailMonitor->remove(item.getState());
  item.setState(mailItem::State::kFolded);
  Action * work = new Action(std::bind(Stuff, item));
  gActionsQueue->push(work);
  gMailMonitor->add(item.getState());
  gMailMonitor->worker_free(std::this_thread::get_id());
}

//------------------------------------------------------------------------------
// Pull work items from a work queue and stop when necessary
// It can sleep.
void pullWork(TsActionPtrQueue * workQueue, std::function<bool()> stopCondition,
              bool sleep) {

  std::chrono::milliseconds sleepDuration(10);
  Action * action = nullptr;
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

void doMonitor(MailMonitor * monitor, std::function<bool()> stopCondition) {
  while (stopCondition()) {
    monitor->update();
    std::chrono::milliseconds sleepDuration(50);
    std::this_thread::sleep_for(sleepDuration);
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
  MailMonitor monitor;
  gMailMonitor = &monitor;
  // Here the real orchestration starts!

  // We need a common signal when to stop
  bool workEnded = false;

  // Create a svc thread for display
  std::thread displaSvcThr(doMonitor, gMailMonitor,
                           [&workEnded] { return !workEnded; });

  // Condition to stop pulling work from queue
  auto stopPullingWork([] { return !gSentMailItemsQueue->isFull(); });

  // Launch worker threads
  std::vector<std::thread> workerThreads;
  for (int i = 0; i < nThreads - 1; ++i) { // 1 thread is the main thread :)
    workerThreads.emplace_back(pullWork, gActionsQueue, stopPullingWork, false);
  }
  // register the worker threads to the monitor
  for (auto & worker : workerThreads) {
    monitor.register_worker(worker.get_id());
  }

  // Create the mail items (this thread owns them)
  std::vector<mailItem> mailItems;
  mailItems.reserve(nItems);
  for (int i = 0; i < nItems; ++i) {
    mailItems.emplace_back(i);
    gMailMonitor->add(mailItem::State::kStart);
  }
  // Pump the work into the work queue
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
  displaSvcThr.join();
  monitor.finalize();

  // clean up
  delete gActionsQueue;
  delete gSentMailItemsQueue;

  std::cout << "Work finished, threads joined\n";
}
