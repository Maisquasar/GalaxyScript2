#pragma once
#include "API.h"

#include <memory>
#include <string>
#include <vector>
#include <set>

#include "ScriptEngine.h"


class PROJECT_API IComponent
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

class PROJECT_API ScriptComponent : public IComponent
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

	std::unordered_map<std::string, Variable> GetAllVariableInfo() const;
};

class ComponentHandler
{
public:
	template<typename T>
	static void RegisterScriptComponent(T* component)
	{
		m_components[component->Internal_GetClassName()] = std::shared_ptr<ScriptComponent>(component);
	}

	static std::shared_ptr<IComponent> CreateWithComponentName(const std::string& name)
	{
		if (!m_components.contains(name))
			return nullptr;
		return std::shared_ptr<IComponent>(static_cast<IComponent*>(m_components.at(name)->Clone()));
	}

private:
	static std::unordered_map<std::string, std::shared_ptr<IComponent>> m_components;
};
