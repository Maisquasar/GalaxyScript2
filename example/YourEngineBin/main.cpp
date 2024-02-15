#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32

#include "Example.h"

int Main()
{
	Example example;
	example.RunExample();
	return 0;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//  TODO: Remove Comments To Break on leaks
	// |
	// V
	//_CrtSetBreakAlloc(863);
#endif
	return Main();
}


/*Need:
 * - For the separate project:
 *	- in solution :
 *		- add the engine path to include dir
 *		- add include dir for Generate/Header folder
 *		- HeaderTool to be launched before compilation
 *		- Set Output dir to be on folder Generate
 *	- Use the HeaderTool syntax : CLASS() GENERATED_BODY() PROPERTY() END_CLASS()
 *
 * - For the Engine :
 *	- Base class for components With methods :
 *	    - Internal_GetClassName
 *	    - Internal_GetClassNames
 *	    - Clone
 */
