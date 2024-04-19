#pragma once 

#include "TestRunner.h"

#define TEST_CASE(name, category) \
TestRunner::RegisterTest(name, category)

#define RUN_TEST_SUITE() \
TestRunner::Run()

#define REQUIRE(condition) \
     Require(#condition, [&]() { return condition; }, __FILE__, __LINE__); \
