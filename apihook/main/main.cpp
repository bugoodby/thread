//----------------------------------------------------
// Sleep(0)でコンテキストスイッチすることのテスト
//----------------------------------------------------

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>
#include "hookapi.h"


volatile LONG g_x = FALSE;

unsigned int _stdcall WorkerThread(void* pParam)
{
	InterlockedExchange(&g_x, TRUE);
	return 0;
}

// Sleep(0)を呼ぶ
void func_sleep()
{
	HANDLE hThread = (HANDLE)_beginthreadex(
		NULL,				// security
		0,					// stack size
		WorkerThread,		// start_address
		NULL,				// arglist
		CREATE_SUSPENDED,	// initflag
		NULL				// thread addr
		);

	::SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
	::SetThreadAffinityMask(hThread, 0x00000001);

	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	::SetThreadAffinityMask(::GetCurrentThread(), 0x00000001);

	::ResumeThread(hThread);
	::CloseHandle(hThread);

	LARGE_INTEGER liFreq = { 0 };
	LARGE_INTEGER liStart = { 0 };
	LARGE_INTEGER liEnd = { 0 };

	::QueryPerformanceFrequency(&liFreq);
	::QueryPerformanceCounter(&liStart);
	while (::InterlockedExchange(&g_x, FALSE) == FALSE)
	{
		Sleep(0);
	}
	::QueryPerformanceCounter(&liEnd);

	std::cout << (liEnd.QuadPart - liStart.QuadPart) / (double)liFreq.QuadPart << 's' << std::endl;
}

long g_sleep0call = 0;
typedef void (*sleep_func)(DWORD);
sleep_func g_sleepFunc = NULL;

void _stdcall MySleep(DWORD dwMilliseconds)
{
	//printf("Sleep Called!\n");
	if ( dwMilliseconds == 0 ) {
		g_sleep0call++;
	} else {
		g_sleepFunc(dwMilliseconds);
	}
	return;
}

int main()
{
	g_sleepFunc = (sleep_func)HookAPI("Kernel32.dll", "Sleep", MySleep);
	func_sleep();
	HookAPI("Kernel32.dll", "Sleep", g_sleepFunc);
	
	printf("Sleep() called: %u\n", g_sleep0call);
	getchar();
	return 0;
}
