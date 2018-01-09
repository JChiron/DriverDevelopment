#pragma once

#ifdef __cplusplus
extern "C"
#endif // __cplusplus
#include <ntddk.h>




typedef struct _DEVICE_EXTENTION
{
	PDEVICE_OBJECT pDev;
	PUNICODE_STRING pusDevName;
	PUNICODE_STRING pusSymName;
}DEVICE_EXTENTION, *PDEVICE_EXTENTION;


VOID DriverUnload(PDRIVER_OBJECT pDrv);
NTSTATUS CreateDevice(PDRIVER_OBJECT pDrv, LPCWSTR wcsDevPostfixName);
NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDev, PIRP pIrp);