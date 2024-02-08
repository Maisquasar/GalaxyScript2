#include "ScriptComponent.h"

std::unordered_map<std::string, std::shared_ptr<IComponent>> ComponentHandler::m_components;

std::unordered_map<std::string, Variable> ScriptComponent::GetAllVariableInfo() const
{
	return ScriptEngine::Get()->GetAllScriptVariablesInfo(Internal_GetClassName());
}
