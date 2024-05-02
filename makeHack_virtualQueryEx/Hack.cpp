#include <windows.h>
#include <stdio.h>
using CPUWORD = UINT_PTR;

int main()
{
	// �۾������ڿ��� Ȯ���ؼ� Process Id �� �Է��ϸ� ��
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
	// ������ġ, ���� ����޸� ���� �����ּҺ��� ����
	UINT_PTR pTargetMemoryPos = (UINT_PTR)si.lpMinimumApplicationAddress;

	// ����޸� ������ ���� ������
	MEMORY_BASIC_INFORMATION mbi;

	// Ž�� ����
	while (pTargetMemoryPos < (UINT_PTR)si.lpMaximumApplicationAddress)
	{
		SIZE_T size = VirtualQueryEx(hTargetProcess, (LPCVOID)pTargetMemoryPos, &mbi, sizeof(mbi));

		// �ٸ� ���μ����� �����ϴ� �޸𸮴� ã�� �ʴ´�.
		if (!(size == sizeof(mbi) && mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT))
		{
			pTargetMemoryPos += (UINT_PTR)mbi.RegionSize;
			continue;
		}

		//������ �޸� ������ ã�ѱ� ������ �ش� �޸𸮸� ���ۿ� �о��.
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
			// �����ϰ��� �ϴ� �޸��� ���� ��ġ�ϴ� ���� �߰���
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