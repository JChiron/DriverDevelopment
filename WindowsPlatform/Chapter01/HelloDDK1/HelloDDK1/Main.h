#pragma once
extern "C"
#include <ntddk.h>


#define NUM_OF_ARRAY_ELEMENT(p) (sizeof(p) / sizeof((p)[0]))

typedef struct _DEVICE_EXTENTION
{
	PDEVICE_OBJECT pDev;
	PUNICODE_STRING pusDevName;
	PUNICODE_STRING pusSymName;
}DEVICE_EXTENTION, *PDEVICE_EXTENTION;


VOID DriverUnload(PDRIVER_OBJECT pDrv);
NTSTATUS CreateDevice(PDRIVER_OBJECT pDrv, LPCWSTR wcsDevPostfixName);
NTSTATUS DefaultDispatchRoutine(PDEVICE_OBJECT pDev, PIRP pIrp);