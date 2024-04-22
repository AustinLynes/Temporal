#pragma once


enum class TReturn 
{
	// GRAPHICS SPECIFIC
	SUBOPTIMAL = 10101, 



	COMMAND_EXECUTECUTION_FAILED = 99,

	FILE_NOT_FOUND		= -2,
	INVALID_ARGUMENT	= -1,
	FAILURE = 0,
	SUCCESS = 1
};

#define TFAILED(res) res < TReturn::SUCCESS

#define TFAIL_MSG(res) (#res " failed...")

#include "cassert"

void operator -(TReturn ret);
