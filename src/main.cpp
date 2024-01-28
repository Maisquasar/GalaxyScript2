#include <iostream>

#include "ScriptComponent.h"
#include "ScriptEngine.h"

int main(int argc, char** argv)
{
	auto scriptEngine = ScriptEngine::Get();

	//TODO:
	//scriptEngine->AddHeader("D:/Code/Moteurs/ExampleProject/Assets/ExampleClass.h");
	//scriptEngine->LoadDLL("D:/Code/Moteurs/ExampleProject/Generate/ExampleProject.dll");

	auto scriptComponent = std::dynamic_pointer_cast<ScriptComponent>(ComponentHandler::CreateWithComponentName("ExampleClass"));
	if (scriptComponent)
	{
		auto variables = scriptComponent->GetAllVariableInfo();

		std::cout << "Variables of " << scriptComponent->GetComponentName() << " :" << std::endl;
		for (auto& variable : variables)
		{
			std::cout << '\t' << variable.second.property.propertyType << " " << variable.second.property.propertyName << std::endl;
		}
	}
	return 0;
}
