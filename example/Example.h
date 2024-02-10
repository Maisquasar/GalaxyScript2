#pragma once

#if defined(_WIN32)
#if defined(EXAMPLE_EXPORT)
#define EXAMPLE_API __declspec(dllexport)
#else
#define EXAMPLE_API __declspec(dllimport)
#endif // GALAXY_EXPORTS
#elif defined(__linux__)
#define EXAMPLE_API __attribute__((visibility("default")))
#endif

class EXAMPLE_API Example
{
public:
	void RunExample();
};

#ifdef _WIN32
extern "C" __declspec(dllexport) void RandomMethod();
#elif defined(__linux__)
extern "C" __attribute__((visibility("default"))) void RandomMethod();
#endif