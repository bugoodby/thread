#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>

bool Show_TimerResolution( FILE *fp )
{
	fprintf(fp, "==== Timer Resolution ===\n");

	DWORD dwTimeAdjustment;
	DWORD dwTimeIncrement;
	BOOL bTimeAdjustmentDisabled;
	if (::GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &bTimeAdjustmentDisabled)) {
		fprintf(fp, "システムタイマー分解能（クロック割り込み間隔）：%f ms\n", (double)dwTimeIncrement / 10000.0);
	}

#if 0
	TIMECAPS timeCaps;
	if (::timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == TIMERR_NOERROR) {
		fprintf(fp, "マルチメディアタイマー分解能：%d ms\n", timeCaps.wPeriodMin);
	}
#endif

	LARGE_INTEGER liFrequency;
	if (::QueryPerformanceFrequency(&liFrequency)) {
		fprintf(fp, "高分解能カウンター分解能：%f ms\n", (double)1000.0 / liFrequency.QuadPart);
	}
	
	fprintf(fp, "\n");
	return true;
}

bool Test_VirtualAlloc()
{
	size_t memsize = 2 * 1024 * 1024;

	SIZE_T min = 0, max = 0;
	::GetProcessWorkingSetSize(::GetCurrentProcess(), &min, &max);
	printf("min: %u KB, max: %u KB\n", min/1024, max/1024);

	{
		BYTE *pBuf = (BYTE*)VirtualAlloc(NULL, memsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		printf("VirtualAlloc: %p\n", pBuf);
		BOOL ret = VirtualLock(pBuf, memsize);
		printf("VirtualLock: %d\n", ret);
		memset(pBuf, 0, memsize);

//		getchar();

		void *pa[10];
		for (int i = 0; i < 10; i++) {
			pa[i] = malloc(10 * 1024 * 1024);
			memset(pa[i], 0, 10 * 1024 * 1024);
		}

		getchar();

		for (int i = 0; i < 10; i++) {
			free(pa[i]);
		}

		if (ret) VirtualUnlock(pBuf, memsize);
		VirtualFree(pBuf, memsize, 0);
	}


	min += 2 * 1024 * 1024;
	max += 2 * 1024 * 1024;
	::SetProcessWorkingSetSize(::GetCurrentProcess(), min, max);
	printf("min: %u KB, max: %u KB\n", min / 1024, max / 1024);

	{
		BYTE *pBuf = (BYTE*)VirtualAlloc(NULL, memsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		printf("VirtualAlloc: %p\n", pBuf);
		BOOL ret = VirtualLock(pBuf, memsize);
		printf("VirtualLock: %d\n", ret);
		memset(pBuf, 0, memsize);

//		getchar();

		if (ret) VirtualUnlock(pBuf, memsize);
		VirtualFree(pBuf, memsize, 0);
	}

	return 0;
}

int main()
{
	Show_TimerResolution(stdout);
	Test_VirtualAlloc();

//	getchar();
	return 0;
}