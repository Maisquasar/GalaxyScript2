#include "ScriptEngine.h"

#include <iostream>
#include <regex>

#include <fstream>

#include <cpp_serializer/CppSerializer.h>
using namespace CppSer;
using namespace GS;

std::unique_ptr<ScriptEngine> ScriptEngine::m_instance;

ScriptEngine& ScriptEngine::CreateScriptEngine()
{
	m_instance = std::make_unique<ScriptEngine>();
	return *m_instance;
}

bool ScriptEngine::LoadDLL(std::filesystem::path dllPath, bool copyDll /*= true*/)
{
	m_headerFilesName.clear();

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

	const std::string dllName = dllPath.filename().stem().string();

	const std::filesystem::path copyDLLPath = copyDll ? m_copyPathFolder / (dllName + dllExtension) : dllPath;
	if (copyDll)
	{
		// Create directories
		std::filesystem::create_directories(m_copyPathFolder);

		const std::filesystem::path pdbPath = dllPath.parent_path() / (dllName + ".pdb");
		const std::filesystem::path libPath = dllPath.parent_path() / (dllName + ".lib");
		const std::filesystem::path lib2Path = dllPath.parent_path() / (dllName + ".dll.a");

		const std::filesystem::path copyPDBPath = m_copyPathFolder / (dllName + pdbPath.extension().string());
		const std::filesystem::path copyLIBPath = m_copyPathFolder / (dllName + libPath.extension().string());
		const std::filesystem::path copyLIB2Path = m_copyPathFolder / (dllName + ".dll.a");

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
	}

	// Load DLL
#if defined(_WIN32)
	m_handle = LoadLibrary(copyDLLPath.generic_string().c_str());
#elif defined(__linux__) || defined(__APPLE__)
	m_handle = dlopen(copyDLLPath.generic_string().c_str(), RTLD_LAZY);
#endif

	std::cout << copyDLLPath.generic_string() << "\n";
	if (!m_handle)
	{
#if defined(_WIN32)
		const DWORD errorMessageID = ::GetLastError();
		if (errorMessageID != 0) {
			LPSTR messageBuffer = nullptr;

			const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			std::cerr << std::string(messageBuffer, size) << "\n";

			LocalFree(messageBuffer);
		}
#elif defined(__linux__) || defined(__APPLE__)
		std::cerr << dlerror() << "\n";
#endif
		return false;
	}

	if (!std::filesystem::exists(m_headerGenFolder)) {
		std::cout << "Header Gen Folder " << m_headerGenFolder << " does not exist" << "\n";
		return false;
	}

	UpdateHeaders(true);

	return true;
}

void ScriptEngine::UpdateHeaders(bool force /*= false*/)
{
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_headerGenFolder))
	{
		std::string filename = entry.path().filename().stem().string();
		if (entry.path().extension() == ".gen" && (!force && m_headerFilesName.contains(filename) || force))
		{
			m_headerFilesName.insert(filename);
			ParseGenFile(entry.path());
		}
	}
}

void ScriptEngine::AddHeaders(const std::string& headerName)
{
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_headerGenFolder))
	{
		if (entry.path().filename().stem().string() != headerName)
			continue;
		std::string filename = entry.path().filename().stem().string();
		if (entry.path().extension() == ".gen")
		{
			m_headerFilesName.insert(filename);
			ParseGenFile(entry.path());
		}
	}
}

void ScriptEngine::FreeDLL() const
{
	if (!m_handle)
		return;
#if defined(_WIN32)
	FreeLibrary((HMODULE)m_handle);
#elif defined(__linux__) || defined(__APPLE__)
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
				Property property;

				parser.PushDepth();

				const int argsSize = parser["Argument Size"].As<int>();
				for (int j = 0; j < argsSize; j++) {
					property.args.push_back(parser["Argument " + std::to_string(j)]);
				}

				property.name = parser["Name"];
				property.type = parser["Type"];

				Variable variable;
				variable.property = property;
				variable.getterMethod = GetDLLMethod<GetterMethod>(m_handle, ("Internal_Get_" + className + "_" + property.name).c_str());
				variable.setterMethod = GetDLLMethod<SetterMethod>(m_handle, ("Internal_Set_" + className + "_" + property.name).c_str());

				scriptInstance->m_variables[property.name] = variable;
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
	while (parser.GetValueMap().size() > parser.GetCurrentDepth());
}




void* ScriptEngine::GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
{
	if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
		std::cout << "Failed to get script instance of " << scriptName << std::endl;
		return nullptr;
	}

	const auto instance = m_scriptInstances[scriptName];
	if (!instance->m_variables.contains(variableName)) {
		std::cout << "Failed to get variable " << variableName << " of class " << scriptName << std::endl;
		return nullptr;
	}

	const auto getter = instance->m_variables.at(variableName).getterMethod;
	if (getter == nullptr)
		return nullptr;
	return getter(scriptComponent);
}

void ScriptEngine::SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value)
{
	if (!scriptComponent || !m_scriptInstances.contains(scriptName)) {
		std::cout << "Failed to get script instance of " << scriptName << std::endl;
		return;
	}

	const auto instance = m_scriptInstances[scriptName];
	if (!instance->m_variables.contains(variableName))
	{
		std::cout << "Failed to get variable " << variableName << " of class " << scriptName << std::endl;
		return;
	}

	const auto setter = instance->m_variables.at(variableName).setterMethod;
	setter(scriptComponent, value);
}
