#include <windows.h>
#include <stdio.h>
using CPUWORD = UINT_PTR;

int main()
{
	// 작업관리자에서 확인해서 Process Id 를 입력하면 됨
	printf("input PID : ");
	DWORD PID;
	scanf_s(" %u", &PID);
	HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	printf("Insert the value you want to find in memory : ");
	DWORD valueToFind;
	scanf_s(" %u", &valueToFind);

	DWORD valueToModify;
	printf("Insert the value you want to push in memory : ");
	scanf_s(" %u", &valueToModify);

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	// 시작위치, 유저 가상메모리 가장 낮은주소부터 시작
	UINT_PTR pTargetMemoryPos = (UINT_PTR)si.lpMinimumApplicationAddress;

	// 가상메모리 페이지 정보 얻어오기
	MEMORY_BASIC_INFORMATION mbi;

	// 탐색 시작
	while (pTargetMemoryPos < (UINT_PTR)si.lpMaximumApplicationAddress)
	{
		SIZE_T size = VirtualQueryEx(hTargetProcess, (LPCVOID)pTargetMemoryPos, &mbi, sizeof(mbi));

		// 다른 프로세스와 공유하는 메모리는 찾지 않는다.
		if (!(size == sizeof(mbi) && mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT))
		{
			pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
			continue;
		}

		//적절한 메모리 구역을 찾앗기 때문에 해당 메모리를 버퍼에 읽어옴.
		CPUWORD* pTempBuffer = new CPUWORD[mbi.RegionSize / sizeof(CPUWORD)];
		BOOL bResult = ReadProcessMemory(hTargetProcess, (LPCVOID)pTargetMemoryPos, (LPVOID)pTempBuffer, mbi.RegionSize, NULL);
		if (!bResult)
		{
			printf("\nRead Process Memory Failed\n addr : %x, protect attributes : %x, Err Code : %u \n", pTargetMemoryPos, mbi.Protect, GetLastError());
			pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
			delete[] pTempBuffer;
			continue;
		}
		printf("Read Process Memory Success\n addr : %x\n", pTargetMemoryPos);
		for (DWORD i = 0; i < mbi.RegionSize / sizeof(CPUWORD); ++i)
		{
			// 조작하고자 하는 메모리의 값과 일치하는 값을 발견함
			if (pTempBuffer[i] == valueToFind) 
			{
				printf("\nsuccess to find value!\nRegion size : %uKbytes", mbi.RegionSize / 1024);
				UINT_PTR memAddrToModify = (UINT_PTR)mbi.BaseAddress + i * sizeof(CPUWORD);
				printf("\nTarget Addr : %x", memAddrToModify);
				SIZE_T NumberOfBytesWritten;
				bResult = WriteProcessMemory(hTargetProcess, (LPVOID)memAddrToModify, &valueToModify, sizeof(valueToModify), &NumberOfBytesWritten);
				if (!bResult) __debugbreak();
				else if (NumberOfBytesWritten != sizeof(CPUWORD)) __debugbreak();

				printf("\nIf successful, press e to end the process : ");
				char TerminateChar;
				scanf_s(" %c", &TerminateChar, sizeof(char));
				if (TerminateChar == 'e')
				{
					delete[] pTempBuffer;
					return 0;
				}
			}
		}
		pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
	}
	return 0;
}