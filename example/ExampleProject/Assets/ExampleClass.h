#include <set>
#include <iostream>
#include <vector>
#include "Macro.h"
#include "ScriptComponent.h"
#include "ExampleClass.generated.h"

CLASS()
class ExampleClass : public ScriptComponent {
	GENERATED_BODY()
public:
	ExampleClass();

	PROPERTY();
	int valueI = 0;

	PROPERTY(HideInInspector, DisplayName = "Aled");
	double valueD = 0.0;

	PROPERTY();
	ExampleClass* valueC;

	PROPERTY();
	class ABCD* value2;

	PROPERTY();
	std::vector<int> value3;
public:
	FUNCTION();
	void Method()
	{
		std::cout << "Method from ExampleClass" << std::endl;
	}
};


CLASS()
class AnotherClass : public ExampleClass
{
	GENERATED_BODY()
public:
	AnotherClass();

	PROPERTY();
	int value = 0;
public:

	FUNCTION();
	void Method()
	{
		std::cout << "Method from AnotherClass" << std::endl;
	}

};

END_FILE()