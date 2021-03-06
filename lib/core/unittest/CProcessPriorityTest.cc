/*
 * Copyright Elasticsearch B.V. and/or licensed to Elasticsearch B.V. under one
 * or more contributor license agreements. Licensed under the Elastic License;
 * you may not use this file except in compliance with the Elastic License.
 */

#include <core/CProcessPriority.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(CProcessPriorityTest)

BOOST_AUTO_TEST_CASE(testReduceMemoryPriority) {
    BOOST_REQUIRE_NO_THROW(ml::core::CProcessPriority::reduceMemoryPriority());
}

BOOST_AUTO_TEST_CASE(testReduceCpuPriority) {
    BOOST_REQUIRE_NO_THROW(ml::core::CProcessPriority::reduceCpuPriority());
}

BOOST_AUTO_TEST_SUITE_END()
