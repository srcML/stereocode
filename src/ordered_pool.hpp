// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ordered_pool.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef ORDERED_POOL_HPP
#define ORDERED_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

// A pool of worker threads that runs jobs in any order and gives their results in the order of submission
// A job gets the number of its worker, from 0, for state of its own, and one thread submits the jobs and takes the results
//
template <typename Result>
class orderedPool {
public:
    explicit                                            orderedPool     (std::size_t);
                                                        ~orderedPool    ();
                                                        orderedPool     (const orderedPool&) = delete;
    orderedPool&                                        operator=       (const orderedPool&) = delete;

    template <typename Job>
    void                                                submit          (Job&&);
    Result                                              next            ();
    std::size_t                                         pending         () const    { return results.size(); }  // Jobs whose result was not taken
    std::size_t                                         workerCount     () const    { return workers.size(); }  // Workers started so far
private:
    void                                                work            (std::size_t);
    void                                                stop            () noexcept;

    std::size_t                                         maxWorkers;             // Workers that the pool starts at most, one per submitted job
    std::mutex                                          mutex;                  // Guards jobs and stopping
    std::condition_variable                             changed;                // Signals a new job or the stop
    std::deque<std::packaged_task<Result(std::size_t)>> jobs;                   // Jobs that no worker has started
    bool                                                stopping{false};        // The workers end
    std::deque<std::future<Result>>                     results;                // Results in the order of submission
    std::vector<std::thread>                            workers;                // Declared last, since the threads use the other members
};

// Prepares a pool of at most the given number of workers, which start with the first jobs
//
template <typename Result>
orderedPool<Result>::orderedPool(std::size_t maxWorkers_) : maxWorkers{maxWorkers_ > 0 ? maxWorkers_ : 1} {}

// Waits for the jobs that run and drops the jobs that no worker has started
//
template <typename Result>
orderedPool<Result>::~orderedPool() {
    stop();
}

// Queues a job, which a worker calls with its number, and starts a worker while the pool has fewer than its maximum
//
template <typename Result>
template <typename Job>
void orderedPool<Result>::submit(Job&& job) {
    if (workers.size() < maxWorkers) workers.emplace_back(&orderedPool::work, this, workers.size());

    std::packaged_task<Result(std::size_t)> task(std::forward<Job>(job));
    results.push_back(task.get_future());
    {
        const std::lock_guard<std::mutex> lock(mutex);
        jobs.push_back(std::move(task));
    }
    changed.notify_one();
}

// Waits for the result of the oldest job whose result was not taken, and throws the exception of a job that failed
//
template <typename Result>
Result orderedPool<Result>::next() {
    if (results.empty()) throw std::logic_error("the pool has no pending result");

    std::future<Result> result = std::move(results.front());
    results.pop_front();
    return result.get();
}

// Runs jobs until the pool stops
//
template <typename Result>
void orderedPool<Result>::work(std::size_t worker) {
    for (;;) {
        std::packaged_task<Result(std::size_t)> job;
        {
            std::unique_lock<std::mutex> lock(mutex);
            changed.wait(lock, [this] { return stopping || !jobs.empty(); });
            if (stopping) return;

            job = std::move(jobs.front());
            jobs.pop_front();
        }
        job(worker);
    }
}

// Ends the workers after their current jobs
//
template <typename Result>
void orderedPool<Result>::stop() noexcept {
    {
        const std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
    }
    changed.notify_all();
    for (std::thread& worker : workers)
        if (worker.joinable()) worker.join();
}

#endif
