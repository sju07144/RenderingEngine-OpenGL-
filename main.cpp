#include "window.h"

int main()
{
	Window* window = new Window(800, 600, "Outline Drawing");
	window->Initialize();
	window->Run();
	window->Shutdown();
	delete window;
}