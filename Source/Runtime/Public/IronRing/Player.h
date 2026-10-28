#pragma once

// Public entry points of the engine module (UnityPlayer.dll / UnityPlayer.so).
// The launcher executable is a few lines that forward to these functions,
// mirroring how a Unity player executable hands control to UnityPlayer.

#if defined(_WIN32)
#  if defined(IR_ENGINE_BUILD)
#    define IR_PLAYER_API __declspec(dllexport)
#  else
#    define IR_PLAYER_API __declspec(dllimport)
#  endif
#else
#  define IR_PLAYER_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Portable entry point. argv is expected to be UTF-8.
IR_PLAYER_API int PlayerMain(int argc, char** argv);

#if defined(_WIN32)
// Same shape as Unity's export: (HINSTANCE, HINSTANCE, LPWSTR, int).
// Re-reads the command line with GetCommandLineW() and forwards to PlayerMain.
IR_PLAYER_API int UnityMain(void* hInstance, void* hPrevInstance, wchar_t* lpCmdLine, int nShowCmd);
#endif

#ifdef __cplusplus
}
#endif
