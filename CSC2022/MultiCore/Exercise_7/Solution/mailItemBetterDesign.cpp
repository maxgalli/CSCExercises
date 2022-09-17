/* Example program that introduces to task based parallelism
g++ mailItemBetterDesign.cpp -o mailItemBetterDesign -std=c++17 -fgnu-tm
-pthread -lcurses
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

template <class T> class TsQueue {
public:
  TsQueue(size_t queueSize)
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

//------------------------------------------------------------------------------
// Useful type definitions
using Action = std::function<void()>;
using TsActionPtrQueue = TsQueue<Action *>;
using Duration = std::chrono::duration<float>;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

//------------------------------------------------------------------------------
// Small dummy class representing a mail item.
// It has an Id, a state, and knows about its transitions
class MailMonitor;

class MailItem {
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

  MailItem() : m_id(0), m_state(State::kStart), m_monitor(nullptr){};

  MailItem(size_t id, MailMonitor * monitor)
      : m_id(id), m_state(State::kStart), m_monitor(monitor){};
  size_t getId() { return m_id; };
  State getState() { return m_state; };
  // Method to go through the mail state machine
  // returns false once finished
  bool next() {
    switch (m_state) {
    case State::kStart:
      doWork(State::kFolded, "folding", 0.12);
      return true;
    case State::kFolded:
      doWork(State::kStuffed, "stuffing", 0.1);
      return true;
    case State::kStuffed:
      doWork(State::kSealed, "sealing", 0.24);
      return true;
    case State::kSealed:
      doWork(State::kAddressed, "addressing", 0.5);
      return true;
    case State::kAddressed:
      doWork(State::kStamped, "stamping", 0.05);
      return true;
    case State::kStamped:
      doWork(State::kMailed, "mailing", 0.7);
      return false;
    default:
      return false;
    }
  };

private:
  size_t m_id;
  State m_state;
  MailMonitor * m_monitor;
  void doWork(const State & newState, const std::string & action,
              const float deltaTf);
};

//------------------------------------------------------------------------------
class MailMonitor {
public:
  MailMonitor() {
    // initialize the stateCounter to zero everywhere
    m_stateCounter[MailItem::State::kStart] = 0;
    m_stateCounter[MailItem::State::kFolded] = 0;
    m_stateCounter[MailItem::State::kStuffed] = 0;
    m_stateCounter[MailItem::State::kSealed] = 0;
    m_stateCounter[MailItem::State::kAddressed] = 0;
    m_stateCounter[MailItem::State::kStamped] = 0;
    m_stateCounter[MailItem::State::kMailed] = 0;
    initscr();
    clear();
  };
  virtual ~MailMonitor(){};

  void add(MailItem::State state) { m_stateCounter[state]++; };
  void finalize() { endwin(); };
  void register_worker(std::thread::id thread) {
    m_workerAction[thread] = "";
    m_actionCounter[thread] = 0;
  }
  void remove(MailItem::State state) { m_stateCounter[state]--; };
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
            m_stateCounter[MailItem::State::kStart]);
    move(2, x_offset);
    printw("%10s :", "Folded");
    mvhline(2, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kFolded]);
    move(3, x_offset);
    printw("%10s :", "Stuffed");
    mvhline(3, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kStuffed]);
    move(4, x_offset);
    printw("%10s :", "Sealed");
    mvhline(4, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kSealed]);
    move(5, x_offset);
    printw("%10s :", "Addressed");
    mvhline(5, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kAddressed]);
    move(6, x_offset);
    printw("%10s :", "Stamped");
    mvhline(6, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kStamped]);
    move(7, x_offset);
    printw("%10s :", "Mailed");
    mvhline(7, x_offset + 14, ACS_BOARD,
            m_stateCounter[MailItem::State::kMailed]);
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
  std::map<MailItem::State, std::atomic<int>> m_stateCounter;
  std::map<std::thread::id, std::atomic<int>> m_actionCounter;
};

//------------------------------------------------------------------------------
// Implementation for MailItem::doWork has to be after full declaration of
// MailMonitor
void MailItem::doWork(const State & newState, const std::string & action,
                      const float deltaTf) {
  m_monitor->worker_busy(std::this_thread::get_id(), action);
  // spend some time doing 'work'
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> d(deltaTf, deltaTf * .4);
  Duration deltaT(std::fabs(d(gen)));
  Duration elapsed_seconds;
  TimePoint start, end;
  std::this_thread::sleep_for(deltaT);
  // update state and notify monitor object
  m_monitor->remove(m_state);
  m_state = newState;
  m_monitor->add(m_state);
  m_monitor->worker_free(std::this_thread::get_id());
};

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

//------------------------------------------------------------------------------
void doMonitor(MailMonitor * monitor, std::function<bool()> stopCondition) {
  while (stopCondition()) {
    monitor->update();
    std::chrono::milliseconds sleepDuration(50);
    std::this_thread::sleep_for(sleepDuration);
  }
}

//------------------------------------------------------------------------------
void doMail(MailItem & item, TsActionPtrQueue * p_actionsQueue,
            TsQueue<MailItem> * p_sentMailItemsQueue) {
  if (item.next()) {
    Action * work = new Action(
        std::bind(doMail, item, p_actionsQueue, p_sentMailItemsQueue));
    p_actionsQueue->push(work);
  } else {
    p_sentMailItemsQueue->push(item);
  }
}

//------------------------------------------------------------------------------
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

  std::cout << "Starting with " << nItems << " items and " << nThreads
            << " threads\n";
  //-------------------------------------------------------
  MailMonitor monitor;
  auto actionsQueue = TsActionPtrQueue(1000);
  auto sentMailItemsQueue = TsQueue<MailItem>(nItems);

  // Here the real orchestration starts!

  // We need a common signal when to stop
  bool workEnded = false;

  // Create a svc thread for display
  std::thread displaSvcThr(doMonitor, &monitor,
                           [&workEnded] { return !workEnded; });

  // Condition to stop pulling work from queue
  auto stopPullingWork(
      [&sentMailItemsQueue] { return !sentMailItemsQueue.isFull(); });

  // Launch worker threads
  std::vector<std::thread> workerThreads;
  for (int i = 0; i < nThreads - 1; ++i) { // 1 thread is the main thread :)
    workerThreads.emplace_back(pullWork, &actionsQueue, stopPullingWork, false);
  }
  // register the worker threads to the monitor
  for (auto & worker : workerThreads) {
    monitor.register_worker(worker.get_id());
  }

  // Create the mail items (this thread owns them)
  std::vector<MailItem> MailItems;
  MailItems.reserve(nItems);
  for (int i = 0; i < nItems; ++i) {
    MailItems.emplace_back(i, &monitor);
    monitor.add(MailItem::State::kStart);
  }
  // Pump the work into the work queue
  Action * action;
  for (auto & MailItem : MailItems) {
    action = new Action(
        std::bind(doMail, MailItem, &actionsQueue, &sentMailItemsQueue));
    actionsQueue.push(action);
  }

  // transform the main thread in a worker
  pullWork(&actionsQueue, stopPullingWork, false);

  // Join threads
  for (auto & thr : workerThreads) // 1 thread is the main thread :)
    thr.join();

  // no more work, let's join the logger thread
  workEnded = true;
  displaSvcThr.join();
  monitor.finalize();

  std::cout << "Work finished, threads joined\n";
}
