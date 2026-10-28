// The player executable. Like a Unity player's launcher it contains no game
// code: it forwards straight into the engine module.
#include "IronRing/Player.h"

#if defined(_WIN32)
#include <windows.h>

// Prefer the discrete GPU on hybrid laptops.
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    return UnityMain(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}
#else
int main(int argc, char** argv) {
    return PlayerMain(argc, argv);
}
#endif
