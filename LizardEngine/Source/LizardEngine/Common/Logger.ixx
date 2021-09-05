module;
#include "windows.h"

export module LizardEngine.Common : Logger;
import std.core;

export enum class LogLevel : std::uint8_t { L_DEBUG,
											L_VERBOSE,
											L_INFO,
											L_WARNING,
											L_ERROR,
											L_FATAL,
};

export class Logger
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

	template <typename... _Types>
	void PrintLog(std::wstring_view format, const _Types &..._Args)
	{
		OutputDebugString(std::format(L"[{}] {}\r\n", L"DEBUG", _Args...).c_str());
	}

private:
	Logger(){};
	~Logger(){};
};
