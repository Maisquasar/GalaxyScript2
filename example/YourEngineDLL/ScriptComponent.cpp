#include "ScriptComponent.h"

std::unordered_map<std::string, Variable> ScriptComponent::GetAllVariableInfo() const
{
	return ScriptEngine::Get()->GetAllScriptVariablesInfo(Internal_GetClassName());
}

std::unordered_map<std::string, CallMethod> ScriptComponent::GetAllMethodsInfo() const
{
	return ScriptEngine::Get()->GetAllScriptMethodsInfo(Internal_GetClassName());
}
