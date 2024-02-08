#include <any>
#include <iostream>

#include "ScriptComponent.h"
#include "ScriptEngine.h"

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32

void SaveValues(ScriptComponent* scriptComponent);
void LoadValues(ScriptComponent* scriptComponent);

int Main()
{
	auto scriptEngine = ScriptEngine::Get();

	//TODO:
	const char* dllPath = "D:/Code/Moteurs/ExampleProject/Generate/ExampleProject.dll";
	const bool loaded = scriptEngine->LoadDLL(dllPath);

	if (!loaded)
		return 1;

	// Create the script
	auto scriptComponent = std::dynamic_pointer_cast<ScriptComponent>(ComponentHandler::CreateWithComponentName("ExampleClass"));
	if (!scriptComponent)
		return 1;

	auto variables = scriptComponent->GetAllVariableInfo();

	std::cout << "Variables of " << scriptComponent->Internal_GetClassName() << " :" << std::endl;
	for (auto& variable : variables)
	{
		auto property = variable.second.property;

		if (property.propertyType == "std::vector<std::string>")
		{
			scriptComponent->SetVariable(property.propertyName, std::vector<std::string> {"Hello", "World"});
		}

		std::cout << '\t' << variable.second.property.propertyType << " " << variable.second.property.propertyName << std::endl;
	}

	getchar();

	//TODO Get all variables value
	SaveValues(scriptComponent.get());

	scriptComponent.reset();
	scriptEngine->FreeDLL();
	scriptEngine->LoadDLL(dllPath);

	//TODO Set all variables value
	scriptComponent = std::dynamic_pointer_cast<ScriptComponent>(ComponentHandler::CreateWithComponentName("ExampleClass"));
	LoadValues(scriptComponent.get());

	const auto names = scriptComponent->Internal_GetClassNames();
	std::cout << std::endl;
	std::cout << "Inherit class names" << std::endl;
	for (auto& name : names)
		std::cout << name << std::endl;
	std::cout << std::endl;

	return 0;
}
#define SAVE_TYPE(x) if (property.propertyType == #x)\
	{\
		const auto value = *scriptComponent->GetVariable<x>(property.propertyName);\
		s_values[property.propertyName] = value;\
}

#define LOAD_TYPE(x) if (property.propertyType == #x)\
	{\
		const auto value = std::any_cast<x>(s_values[property.propertyName]);\
		scriptComponent->SetVariable(property.propertyName, value);\
	}

std::unordered_map<std::string, std::any> s_values;
void SaveValues(ScriptComponent* scriptComponent)
{
	const auto variables = scriptComponent->GetAllVariableInfo();
	std::cout << "Before Values" << std::endl;
	for (auto& variable : variables)
	{
		auto property = variable.second.property;
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
		auto property = variable.second.property;
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

int main(int argc, char** argv)
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//  TODO: Remove Comments To Break on leaks
	// |
	// V
	//_CrtSetBreakAlloc(863);
#endif
	return Main();
}


/*Need:
 * - For the separate project:
 *	- in solution :
 *		- add the engine path to include dir
 *		- add include dir for Generate/Header folder
 *		- HeaderTool to be launched before compilation
 *		- Set Output dir to be on folder Generate
 *	- Use the HeaderTool syntax : CLASS() GENERATED_BODY() PROPERTY() END_CLASS()
 *
 * - For the Engine :
 *	- Base class for components With methods :
 *	    - Internal_GetClassName
 *	    - Internal_GetClassNames
 *	    - Clone
 */
