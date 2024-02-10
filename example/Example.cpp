#include "Example.h"


#include "../include/ScriptEngine.h"
#include "ScriptComponent.h"

#include <iostream>
#include <vector>
#include <any>
#include <ranges>
#include <stdexcept>

void SaveValues(ScriptComponent* scriptComponent);
void LoadValues(ScriptComponent* scriptComponent);

void Example::RunExample()
{
	const auto scriptEngine = ScriptEngine::Get();

	// DLL ALWAYS NEED TO BE THE SAME BUILD MODE AS THE LIB
	#ifdef _WIN32
	const char* dllPath = "D:/Code/Moteurs/ExampleProject/Generate/ExampleProject";
	#else
	const char* dllPath = "/home/uwu/Documents/ExampleProject/Generate/ExampleProject";
	#endif
	const bool loaded = scriptEngine->LoadDLL(dllPath);

	if (!loaded)
		throw std::runtime_error("Failed to load DLL");

	// Create the script
	std::shared_ptr<ScriptComponent> scriptComponent = scriptEngine->CreateWithClassName<ScriptComponent>("ExampleClass");
	if (!scriptComponent)
		return;

	const auto variables = scriptComponent->GetAllVariableInfo();

	std::cout << "Variables of " << scriptComponent->Internal_GetClassName() << " :" << std::endl;
	for (const auto& variable : variables | std::views::values)
	{
		auto property = variable.property;

		if (property.propertyType == "std::vector<std::string>")
		{
			scriptComponent->SetVariable(property.propertyName, std::vector<std::string> {"Hello", "World"});
		}

		std::cout << '\t' << variable.property.propertyType << " " << variable.property.propertyName << std::endl;
	}
	std::cout << std::endl;

	const auto methods = scriptComponent->GetAllMethodsInfo();
	std::cout << "Methods of " << scriptComponent->Internal_GetClassName() << " :" << std::endl;

	for (const auto& method : methods)
	{
		std::cout << '\t' << method.first << std::endl;
	}
	std::cout << std::endl;

	// Call a class method:
	scriptComponent->CallMethodByName("Method");

	std::cout << "Press enter to HotReload" << std::endl;
	getchar();

	//TODO Get all variables value
	SaveValues(scriptComponent.get());

	scriptComponent.reset();
	scriptEngine->FreeDLL();
	scriptEngine->LoadDLL(dllPath);

	//TODO Set all variables value
	scriptComponent = scriptEngine->CreateWithClassName<ScriptComponent>("ExampleClass");
	LoadValues(scriptComponent.get());

	// Call a class method:
	scriptComponent->CallMethodByName("Method");

	const auto names = scriptComponent->Internal_GetClassNames();
	std::cout << std::endl;
	std::cout << "Inherit class names" << std::endl;
	for (auto& name : names)
		std::cout << '\t' << name << std::endl;
	std::cout << std::endl;
}

#define SAVE_TYPE(x) if (_property.propertyType == #x)\
	{\
		const auto value = scriptComponent->GetVariable<x>(_property.propertyName);\
		if (value != nullptr)\
		{\
			s_values[_property.propertyName] = *value;\
		}\
}

#define LOAD_TYPE(x) if (_property.propertyType == #x)\
	{\
		if (s_values.contains(_property.propertyName))\
		{\
			const auto value = std::any_cast<x>(s_values[_property.propertyName]);\
			scriptComponent->SetVariable(_property.propertyName, value);\
		}\
	}

std::unordered_map<std::string, std::any> s_values;
void SaveValues(ScriptComponent* scriptComponent)
{
	const auto variables = scriptComponent->GetAllVariableInfo();
	std::cout << "Before Values" << std::endl;
	for (auto& variable : variables)
	{
		auto _property = variable.second.property;
		SAVE_TYPE(bool)
else SAVE_TYPE(char)
else SAVE_TYPE(int)
else SAVE_TYPE(float)
		else SAVE_TYPE(double)
		else SAVE_TYPE(std::string)
		else SAVE_TYPE(std::vector<bool>)
		else SAVE_TYPE(std::vector<char>)
		else SAVE_TYPE(std::vector<int>)
		else SAVE_TYPE(std::vector<float>)
		else SAVE_TYPE(std::vector<double>)
		else SAVE_TYPE(std::vector<std::string>)
	}
}

void LoadValues(ScriptComponent* scriptComponent)
{
	const auto variables = scriptComponent->GetAllVariableInfo();
	std::cout << "After Values" << std::endl;
	for (auto& variable : variables)
	{
		auto _property = variable.second.property;
		LOAD_TYPE(bool)
else LOAD_TYPE(char)
else LOAD_TYPE(int)
else LOAD_TYPE(float)
		else LOAD_TYPE(double)
		else LOAD_TYPE(std::string)
		else LOAD_TYPE(std::vector<bool>)
		else LOAD_TYPE(std::vector<char>)
		else LOAD_TYPE(std::vector<int>)
		else LOAD_TYPE(std::vector<float>)
		else LOAD_TYPE(std::vector<double>)
		else LOAD_TYPE(std::vector<std::string>)
	}
}

EXAMPLE_API void RandomMethod()
{
	std::cout << "RandomMethod Called from Script" << std::endl;
}
