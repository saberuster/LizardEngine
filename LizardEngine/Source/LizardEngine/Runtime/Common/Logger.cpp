#include "Logger.h"
#include <format>
#include <iostream>

void Logger::PrintLog()
{
	std::cout << std::format("{}", "hello world");
}
