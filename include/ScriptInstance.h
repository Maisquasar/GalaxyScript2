#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace GS {
	using GetterMethod = void* (*)(void*);
	using SetterMethod = void (*)(void*, void*);
	using Constructor = void* (*)();
	using CallMethod = void (*)(void*);

	struct Property
	{
		std::vector<std::string> args;
		std::string name;
		std::string type;
	};

	struct Variable
	{
		Property property;
		GetterMethod getterMethod;
		SetterMethod setterMethod;
	};

	class ScriptInstance
	{
	public:
		friend class ScriptEngine;

		Constructor m_constructor;
		std::unordered_map<std::string, CallMethod> m_methods; // method name -> function pointer
		std::unordered_map<std::string, Variable> m_variables;
	};
}