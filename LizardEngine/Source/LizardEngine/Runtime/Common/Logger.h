#pragma once

#include "Runtime/Macros.h"
#include "windows.h"
#include <cstdint>
#include <string>
#include <format>

namespace LizardEngine
{

	enum class LIZARD_API LogLevel : std::uint8_t
	{
		L_DEBUG,
		L_VERBOSE,
		L_INFO,
		L_WARNING,
		L_ERROR,
		L_FATAL,
	};

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

		template <typename... _Types>
		void PrintLog(std::wstring_view format, const _Types &..._Args)
		{
			OutputDebugString(std::format(L"[{}] {}", L"DEBUG", _Args...).c_str());
		}

	private:
		Logger(){};
		~Logger(){};
	};
}
