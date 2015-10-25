#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>

//#define PRINTF printf
#define PRINTF 


unsigned int __stdcall WorkerThread_H(void *param)
{
	printf("ThreadH start - tid=%u\n", ::GetCurrentThreadId());
	
	double val = 0.0;
	
#pragma omp parallel num_threads(2)
	{
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		PRINTF("ThreadH CPU=%u Priority=%d tid=%u\n",
			::GetCurrentProcessorNumber(),
			::GetThreadPriority(::GetCurrentThread()),
			::GetCurrentThreadId());
		
#pragma omp parallel for
		for (int i = 0; i < 20000000; i++)
		{
			double x = 1;
			val += x;
		}
	}


	for (int i = 0; i < 10000000; i++)
	{
		double x = 1;
		val += x;
	}

#pragma omp parallel num_threads(2)
	{
#pragma omp parallel for
		for (int i = 0; i < 20000000; i++)
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
	
	// 低優先度スレッド作成
	hThread_L[0] = (HANDLE)_beginthreadex(
		NULL,				// security
		0,					// stack size
		WorkerThread_L,		// start_address
		NULL,				// arglist
		CREATE_SUSPENDED,	// initflag
		&nTID_L[0]			// thread addr
		);
	hThread_L[1] = (HANDLE)_beginthreadex(
		NULL,				// security
		0,					// stack size
		WorkerThread_L,		// start_address
		NULL,				// arglist
		CREATE_SUSPENDED,	// initflag
		&nTID_L[1]			// thread addr
		);
	
	SetThreadPriority(hThread_L[0], THREAD_PRIORITY_LOWEST);
	SetThreadPriority(hThread_L[1], THREAD_PRIORITY_LOWEST);
	ResumeThread(hThread_L[0]);
	ResumeThread(hThread_L[1]);
	
	for ( int i = 0; i < 5; i++ )
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
	CloseHandle(hThread_L[0]);
	CloseHandle(hThread_L[1]);
	
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

	//getchar();
	return 0;
}
