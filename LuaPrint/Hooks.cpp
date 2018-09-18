#include "stdafx.h"
#include "Hooks.h"

Hooks::_DoString Hooks::oDoString;
Hooks::_GetTop Hooks::oGetTop;
Global Hooks::_Global;
Hooks::Hooks(Global _g)
{
	Hooks::_Global = _g;
}

Hooks::~Hooks()
{
}

INT(CALLBACK h_Dostring)(lua_State *L, CONST CHAR *s, size_t size)
{
	// Log lua functions.
	if (!(s[0] == '\0'))
	{
		std::cout << "[LUA] : Exec ->" << std::hex << s << std::endl;
	}

	// Return to original function.
	return Hooks::oDoString(L, s, size);
}

INT(CALLBACK h_GetTop)(lua_State *L)
{
	// Update lua_State pointer.
	if (Hooks::_Global.luaState != L) {
		std::cout << "[LUA] : Lua State changed " << std::hex << Hooks::_Global.luaState << " -> " << std::hex << L << std::endl;
		Hooks::_Global.luaState = L;
	}
	return Hooks::oGetTop(L);
}

bool Hooks::hook_function(PVOID & t1, PBYTE t2, const char * s)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&t1, t2);
	if (DetourTransactionCommit() != NO_ERROR) {
		std::cout << "[Hook] : Failed to hook " << s << std::endl;
		return false;
	}
	else {
		std::cout << "[Hook] : Successfully hooked " << s << std::endl;
		return true;
	}
}

bool Hooks::setHooks()
{
	bool result = true;
	Hooks::oDoString = (Hooks::_DoString)_Global.addresses["DoString"];
	Hooks::oGetTop = (Hooks::_GetTop)_Global.addresses["GetTop"];
	result &= Hooks::hook_function((PVOID &)oDoString, (PBYTE)h_Dostring, _Global.doString[2].c_str());
	result &= Hooks::hook_function((PVOID &)oGetTop, (PBYTE)h_GetTop, _Global.getTop[2].c_str());
	return result;
}
