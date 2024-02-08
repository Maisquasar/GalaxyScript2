#include "ScriptEngine.h"

#include <iostream>
#include <regex>

#include <fstream>

#include <cpp_serializer/CppSerializer.h>
using namespace CppSer;

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

	const auto path = dllPath.parent_path() / "Headers";
	if (!std::filesystem::exists(path))
		return false;
	// Generated/file.dll so parent is Generated
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.path().extension() == ".gen")
		{
			ParseGenFile(entry.path());
		}
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

void ScriptEngine::ParseGenFile(const std::filesystem::path& headerPath)
{
	Parser parser(headerPath);
	const std::string className = parser["Class Name"];

	using ScriptConstructor = void* (*)();
	const auto constructor = GetDLLMethod<ScriptConstructor>(m_handle, ("Internal_Create_" + className).c_str());

	if (!constructor)
	{
		std::cerr << "Failed to get constructor" << std::endl;
		return;
	}
	ComponentHandler::RegisterScriptComponent(static_cast<ScriptComponent*>(constructor()));
	const auto scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();

	const int propertySize = parser["Property Size"].As<int>();
	for (int i = 0; i < propertySize; i++) {


		parser.PushDepth();
		const std::string typeName = parser["Type"];
		const std::string varName = parser["Name"];

		Property property;
		property.propertyName = varName;
		property.propertyType = typeName;

		Variable variable;
		variable.property = property;
		variable.getterMethod = GetDLLMethod<GetterMethod>(m_handle, ("Internal_Get_" + className + "_" + property.propertyName).c_str());
		variable.setterMethod = GetDLLMethod<SetterMethod>(m_handle, ("Internal_Set_" + className + "_" + property.propertyName).c_str());

		scriptInstance->m_variables[property.propertyName] = variable;
	}
}

