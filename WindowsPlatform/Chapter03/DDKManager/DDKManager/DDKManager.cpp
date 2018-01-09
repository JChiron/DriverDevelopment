// DDKManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include "DDKManager.h"


#define DRIVERNAME (L"DDKSERVER")
#define FULLPATHNAME (L"C:\\Users\\jchiron\\Documents\\Visual Studio 2015\\Projects\\DDKManager\\Debug\\HelloDDK1.sys")
SC_HANDLE hSCM{};
SC_HANDLE hService{};

int main()
{
	LoadDDK(DRIVERNAME, FULLPATHNAME);
	UnloadDDK();

    return 0;
}

bool LoadDDK(LPCWSTR wcsNakedFileName, LPCWSTR wcsFullFileName)
{
	// go
	// 0.�򿪷�����ƹ�����SCM
	// 1.������������
	// 2.������������
	// end

	// go

	// 0.�򿪷�����ƹ�����SCM
	hSCM = OpenSCManagerW(0, 0, SC_MANAGER_ALL_ACCESS);
	assert(hSCM);

	// 1.������������
	hService = CreateServiceW(hSCM, DRIVERNAME, DRIVERNAME, SERVER_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, wcsFullFileName, 0, 0, 0, 0, 0);
	if (!hService)
	{
		DWORD dwLastError = GetLastError();
		if (ERROR_SERVICE_EXISTS != dwLastError)
		{
			printf("Error had Occurred at CreateServiceW()!\n");
			system("puase");

			CloseServiceHandle(hSCM);
			return false;
		}
	}

	// 2.������������
	bool bRet = StartServiceW(hService, 0, 0);
	if (!bRet)
	{
		DWORD dwLastError = GetLastError();
		if (ERROR_SERVICE_ALREADY_RUNNING != dwLastError)
		{
			printf("Error had Occurred at StartServiceW()!\n");
			system("puase");

			::CloseServiceHandle(hSCM);
			::CloseServiceHandle(hService);
			return false;
		}
	}

	// end
	return true;
}

bool UnloadDDK()
{
	SERVICE_STATUS ss{};
	bool bRet = ControlService(hService, SERVICE_CONTROL_STOP, &ss);
	if (!bRet)
	{
		if (ERROR_SERVICE_NOT_ACTIVE != GetLastError())
		{
			printf("Error had Occurred at ControlService()!\n");
			system("puase");
			return false;
		}
	}

	bRet = DeleteService(hService);
	{
		printf("Error had Occurred at DeleteService()!\n");
		system("puase");
		return false;
	}
}