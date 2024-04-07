#include <Windows.h>

// -------------------------
// HOW TO LOAD              |
// -------------------------
//  auto lib = LoadLibraryA("..\\Vixen\\Vixen.dll");
//
//  if (lib == nullptr) {
//	    WORD err = GetLastError();
//	    std::cout << "Load Library Failed: Vixen.dll" << std::endl;
//	    return 1;
//  }
// ** LIB IS LOADED AND CAN BE USED ** 
// -------------------------
// HOW TO USE               |
// -------------------------
// auto pFunc = static_cast<Functor>(GetProcAddress(lib, ""))
// pFunc(); 

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(0, L"Test", L"From unmanaged dll", MB_HELP);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}