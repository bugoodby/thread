//----------------------------------------------------
// Sleep(0)でコンテキストスイッチすることのテスト
//----------------------------------------------------

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>

volatile LONG g_x = FALSE;

unsigned int _stdcall WorkerThread(void* pParam)
{
	InterlockedExchange(&g_x, TRUE);
	return 0;
}

// Sleep(0)を呼ばない
void func_no_sleep()
{
	HANDLE hThread = (HANDLE)_beginthreadex(
		NULL,				// security
		0,					// stack size
		WorkerThread,		// start_address
		NULL,				// arglist
		CREATE_SUSPENDED,	// initflag
		NULL				// thread addr
		);

	::SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
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
	}
	::QueryPerformanceCounter(&liEnd);

	std::cout << (liEnd.QuadPart - liStart.QuadPart) / (double)liFreq.QuadPart << 's' << std::endl;
}

// Sleep(0)を呼ばないでスピン
void func_spin()
{
	HANDLE hThread = (HANDLE)_beginthreadex(
		NULL,				// security
		0,					// stack size
		WorkerThread,		// start_address
		NULL,				// arglist
		CREATE_SUSPENDED,	// initflag
		NULL				// thread addr
		);

	::SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
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
	long val = 0;
	while (1)
	{
		val += 1;
		if (::InterlockedExchange(&g_x, FALSE) == TRUE) break;
	}
	::QueryPerformanceCounter(&liEnd);

	std::cout << (liEnd.QuadPart - liStart.QuadPart) / (double)liFreq.QuadPart << 's' << std::endl;
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
		::Sleep(0);
		//SwitchToThread();
	}
	::QueryPerformanceCounter(&liEnd);

	std::cout << (liEnd.QuadPart - liStart.QuadPart) / (double)liFreq.QuadPart << 's' << std::endl;
}

int main()
{
//	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	::SetProcessPriorityBoost(::GetCurrentProcess(), TRUE);

	func_sleep();
	func_no_sleep();
	func_spin();

	getchar();
	return 0;
}