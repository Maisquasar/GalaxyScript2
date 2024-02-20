#include "ScriptComponent.h"

std::unordered_map<std::string, GS::Variable> ScriptComponent::GetAllVariableInfo() const
{
	return GS::ScriptEngine::Get()->GetAllScriptVariablesInfo(Internal_GetClassName());
}

std::unordered_map<std::string, GS::CallMethod> ScriptComponent::GetAllMethodsInfo() const
{
	return GS::ScriptEngine::Get()->GetAllScriptMethodsInfo(Internal_GetClassName());
}
