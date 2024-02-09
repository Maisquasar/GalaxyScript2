#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>

#include "../include/ScriptEngine.h"

class IComponent
{
public:
	IComponent() {}
	IComponent& operator=(const IComponent& other) = default;
	IComponent(const IComponent&) = default;
	IComponent(IComponent&&) noexcept = default;
	virtual ~IComponent() {}

	virtual void* Clone() = 0;
	virtual const char* Internal_GetClassName() const { return "IComponent"; }
	virtual std::set<const char*> Internal_GetClassNames() const { return {}; }

};

class ScriptComponent : public IComponent
{
public:
	ScriptComponent() {}
	ScriptComponent& operator=(const ScriptComponent& other) = default;
	ScriptComponent(const ScriptComponent&) = default;
	ScriptComponent(ScriptComponent&&) noexcept = default;
	~ScriptComponent() override {}

	virtual void* Clone() override { return nullptr; }

	virtual const char* Internal_GetClassName() const { return "ScriptComponent"; }
	virtual std::set<const char*> Internal_GetClassNames() const
	{
		std::set<const char*> names = IComponent::Internal_GetClassNames();
		names.insert(ScriptComponent::Internal_GetClassName());
		return names;
	}

	template<typename T>
	inline T* GetVariable(const std::string& variableName)
	{
		return ScriptEngine::Get()->GetScriptVariable<T>(this, Internal_GetClassName(), variableName);
	}

	template<typename T>
	inline void SetVariable(const std::string& variableName, T value)
	{
		return ScriptEngine::Get()->SetScriptVariable<T>(this, Internal_GetClassName(), variableName, &value);
	}

	inline void CallMethodByName(const std::string& methodName)
	{
		ScriptEngine::Get()->CallScriptMethod(this, Internal_GetClassName(), methodName);
	}

	std::unordered_map<std::string, Variable> GetAllVariableInfo() const;
	std::unordered_map<std::string, CallMethod> GetAllMethodsInfo() const;
};
