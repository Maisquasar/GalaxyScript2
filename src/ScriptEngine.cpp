#include "ScriptEngine.h"

#include <iostream>
#include <regex>

#include <fstream>

#include <cpp_serializer/CppSerializer.h>
using namespace CppSer;

std::unique_ptr<ScriptEngine> ScriptEngine::m_instance;

ScriptEngine& ScriptEngine::CreateScriptEngine()
{
	m_instance = std::make_unique<ScriptEngine>();
	return *m_instance;
}

bool ScriptEngine::LoadDLL(std::filesystem::path dllPath)
{
#if defined(_WIN32)
	std::string dllExtension = ".dll";
#elif defined(__linux__)
	std::string dllExtension = ".so";
#elif defined(__APPLE__)
	std::string dllExtension = ".dylib";
#endif

	dllPath = dllPath.generic_string() + dllExtension;

	if (!std::filesystem::exists(dllPath))
	{
		std::cerr << "DLL not found on " << dllPath << std::endl;
		return false;
	}

	const auto dllName = dllPath.filename().stem();

	// Create directories
	std::filesystem::create_directories(m_copyPathFolder);

	const std::filesystem::path pdbPath = dllPath.parent_path() / (dllName.string() + ".pdb");
	const std::filesystem::path libPath = dllPath.parent_path() / (dllName.string() + ".lib");
	const std::filesystem::path lib2Path = dllPath.parent_path() / (dllName.string() + ".dll.a");

	const std::filesystem::path copyDLLPath = m_copyPathFolder / (dllName.string() + dllExtension);
	const std::filesystem::path copyPDBPath = m_copyPathFolder / (dllName.string() + pdbPath.extension().string());
	const std::filesystem::path copyLIBPath = m_copyPathFolder / (dllName.string() + libPath.extension().string());
	const std::filesystem::path copyLIB2Path = m_copyPathFolder / (dllName.string() + ".dll.a");

	// Remove dll and dependent files
	if (std::filesystem::exists(copyDLLPath))
		std::filesystem::remove(copyDLLPath);
	if (std::filesystem::exists(copyPDBPath))
		std::filesystem::remove(copyPDBPath);
	if (std::filesystem::exists(copyLIBPath))
		std::filesystem::remove(copyLIBPath);
	if (std::filesystem::exists(copyLIB2Path))
		std::filesystem::remove(copyLIB2Path);
	// Copy dll and dependent files
	if (std::filesystem::exists(dllPath))
		std::filesystem::copy(dllPath, copyDLLPath);
	if (std::filesystem::exists(pdbPath))
		std::filesystem::copy(pdbPath, copyPDBPath);
	if (std::filesystem::exists(libPath))
		std::filesystem::copy(libPath, copyLIBPath);
	if (std::filesystem::exists(lib2Path))
		std::filesystem::copy(lib2Path, copyLIB2Path);

	// Load DLL
#if defined(_WIN32)
	m_handle = LoadLibrary(copyDLLPath.generic_string().c_str());

#elif defined(__linux__)
	m_handle = dlopen(copyDLLPath.generic_string().c_str(), RTLD_LAZY);
#endif

	std::cout << copyDLLPath.generic_string() << std::endl;
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

	if (!std::filesystem::exists(m_headerGenFolder)) {
		std::cout << "Header Gen Folder " << m_headerGenFolder << " does not exist" << std::endl;
		return false;
	}
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_headerGenFolder))
	{
		if (entry.path().extension() == ".gen")
		{
			ParseGenFile(entry.path());
		}
	}

	return true;
}

void ScriptEngine::FreeDLL() const
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

	do
	{
		const std::string className = parser["Class Name"];

		if (!className.empty())
		{
			// Parse class map
			const auto constructor = GetDLLMethod<Constructor>(m_handle, ("Internal_Create_" + className).c_str());
			if (!constructor)
			{
				std::cout << "Failed to get constructor of class " << className << std::endl;
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
		parser.PushDepth();
	}
	while (parser.GetValueMap().size() != parser.GetCurrentDepth());
}

