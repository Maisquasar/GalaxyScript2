#include <iostream>
#include <vector>

#ifdef _WIN32
#define EXPORT_FUNC extern "C" __declspec(dllexport)
#elif defined(__linux__)
#define EXPORT_FUNC extern "C"
#endif

#define GENERATED_BODY()
#define PROPERTY(...)