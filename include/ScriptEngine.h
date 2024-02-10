#pragma once

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#elif defined(__APPLE__)
#endif

#include <iostream>
#include <filesystem>
#include <memory>
#include <set>
#include <string>

#include "ScriptInstance.h"

class ScriptEngine
{
public:
	ScriptEngine() = default;

	bool LoadDLL(std::filesystem::path dllPath);

	void FreeDLL() const;

	// Will create (if not exist) and get the script instance
	static ScriptEngine* Get() {
		if (!m_instance)
		{
			CreateScriptEngine();
		}
		return m_instance.get();
	}

	// Set the folder path where all the .gen files are
	void SetHeaderGenFolder(const std::filesystem::path& path) { m_headerGenFolder = path; }

	// Will create a shared_ptr of the class script with the name, return nullptr if not found
	template<typename T>
	std::shared_ptr<T> CreateWithClassName(const std::string& name)
	{
		if (!m_scriptInstances.contains(name)) {
			std::cout << "Failed to get script instance of " << name << std::endl;
			return nullptr;
		}
		return std::shared_ptr<T>(static_cast<T*>(m_scriptInstances.at(name)->m_constructor()));
	}

	// Set the folder path where all the dll will be copied
	void SetCopyToFolder(const std::filesystem::path& path) { m_copyPathFolder = path; }

	// Get a variable ptr from a script with the ptr of the script class, the name of the script and the name of the variable, return nullptr if not found
	template<typename T>
	inline T* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return nullptr;
		}

		const auto instance = m_scriptInstances[scriptName];
		if (!instance->m_variables.contains(variableName)) {
			std::cout << "Failed to get variable " << variableName << " of class " << scriptName << std::endl;
			return nullptr;
		}

		const auto getter = instance->m_variables.at(variableName).getterMethod;
		if (getter == nullptr)
			return nullptr;
		return static_cast<T*>(getter(scriptComponent));
	}

	// Set a variable value from a script with the ptr of the script class, the name of the script and the name of the variable
	template<typename T>
	inline void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, T* value)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return;
		}

		const auto instance = m_scriptInstances[scriptName];
		if (!instance->m_variables.contains(variableName)) 
		{
			std::cout << "Failed to get variable " << variableName << " of class " << scriptName << std::endl;
			return;
		}

		const auto setter = instance->m_variables.at(variableName).setterMethod;
		setter(scriptComponent, value);
	}

	// Get all the variables data from a script, first is the name of the variable, second is the struct Variable whic contains the getter and setter and the type name
	inline std::unordered_map<std::string, Variable> GetAllScriptVariablesInfo(const std::string& scriptName)
	{
		if (!m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return {};
		}

		const auto instance = m_scriptInstances[scriptName];
		return instance->m_variables;
	}

	// Get all the methods data from a script with the name of the script class, first is the name of the method, second is the function ptr of the method which will call the method
	inline std::unordered_map<std::string, CallMethod> GetAllScriptMethodsInfo(const std::string& scriptName)
	{
		if (!m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return {};
		}

		const auto instance = m_scriptInstances[scriptName];
		return instance->m_methods;
	}

	// Get all the variables ptr from a script with the name of the script class
	inline std::unordered_map<std::string, void*> GetAllScriptVariables(void* scriptComponent, const std::string& scriptName)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return {};
		}

		const auto instance = m_scriptInstances[scriptName];

		std::unordered_map<std::string, void*> variables;

		for (const auto& variable : instance->m_variables)
		{
			variables[variable.first] = variable.second.getterMethod(scriptComponent);
		}

		return variables;
	}

	// Call a method from a script with the ptr of the script class, the name of the script and the name of the method
	inline void CallScriptMethod(void* scriptComponent, const std::string& scriptName, const std::string& methodName)
	{
		if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
			std::cout << "Failed to get script instance of " << scriptName << std::endl;
			return;
		}

		const auto instance = m_scriptInstances[scriptName];
		if (!instance->m_methods.contains(methodName)) {
			std::cout << "Failed to get method " << methodName << " of class " << scriptName << std::endl;
			return;
	}

		const auto method = instance->m_methods.at(methodName);
		method(scriptComponent);
}
private:
	static ScriptEngine& CreateScriptEngine();

	void ParseGenFile(const std::filesystem::path& headerPath);

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
	static std::unique_ptr<ScriptEngine> m_instance;

	std::set<std::string> m_typeNames;
	std::filesystem::path m_headerGenFolder;
	std::filesystem::path m_copyPathFolder = std::filesystem::current_path() / "ProjectDLL";

	std::unordered_map<std::string, std::shared_ptr<ScriptInstance>> m_scriptInstances;

	void* m_handle = nullptr;
};
