#include <set>
#include <iostream>
#include <vector>
#include "Macro.h"
#include "ScriptComponent.h"
#include "ExampleClass.generated.h"

GCLASS()
class ExampleClass : public ScriptComponent {
	GENERATED_BODY()
public:
	ExampleClass();

	GPROPERTY();
	int valueI = 0;

	GPROPERTY(HideInInspector, DisplayName = "Aled");
	double valueD = 0.0;

	GPROPERTY();
	ExampleClass* valueC;

	GPROPERTY();
	class ABCD* value2;

	GPROPERTY();
	std::vector<int> value3;
public:
	GFUNCTION();
	void Method()
	{
		std::cout << "Method from ExampleClass" << std::endl;
	}
};


GCLASS()
class AnotherClass : public ExampleClass
{
	GENERATED_BODY()
public:
	AnotherClass();

	GPROPERTY();
	int value = 0;
public:

	GFUNCTION();
	void Method()
	{
		std::cout << "Method from AnotherClass" << std::endl;
	}

};

END_FILE()