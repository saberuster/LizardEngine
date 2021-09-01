#pragma once

#include "Runtime/Macros.h"

class LIZARD_API Logger
{
public:
	Logger(Logger const &) = delete;
	Logger(Logger &&) = delete;
	Logger &operator=(Logger const &) = delete;
	Logger &operator=(Logger &&) = delete;

	static Logger &Instance()
	{
		static Logger instance;
		return instance;
	}

	void PrintLog();

private:
	Logger(){};
	~Logger(){};
};
