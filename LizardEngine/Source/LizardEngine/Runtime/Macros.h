#pragma once

#ifdef LIZARD_PLATFORM_WINDOWS
#ifdef LIZARD_BUILD_DLL
#define LIZARD_API __declspec(dllexport)
#else
#define LIZARD_API __declspec(dllimport)
#endif // LIZARD_BUILD_DLL
#else
#error LizardEngine only suport Windows!
#endif // LIZARD_PLATFORM_WINDOWS
