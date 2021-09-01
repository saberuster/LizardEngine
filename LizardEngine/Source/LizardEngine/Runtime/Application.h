#pragma once

#include "Macros.h"
namespace LizardEngine
{
	class LIZARD_API IApplication
	{
	public:
		IApplication();
		virtual ~IApplication();

		IApplication(const IApplication &app) = delete;
		IApplication &operator=(const IApplication &app) = delete;

		virtual void Init() = 0;
		virtual void Tick() = 0;
		virtual void Run() = 0;
		virtual void Quit() = 0;
	};
}