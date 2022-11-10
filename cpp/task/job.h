#pragma once
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "t9/noncopyable.h"

namespace task {

class Job;

// JobObserver.
class JobObserver {
 public:
  virtual ~JobObserver() = default;
  virtual void on_pre_exec_job(Job* job) = 0;
  virtual void on_post_exec_job(Job* job) = 0;
};

// Job.
class Job {
 public:
  enum class State {
    None,
    Submit,
    Exec,
    Done,
  };

 private:
  std::string name_;
  State state_ = State::None;
  JobObserver* observer_ = nullptr;

 public:
  explicit Job(std::string_view name) : name_(name) {}
  virtual ~Job() = default;

  bool can_exec() const { return on_can_exec(); }
  void exec();

  bool change_state(State state);
  bool reset_state() { return change_state(State::None); }
  bool is_state(State state) const { return state_ == state; }
  std::string_view name() const { return name_; }

  void set_observer(JobObserver* observer) { observer_ = observer; }

 protected:
  virtual bool on_can_exec() const { return true; }
  virtual void on_exec() = 0;
};

// JobExecutor.
class JobExecutor : private t9::NonCopyable, public JobObserver {
 private:
  std::vector<std::thread> threads_;
  std::deque<std::shared_ptr<Job>> jobs_;
  std::size_t active_job_count_ = 0;
  std::mutex mutex_;
  std::condition_variable condition_;
  volatile bool is_stop_ = false;

 public:
  virtual ~JobExecutor();

  bool start(std::size_t thread_n);
  void stop();

  void submit(std::shared_ptr<Job> job);
  void kick();
  void join();

  std::size_t thread_count() const { return threads_.size(); }

 public:
  // JobObserver.
  virtual void on_pre_exec_job(Job* job) override;
  virtual void on_post_exec_job(Job* job) override;

 private:
  void exec_jobs_();
};

}  // namespace task