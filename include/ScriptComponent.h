#pragma once
#include "API.h"

#include <memory>
#include <string>
#include <vector>

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
	virtual	const char* GetComponentName() const = 0;

};

class PROJECT_API ScriptComponent : public IComponent
{
public:
	ScriptComponent() {}
	ScriptComponent& operator=(const ScriptComponent& other) = default;
	ScriptComponent(const ScriptComponent&) = default;
	ScriptComponent(ScriptComponent&&) noexcept = default;
	~ScriptComponent() override {}

	virtual void* Clone() override { return nullptr; };
	virtual	const char* GetComponentName() const override { return ""; };

	template<typename T>
	inline T* GetVariable(const std::string& variableName)
	{
		return ScriptEngine::Get()->GetScriptVariable<T>(this, GetComponentName(), variableName);
	}

	template<typename T>
	inline void SetVariable(const std::string& variableName, T value)
	{
		return ScriptEngine::Get()->SetScriptVariable<T>(this, GetComponentName(), variableName, &value);
	}

	std::unordered_map<std::string, Variable> GetAllVariableInfo() const;
};

class ComponentHandler
{
public:
	template<typename T>
	static void RegisterComponent()
	{
		m_components.push_back(std::make_shared<T>());
	}

	template<typename T>
	static void RegisterScriptComponent(T* component)
	{
		m_components.push_back(std::shared_ptr<ScriptComponent>(component));
	}

	static std::shared_ptr<IComponent> CreateWithComponentName(const std::string& name)
	{
		for (const auto& component : m_components)
		{
			if (component->GetComponentName() == name)
			{
				return std::shared_ptr<IComponent>(static_cast<IComponent*>(component->Clone()));
			}
		}
		return nullptr;
	}

private:
	static std::vector<std::shared_ptr<IComponent>> m_components;
};
