#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "reference_count.h"

namespace {

class JobBase : public t9::RcObject {
 private:
  enum class State {
    INIT,
    QUEUED,
    RUNNING,
    DONE,
  };

 private:
  State state_ = State::INIT;
  std::mutex mutex_;
  std::condition_variable condition_;

 public:
  virtual ~JobBase() = default;

  void enqueue() { state_ = State::QUEUED; }
  void run() {
    state_ = State::RUNNING;
    on_run();
    std::unique_lock lock(mutex_);
    state_ = State::DONE;
    condition_.notify_all();
  }
  void join() {
    std::unique_lock lock(mutex_);
    if (state_ == State::QUEUED || state_ == State::RUNNING) {
      condition_.wait(lock, [&]() { return state_ == State::DONE; });
    }
  }

  bool is_submitted() const {
    return state_ == State::QUEUED || state_ == State::RUNNING;
  }
  bool is_done() const { return state_ == State::DONE; }

 protected:
  virtual void on_run() = 0;
};

class JobExecutor {
 private:
  JobExecutor(const JobExecutor&) = delete;
  JobExecutor(JobExecutor&&) = delete;
  JobExecutor& operator=(const JobExecutor&) = delete;
  JobExecutor& operator=(JobExecutor&&) = delete;

 private:
  std::vector<std::thread> threads_;
  bool is_stop_ = false;
  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<t9::Rc<JobBase>> jobs_;

 public:
  explicit JobExecutor(std::size_t n) : threads_(n) {
    for (std::size_t i = 0; i < n; ++i) {
      threads_[i] = std::thread([&]() { run_job(); });
    }
  }
  ~JobExecutor() {
    is_stop_ = true;
    condition_.notify_all();
    for (auto& thread : threads_) {
      thread.join();
    }
  }

  bool submit(const t9::Rc<JobBase>& job) {
    if (!job || job->is_submitted()) return false;

    job->enqueue();
    jobs_.push_back(job);
    condition_.notify_one();
    return true;
  }

  void run_all_jobs() {
    while (!jobs_.empty()) {
      t9::Rc<JobBase> job;
      {
        std::unique_lock lock(mutex_);
        if (!jobs_.empty()) {
          job = jobs_.front();
          jobs_.pop_front();
        }
      }
      if (job) {
        job->run();
      }
    }
  }

 private:
  void run_job() {
    while (!is_stop_) {
      t9::Rc<JobBase> job;
      {
        std::unique_lock lock(mutex_);
        condition_.wait(lock, [&]() { return !jobs_.empty() || is_stop_; });
        if (!jobs_.empty()) {
          job = jobs_.front();
          jobs_.pop_front();
        }
      }
      if (job) {
        job->run();
      }
    }
  }
};

template <typename... Args>
class Task {
 private:
  using FuncType = std::function<void(Args...)>;

 private:
  int phase_ = 0;
  FuncType func_;
  Task* next_ = nullptr;

 public:
  template <typename F>
  Task(int phase, F f) : phase_(phase), func_(f) {}

  void run(Args... args) { func_(args...); }

  void link_next(Task* task) { next_ = task; }

  int phase() const { return phase_; }
  Task* next_task() const { return next_; }
};

template <typename... Args>
class TaskJob : public JobBase {
 private:
  using TaskType = Task<Args...>;

 private:
  TaskType* task_;
  std::tuple<Args...> args_;

 public:
  TaskJob(TaskType* task, Args... args) : task_(task), args_(args...) {}

 protected:
  virtual void on_run() override {
    std::apply([&](Args... args) { task_->run(args...); }, args_);
  }
};

template <typename... Args>
class TaskScheduler {
 private:
  TaskScheduler(const TaskScheduler&) = delete;
  TaskScheduler(TaskScheduler&&) = delete;
  TaskScheduler& operator=(const TaskScheduler&) = delete;
  TaskScheduler& operator=(TaskScheduler&&) = delete;

 private:
  using TaskType = Task<Args...>;
  using TaskJobType = TaskJob<Args...>;

 private:
  JobExecutor jobExecutor_;
  std::list<std::unique_ptr<TaskType>> tasks_;
  TaskType* first_task_ = nullptr;

 public:
  explicit TaskScheduler(std::size_t n) : jobExecutor_(n) {}

  template <class F>
  TaskType* create(int phase, F f) {
    auto p = new TaskType(phase, f);
    tasks_.emplace_back(p);
    TaskType* last_task = nullptr;
    for (auto task = first_task_; task; task = task->next_task()) {
      if (task->phase() <= phase) {
        last_task = task;
        continue;
      }
      break;
    }
    if (last_task) {
      p->link_next(last_task->next_task());
      last_task->link_next(p);
    } else {
      p->link_next(first_task_);
      first_task_ = p;
    }
    return p;
  }

  void run(Args... args) {
    if (!first_task_) return;

    std::vector<t9::Rc<JobBase>> jobs;
    auto run_all_jobs = [&]() {
      for (auto& job : jobs) {
        jobExecutor_.submit(job);
      }
      jobExecutor_.run_all_jobs();
      for (auto& job : jobs) {
        job->join();
      }
      jobs.clear();
    };

    int phase = first_task_->phase();
    for (auto task = first_task_; task; task = task->next_task()) {
      if (phase != task->phase()) {
        run_all_jobs();
        phase = task->phase();
      }
      t9::Rc<JobBase> job(new TaskJobType(task, args...), false);
      jobs.push_back(job);
    }
    if (!jobs.empty()) {
      run_all_jobs();
    }
  }
};

struct Foo {
  std::mutex io_mutex;

  template <int Id>
  void print(int frame) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::lock_guard lock(io_mutex);
    std::cout << frame << "> ID: " << Id << " in " << std::this_thread::get_id()
              << std::endl;
  }
};

}  // namespace

int main() {
  auto n = std::thread::hardware_concurrency();
  std::cout << "thread num: " << n << std::endl;
  std::cout << "main thread: " << std::this_thread::get_id() << std::endl;

  Foo foo;

  TaskScheduler<int> tasks(std::max(n, 1U));

  tasks.create(1, [&foo](int frame) { foo.print<0>(frame); });
  tasks.create(1, [&foo](int frame) { foo.print<1>(frame); });
  tasks.create(1, [&foo](int frame) { foo.print<2>(frame); });
  tasks.create(1, [&foo](int frame) { foo.print<3>(frame); });
  tasks.create(1, [&foo](int frame) { foo.print<4>(frame); });

  tasks.create(2, [&foo](int frame) { foo.print<5>(frame); });
  tasks.create(2, [&foo](int frame) { foo.print<6>(frame); });
  tasks.create(2, [&foo](int frame) { foo.print<7>(frame); });
  tasks.create(2, [&foo](int frame) { foo.print<8>(frame); });
  tasks.create(2, [&foo](int frame) { foo.print<9>(frame); });

  for (int i = 0; i < 2; ++i) {
    std::cout << "--> frame: " << i << std::endl;
    tasks.run(i);
  }
}
