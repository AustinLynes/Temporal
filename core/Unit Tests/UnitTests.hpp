#pragma once

#include "testing/Framework.h"

#include "graphics/tests.h"
#include "memory/tests.h"


void AllTests() {

	GRAPHICS_TESTS;
	MEMORY_TESTS;
}

#define _ AllTests(); RUN_TEST_SUITE();