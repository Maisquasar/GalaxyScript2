#include "ScriptComponent.h"

std::vector<std::shared_ptr<IComponent>> ComponentHandler::m_components;

std::unordered_map<std::string, Variable> ScriptComponent::GetAllVariableInfo() const
{
	return ScriptEngine::Get()->GetAllScriptVariablesInfo(GetComponentName());
}
