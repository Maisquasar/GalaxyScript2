#pragma once
#include <string>
#include <unordered_map>

using GetterMethod = void* (*)(void*);
using SetterMethod = void (*)(void*, void*);

struct Property
{
	std::string propertyName;
	std::string propertyType;
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

	std::unordered_map<std::string, Variable> m_variables;

};
