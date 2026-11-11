// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ordered_pool_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "ordered_pool.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

TEST_CASE("The pool gives the results in the order of submission") {
    orderedPool<int> pool(4);
    for (int job = 0; job < 8; ++job) {
        pool.submit([job](std::size_t) {
            std::this_thread::sleep_for(std::chrono::milliseconds(8 - job));
            return job;
        });
    }

    for (int job = 0; job < 8; ++job) CHECK(pool.next() == job);
    CHECK(pool.pending() == 0);
}

TEST_CASE("The pool rethrows the exception of a failed job when its result is taken") {
    orderedPool<int> pool(2);
    pool.submit([](std::size_t) { return 1; });
    pool.submit([](std::size_t) -> int { throw std::runtime_error("failed job"); });
    pool.submit([](std::size_t) { return 3; });

    CHECK(pool.next() == 1);
    CHECK_THROWS_WITH_AS(pool.next(), "failed job", std::runtime_error);
    CHECK(pool.next() == 3);
}

TEST_CASE("The pool starts a worker for each job up to its number of workers") {
    orderedPool<std::size_t> pool(8);
    CHECK(pool.workerCount() == 0);

    pool.submit([](std::size_t worker) { return worker; });
    pool.submit([](std::size_t worker) { return worker; });
    CHECK(pool.workerCount() == 2);

    for (int job = 0; job < 20; ++job) pool.submit([](std::size_t worker) { return worker; });
    CHECK(pool.workerCount() == 8);

    while (pool.pending() > 0) CHECK(pool.next() < 8);
}

TEST_CASE("Taking a result without a pending job is an error") {
    orderedPool<int> pool(1);
    CHECK_THROWS_AS(pool.next(), std::logic_error);

    pool.submit([](std::size_t) { return 1; });
    CHECK(pool.next() == 1);
    CHECK_THROWS_AS(pool.next(), std::logic_error);
}
