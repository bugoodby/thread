#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>
#include <tlhelp32.h>

//#define PRINTF printf
#define PRINTF 

class Stopwatch 
{
public:
	Stopwatch() {
		::QueryPerformanceFrequency(&m_freq);
	}
	void Start() {
		::QueryPerformanceCounter(&m_start);
	}
	long long Now() {
		LARGE_INTEGER now;
		::QueryPerformanceCounter(&now);
		return (((now.QuadPart - m_start.QuadPart) * 1000000) / m_freq.QuadPart);
	}
	
private:
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_start;
};


int GetThreadList( std::list<DWORD> &list )
{
	int count = 0;
	list.clear();
	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if ( hSnapshot != INVALID_HANDLE_VALUE ) {
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);
		if ( ::Thread32First(hSnapshot, &te32) ) {
			do {
				list.push_back(te32.th32ThreadID);
				printf( "%d: %6u, %6u, %6u, %6u, %6u, %08X\n", 
					++count, te32.cntUsage, te32.th32OwnerProcessID, te32.th32ThreadID, 
					te32.tpBasePri, te32.tpDeltaPri, te32.dwFlags );
			} while ( ::Thread32Next(hSnapshot, &te32) );
		}
		::CloseHandle(hSnapshot);
	}
	return count;
}


unsigned int __stdcall WorkerThread_H(void *param)
{
	printf("ThreadH start - tid=%u\n", ::GetCurrentThreadId());
	
	double val = 0.0;
	
	std::list<DWORD> threadListBefore;
	GetThreadList(threadListBefore);
	
	{
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		PRINTF("ThreadH CPU=%u Priority=%d tid=%u\n",
			::GetCurrentProcessorNumber(),
			::GetThreadPriority(::GetCurrentThread()),
			::GetCurrentThreadId());
		
		for (int i = 0; i < 100000000; i++)
		{
			double x = 1;
			val += x;
		}
	}
	
	printf("ThreadH end\n");
	return (int)val;
}


unsigned int __stdcall WorkerThread_L(void *param)
{
	printf("ThreadL start - tid=%u\n", ::GetCurrentThreadId());

	double val = 0.0, val2 = 0.0;
	
	{
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);
		PRINTF("ThreadL CPU=%u Priority=%d tid=%u\n",
			::GetCurrentProcessorNumber(),
			::GetThreadPriority(::GetCurrentThread()),
			::GetCurrentThreadId());
		
		for (int i = 0; i < 1000000000; i++)
		{
			double x = 1;
			val += x;
			val2 = val / x;
		}
	}
	
	printf("ThreadL end\n");
	return (int)val2;
}



void testfunc()
{
	printf("testfunc start\n");
	
	const unsigned int thread_num = 2;
	HANDLE hThread_L[thread_num] = { NULL };
	unsigned int nTID_L[thread_num] = { 0 };
	
	for ( int i = 0; i < thread_num; i++ ) {
		// 低優先度スレッド作成
		hThread_L[i] = (HANDLE)_beginthreadex(
			NULL,				// security
			0,					// stack size
			WorkerThread_L,		// start_address
			NULL,				// arglist
			CREATE_SUSPENDED,	// initflag
			&nTID_L[i]			// thread addr
			);
	}
	for ( int i = 0; i < thread_num; i++ ) {
		SetThreadPriority(hThread_L[i], THREAD_PRIORITY_LOWEST);
	}
	for ( int i = 0; i < thread_num; i++ ) {
		ResumeThread(hThread_L[i]);
	}
	
	for ( int i = 0; i < 1; i++ )
	{
		Sleep(100);
		
		HANDLE hThread = NULL;
		unsigned int nThreadID = 0;
		
		// 高優先度スレッド作成
		hThread = (HANDLE)_beginthreadex(
			NULL,				// security
			0,					// stack size
			WorkerThread_H,		// start_address
			NULL,				// arglist
			CREATE_SUSPENDED,	// initflag
			&nThreadID			// thread addr
			);
		
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
		ResumeThread(hThread);
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	
	::WaitForMultipleObjects(thread_num, hThread_L, TRUE, INFINITE);
	
	for ( int i = 0; i < thread_num; i++ ) {
		CloseHandle(hThread_L[i]);
	}
	
	printf("testfunc end\n");
}


int main()
{
	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	::SetProcessPriorityBoost(::GetCurrentProcess(), TRUE);
	::SetProcessAffinityMask(::GetCurrentProcess(), 0x00000005);

	printf("pid:%u\n", ::GetCurrentProcessId());
	getchar();

	for (int i = 0; i < 10; i++)
	{
		testfunc();
	}
	
//	getchar();
	return 0;
}
