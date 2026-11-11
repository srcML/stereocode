// Entry points (Free-Function-Stereotypes wiki, main): main and the entry points of Windows programs

int run(int value);

// @stereotype main
int wmain(int argc, wchar_t* argv[]) { return run(argc); }

// @stereotype main
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int show) { return run(show); }

// @stereotype main
int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ PWSTR commandLine, _In_ int show) { return run(show); }

// @stereotype main
int _tmain(int argc, _TCHAR* argv[]) { return run(argc); }

// @stereotype main
int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE previous, LPTSTR commandLine, int show) { return run(show); }

// A name that only starts with main names no entry point
// @stereotype property wrapper
int mainLoop(int ticks) { return run(ticks); }
