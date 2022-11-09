#include "job.h"

#include <algorithm>

namespace task {

// Job.
void Job::exec() {
  if (observer_) {
    observer_->on_pre_exec_job(this);
  }
  state_ = State::Exec;
  on_exec();
  state_ = State::Done;
  if (observer_) {
    observer_->on_post_exec_job(this);
  }
}

bool Job::change_state(State state) {
  if (state_ == state) return true;
  bool is_ok = false;
  switch (state) {
    case State::None:
      is_ok = state_ == State::Done;
      break;
    case State::Submit:
      is_ok = state_ == State::None;
      break;
    case State::Exec:
      is_ok = state_ == State::Submit;
      break;
    case State::Done:
      is_ok = state_ == State::Exec;
      break;
  }
  if (is_ok) {
    state_ = state;
  }
  return is_ok;
}

// JobExecutor.
/*virtual*/ JobExecutor::~JobExecutor() { stop(); }

bool JobExecutor::start(std::size_t thread_n) {
  threads_.resize(thread_n);
  for (std::size_t i = 0; i < thread_n; ++i) {
    std::thread t([this]() { exec_jobs_(); });
    threads_[i] = std::move(t);
  }
  return true;
}
void JobExecutor::stop() {
  is_stop_ = true;
  condition_.notify_all();
  std::for_each(threads_.begin(), threads_.end(), [](auto& t) { t.join(); });
  threads_.clear();
  jobs_.clear();
  is_stop_ = false;
}

void JobExecutor::submit(std::shared_ptr<Job> job) {
  std::unique_lock lock(mutex_);
  job->set_observer(this);
  jobs_.emplace_back(std::move(job));
}
void JobExecutor::kick() { condition_.notify_all(); }
void JobExecutor::join() {
  do {
    std::shared_ptr<Job> job;
    {
      std::unique_lock lock(mutex_);
      bool is_done = jobs_.empty() && active_job_count_ == 0;
      if (is_done) break;
      auto it = std::find_if(jobs_.begin(), jobs_.end(),
                             [](auto& job) { return job->can_exec(); });
      if (it == jobs_.end()) {
        condition_.wait(lock);
        continue;
      }
      job = *it;
      jobs_.erase(it);
      ++active_job_count_;
    }
    job->exec();
  } while (true);
}

/*virtual*/ void JobExecutor::on_pre_exec_job(Job* /*job*/) /*override*/ {}
/*virtual*/ void JobExecutor::on_post_exec_job(Job* /*job*/) /*override*/ {
  std::unique_lock lock(mutex_);
  --active_job_count_;
  condition_.notify_all();
}

void JobExecutor::exec_jobs_() {
  while (!is_stop_) {
    std::shared_ptr<Job> job;
    {
      std::unique_lock lock(mutex_);
      auto it = std::find_if(jobs_.begin(), jobs_.end(),
                             [](auto& job) { return job->can_exec(); });
      if (it == jobs_.end()) {
        if (is_stop_) continue;
        condition_.wait(lock);
        continue;
      }
      job = *it;
      jobs_.erase(it);
      ++active_job_count_;
    }
    job->exec();
  }
}

}  // namespace task