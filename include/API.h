#pragma once
#if defined(_WIN32)
#define PROJECT_API __declspec(dllexport)
#elif defined(__linux__)
#define PROJECT_API __attribute__((visibility("default")))
#endif