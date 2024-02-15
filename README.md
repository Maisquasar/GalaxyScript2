# GalaxyScript

## Overview

GalaxyScript is a powerful scripting library designed for C++ development, particularly for game development. It provides an intuitive and efficient way to script functionality in your C++ projects. Whether you are creating games or simulations, GalaxyScript aims to streamline the scripting process, making it accessible and flexible.

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://en.cppreference.com/w/cpp/20)
[![Build with XMake](https://img.shields.io/badge/Built%20with-XMake-lightgray)](https://xmake.io/)

### Build it yourself

1. Clone the GalaxyScript repository.
   ```bash
   git clone https://github.com/your_username/GalaxyScript.git
   ```

2. Build the library using xmake base on your config e.g for windows with msvc :  
   ```bash
   xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
   xmake
   ```

## Example of script
```c++
#include <set>
#include <iostream>
#include <vector>
#include "Macro.h"
#include "ScriptComponent.h"
#include "ExampleClass.generated.h"

CLASS()
class ExampleClass : public ScriptComponent
{
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
END_CLASS()


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
END_CLASS()

END_FILE()
```

## Documentation

Detailed documentation, including examples and API references, can be found on the [GalaxyScript Wiki](https://github.com/your_username/GalaxyScript/wiki).

## License

This project is licensed under the [MIT License](LICENSE.txt).

## Credits
[Romain Bourgogne](https://github.com/Maisquasar)