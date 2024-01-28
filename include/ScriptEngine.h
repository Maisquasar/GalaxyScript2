#pragma once

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#elif defined(__APPLE__)
#endif

#include <filesystem>
#include <memory>
#include <set>
#include <string>

#include "ScriptInstance.h"

class ScriptEngine
{
public:
	ScriptEngine();
	~ScriptEngine();

	static ScriptEngine& CreateScriptEngine();

	// Add a new type with the parse name to the script engine
	void AddType(const std::string& typeName);

	bool LoadDLL(const std::filesystem::path& dllPath);

	void FreeDLL();

	// Add a header that will be parsed by the script engine
	void AddHeader(const std::filesystem::path& headerPath);

	static ScriptEngine* Get() {
		if (!m_instance)
		{
			CreateScriptEngine();
		}
		return m_instance.get();
	}
private:
	void ParseHeaderFile(const std::filesystem::path& headerPath);

	template<typename T>
	inline T* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName))
			return nullptr;

		const auto instance = m_scriptInstances[scriptName];
		if (!instance->m_variables.contains(variableName))
			return nullptr;

		const auto getter = instance->m_variables.at(variableName).getterMethod;
		return static_cast<T*>(getter(scriptComponent));
	}

	inline std::unordered_map<std::string, Variable> GetAllScriptVariablesInfo(const std::string& scriptName)
	{
		if (!m_scriptInstances.contains(scriptName))
			return {};

		const auto instance = m_scriptInstances[scriptName];
		return instance->m_variables;
	}

	inline std::unordered_map<std::string, void*> GetAllScriptVariables(void* scriptComponent, const std::string& scriptName)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName))
			return {};

		const auto instance = m_scriptInstances[scriptName];

		std::unordered_map<std::string, void*> variables;

		for (const auto& variable : instance->m_variables)
		{
			variables[variable.first] = variable.second.getterMethod(scriptComponent);
		}

		return variables;
	}

	template<typename T>
	inline void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, T* value)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName))
			return;

		const auto instance = m_scriptInstances[scriptName];
		if (!instance->m_variables.contains(variableName))
			return;

		const auto setter = instance->m_variables.at(variableName).setterMethod;
		setter(scriptComponent, value);
	}

	template<typename T>
	static inline T GetDLLMethod(void* dll, const char* name)
	{
#ifdef _WIN32
		return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(dll), name));
#elif defined(__linux__)
		return reinterpret_cast<T>(dlsym(dll, name));
#endif
		return T();
	}
private:
	friend class ScriptComponent;

	static std::unique_ptr<ScriptEngine> m_instance;

	std::set<std::string> m_typeNames;
	std::filesystem::path m_copyPath = std::filesystem::current_path() / "Dlls";

	std::vector<std::filesystem::path> m_headers;

	std::unordered_map<std::string, std::shared_ptr<ScriptInstance>> m_scriptInstances;

	void* m_handle = nullptr;
};
