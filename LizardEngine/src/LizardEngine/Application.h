#pragma once

#include "Macros.h"

namespace LizardEngine {


	class LIZARD_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};


	LIZARD_API void TestFunc();

}