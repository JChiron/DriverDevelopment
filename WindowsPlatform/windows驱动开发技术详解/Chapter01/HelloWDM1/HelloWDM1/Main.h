#pragma once
extern "C"
#include <ntddk.h>

#define NUM_OF_ARRAY_ELEMENT(p) (sizeof(p) / sizeof((p)[0]))

typedef struct _DEVICE_EXTENTION
{
	PDEVICE_OBJECT pOwnerDev;
	UNICODE_STRING usOwnerDevName;
	UNICODE_STRING usOwnerSymName;
	PDEVICE_OBJECT pLowerLayerDev;
}DEVICE_EXTENTION, *PDEVICE_EXTENTION;


VOID DriverUnload(PDRIVER_OBJECT pDri);
NTSTATUS AddDevice(PDRIVER_OBJECT pDri, PDEVICE_OBJECT pPDO);

NTSTATUS Pnp(PDEVICE_OBJECT pFDO, PIRP pIrp);
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENTION pDevEx, PIRP pIrp);
NTSTATUS RemoveDevice(PDEVICE_EXTENTION pDevEx, PIRP pIrp);
NTSTATUS DispatchRoutine(PDEVICE_OBJECT pFDO, PIRP pIrp);