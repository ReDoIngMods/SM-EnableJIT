#include <Windows.h>

#include <cstring>
#include <type_traits>

struct lua_State;

using luaL_newstate_Func = std::add_pointer_t<lua_State*()>;
luaL_newstate_Func gOriginal = nullptr;

static lua_State* HookedNewState() {
    lua_State* L = gOriginal();
    if (!L)
        return nullptr;

	HMODULE luaModule = GetModuleHandleA("lua51.dll");
	if (!luaModule)
		return L;

	using luaopen_jit_Func = std::add_pointer_t<int(lua_State*)>;
	luaopen_jit_Func luaopen_jit = reinterpret_cast<luaopen_jit_Func>(GetProcAddress(luaModule, "luaopen_jit"));
	if (luaopen_jit)
		luaopen_jit(L);
	
    return L;
}

inline static bool IATHook(HMODULE module, const char* targetImport, void* targetFunction, void** originalFunction) {
    BYTE* baseAddress = reinterpret_cast<BYTE*>(module);
    IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(baseAddress);
    IMAGE_NT_HEADERS* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(baseAddress + dosHeader->e_lfanew);

    const IMAGE_DATA_DIRECTORY& importDataDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!importDataDirectory.VirtualAddress)
        return false;

    IMAGE_IMPORT_DESCRIPTOR* desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(baseAddress + importDataDirectory.VirtualAddress);
    for (; desc->Name; ++desc) {
        const DWORD lookupRva = desc->OriginalFirstThunk ? desc->OriginalFirstThunk : desc->FirstThunk;
        IMAGE_THUNK_DATA* lookup = reinterpret_cast<IMAGE_THUNK_DATA*>(baseAddress + lookupRva);
        IMAGE_THUNK_DATA* iat = reinterpret_cast<IMAGE_THUNK_DATA*>(baseAddress + desc->FirstThunk);

        for (; lookup->u1.AddressOfData; ++lookup, ++iat) {
            if (IMAGE_SNAP_BY_ORDINAL(lookup->u1.Ordinal))
                continue;

            IMAGE_IMPORT_BY_NAME* import = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(baseAddress + lookup->u1.AddressOfData);
            if (strcmp(import->Name, targetImport) != 0)
                continue;

            void** slot = reinterpret_cast<void**>(&iat->u1.Function);
            DWORD oldProtect;
            if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &oldProtect))
                return false;

            *originalFunction = reinterpret_cast<void*>(*slot);
            *slot = targetFunction;
            VirtualProtect(slot, sizeof(void*), oldProtect, &oldProtect);
            return true;
        }
    }
 
    return false;
}

static void Attach() {
	HMODULE module = GetModuleHandleA("ScrapMechanic.exe");
	if (!module)
		return;

	if (!IATHook(module, "luaL_newstate", reinterpret_cast<void*>(HookedNewState), reinterpret_cast<void**>(&gOriginal)))
		MessageBoxA(nullptr, "Failed to hook luaL_newstate", "SM-EnableJIT", MB_ICONERROR);
}

static void Detach() {
    HMODULE module = GetModuleHandleA("ScrapMechanic.exe");
    if (!module || !gOriginal)
        return;

    void* current = nullptr;
	IATHook(module, "luaL_newstate", reinterpret_cast<void*>(gOriginal), &current);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        Attach();
    } else if (reason == DLL_PROCESS_DETACH && !reserved) {
        Detach();
    }

    return TRUE;
}