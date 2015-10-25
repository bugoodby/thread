#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <imagehlp.h>
#pragma comment(lib,"imagehlp.lib")

void* HookAPI(const char* szModuleName, const char* szFuncName, void* pNewFuncPtr)
{
	HMODULE hModule = GetModuleHandleA(szModuleName);
	if (!hModule) {
		return NULL;
	}

	ULONG_PTR ulBase = (ULONG_PTR)hModule;
	ULONG ulSize = 0;
	PIMAGE_IMPORT_DESCRIPTOR pImgDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
	for(; pImgDesc->Name != 0; pImgDesc++)
	{
		PIMAGE_THUNK_DATA pFirstThunk = (PIMAGE_THUNK_DATA)(ulBase + pImgDesc->FirstThunk);
		PIMAGE_THUNK_DATA pOrgFirstThunk = (PIMAGE_THUNK_DATA)(ulBase + pImgDesc->OriginalFirstThunk);
		
		for(; pFirstThunk->u1.Function != 0; pFirstThunk++, pOrgFirstThunk++)
		{
			if (IMAGE_SNAP_BY_ORDINAL(pOrgFirstThunk->u1.Ordinal)) {
				continue;
			}
			PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)(ulBase + pOrgFirstThunk->u1.AddressOfData);
			//printf("%s: %s\n", (char*)(ulBase + pImgDesc->Name), (char*)pImportName->Name);
			if (_stricmp((const char*)pImportName->Name, szFuncName) == 0)
			{
				DWORD dwOldProtect = 0;
				if (!VirtualProtect(&pFirstThunk->u1.Function, sizeof(pFirstThunk->u1.Function), PAGE_READWRITE, &dwOldProtect)) {
					return NULL;
				}

				void* pOrgFunc = (void*)pFirstThunk->u1.Function;
				WriteProcessMemory(GetCurrentProcess(), &pFirstThunk->u1.Function, &pNewFuncPtr, sizeof(pFirstThunk->u1.Function), NULL);
				pFirstThunk->u1.Function = (ULONG_PTR)pNewFuncPtr;

				VirtualProtect(&pFirstThunk->u1.Function, sizeof(pFirstThunk->u1.Function), dwOldProtect, &dwOldProtect);
				return pOrgFunc;
			}
		}
	}
	return NULL;
}
