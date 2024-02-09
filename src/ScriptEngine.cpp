#include "ScriptEngine.h"

#include <iostream>
#include <regex>

#include <fstream>

#include <cpp_serializer/CppSerializer.h>
using namespace CppSer;

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
	return *m_instance;
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

	const std::filesystem::path pdbPath = dllPath.parent_path() / (dllName.string() + ".pdb");
	const std::filesystem::path libPath = dllPath.parent_path() / (dllName.string() + ".lib");
	const std::filesystem::path lib2Path = dllPath.parent_path() / (dllName.string() + ".dll.a");

	const std::filesystem::path copyDLLPath = m_copyPath / (dllName.string() + dllExtension);
	const std::filesystem::path copyPDBPath = m_copyPath / (dllName.string() + pdbPath.extension().string());
	const std::filesystem::path copyLIBPath = m_copyPath / (dllName.string() + libPath.extension().string());
	const std::filesystem::path copyLIB2Path = m_copyPath / (dllName.string() + ".dll.a");

	// Remove dll and dependent files
	std::filesystem::remove(copyDLLPath);
	std::filesystem::remove(copyPDBPath);
	std::filesystem::remove(copyLIBPath);
	std::filesystem::remove(copyLIB2Path);
	// Copy dll and dependent files
	std::filesystem::copy(dllPath, copyDLLPath, std::filesystem::copy_options::overwrite_existing);
	std::filesystem::copy(pdbPath, copyPDBPath, std::filesystem::copy_options::overwrite_existing);
	std::filesystem::copy(libPath, copyLIBPath, std::filesystem::copy_options::overwrite_existing);
	std::filesystem::copy(lib2Path, copyLIB2Path, std::filesystem::copy_options::overwrite_existing);

	// Load DLL
#if defined(_WIN32)
	m_handle = LoadLibrary(copyDLLPath.generic_string().c_str());
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

	const auto constructor = GetDLLMethod<Constructor>(m_handle, ("Internal_Create_" + className).c_str());
	if (!constructor)
	{
		std::cerr << "Failed to get constructor" << std::endl;
		return;
	}

	const auto scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();
	scriptInstance->m_constructor = constructor;

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

	const int methodSize = parser["Method Size"].As<int>();
	for (int i = 0; i < methodSize; i++)
	{
		parser.PushDepth();

		const std::string methodName = parser["Name"];

		scriptInstance->m_methods[methodName] = GetDLLMethod<CallMethod>(m_handle, ("Internal_Call_" + className + "_" + methodName).c_str());
	}
}

