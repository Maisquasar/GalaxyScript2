#include "ScriptEngine.h"

#include <iostream>
#include <regex>

#include <fstream>

#include "ScriptComponent.h"

std::unique_ptr<ScriptEngine> ScriptEngine::m_instance;

ScriptEngine::ScriptEngine()
{

}

ScriptEngine::~ScriptEngine()
{
	std::cout << "ScriptEngine destroyed" << std::endl;
}

ScriptEngine& ScriptEngine::CreateScriptEngine()
{
	m_instance = std::make_unique<ScriptEngine>();
	m_instance->m_typeNames =
	{
		"bool",
		"int",
		"float",
		"double",
		"string",
	};

	return *m_instance;
}

void ScriptEngine::AddType(const std::string& typeName)
{
	if (m_typeNames.contains(typeName))
		return;
	m_typeNames.insert(typeName);
}

bool ScriptEngine::LoadDLL(const std::filesystem::path& dllPath)
{
	if (!std::filesystem::exists(dllPath)) {
		std::cerr << "File not found: " << dllPath << std::endl;
		return false;
	}

#if defined(_WIN32)
	std::string dllExtension = ".dll";
#elif defined(__linux__)
	std::string dllExtension = ".so";
#elif defined(__APPLE__)
	std::string dllExtension = ".dylib";
#endif

	const auto dllName = dllPath.filename().stem();

	// Create directories
	std::filesystem::create_directories(m_copyPath);

	const std::filesystem::path copyDllPath = m_copyPath / (dllName.string() + dllExtension);

	// Copy DLL to copy directory
	std::filesystem::copy(dllPath, copyDllPath, std::filesystem::copy_options::overwrite_existing);

	// Load DLL
#if defined(_WIN32)
	m_handle = LoadLibrary(copyDllPath.generic_string().c_str());
#elif defined(__linux__)
	handle = dlopen(DllPath.generic_string().c_str(), RTLD_LAZY);
#endif

	if (!m_handle)
	{
#if defined(_WIN32)
		const DWORD errorMessageID = ::GetLastError();
		if (errorMessageID != 0) {
			LPSTR messageBuffer = nullptr;

			const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			std::cerr << std::string(messageBuffer, size) << std::endl;

			LocalFree(messageBuffer);
		}
#elif defined(__linux__)
		std::cerr << dlerror() << std::endl;
#endif
		return false;
	}

	for (auto& header : m_headers)
	{
		ParseHeaderFile(header);
	}

	return true;
}

void ScriptEngine::FreeDLL()
{
	if (!m_handle)
		return;
#if defined(_WIN32)
	FreeLibrary((HMODULE)m_handle);
#elif defined(__linux__)
	dlclose(m_handle);
#endif
}

void ScriptEngine::AddHeader(const std::filesystem::path& headerPath)
{
	m_headers.push_back(headerPath);
}

void ScriptEngine::ParseHeaderFile(const std::filesystem::path& headerPath)
{
	std::string className = headerPath.filename().stem().string();

	using ScriptConstructor = void* (*)();
	auto constructor = GetDLLMethod<ScriptConstructor>(m_handle, ("Create_" + className).c_str());

	if (!constructor)
	{
		std::cerr << "Failed to get constructor" << std::endl;
		return;
	}
	ComponentHandler::RegisterScriptComponent(static_cast<ScriptComponent*>(constructor()));

	auto scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();

	std::ifstream file(headerPath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << headerPath << std::endl;
		return;
	}

	std::string content((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	std::smatch match;

	std::regex propertyRegex(R"(PROPERTY\(\)\s*(?:class\s+|struct\s+)?((?:\w+::)*\w+(?:\s*<[^;<>]*(?:<(?:[^;<>]*)>)*[^;<>]*>)?\s*[*&]?)\s+(\w+)\s*(?:=\s*[^;]*)?;)");
	auto begin = std::sregex_iterator(content.begin(), content.end(), propertyRegex);
	auto end = std::sregex_iterator();

	for (std::sregex_iterator i = begin; i != end; ++i) {
		match = *i;
		std::string typeName = match[1].str(); // Type of the variable, including namespace and template if any
		std::string varName = match[2].str(); // Name of the variable

		Property property;
		property.propertyName = varName;
		property.propertyType = typeName;

		Variable variable;
		variable.property = property;
		variable.getterMethod = GetDLLMethod<GetterMethod>(m_handle, ("Get_" + className + "_" + property.propertyName).c_str());
		variable.setterMethod = GetDLLMethod<SetterMethod>(m_handle, ("Set_" + className + "_" + property.propertyName).c_str());

		scriptInstance->m_variables[property.propertyName] = variable;
	}

}

