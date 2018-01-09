#pragma once

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <ntddk.h>
#include <ntddkbd.h>
extern POBJECT_TYPE* IoDriverObjectType;

#ifdef __cplusplus
}
#endif // __cplusplus


typedef struct _DEVICE_EXTENTION
{
	PDEVICE_OBJECT pDev{};
	PUNICODE_STRING pusDevName{};
	PUNICODE_STRING pusSymName{};
	PDEVICE_OBJECT pNextLayerDev{};
}DEVICE_EXTENTION, *PDEVICE_EXTENTION;


VOID DriverUnload(PDRIVER_OBJECT pDrv);
NTSTATUS CreateDevice(PDRIVER_OBJECT pDrv, LPCWSTR wcsDevPostfixName);
NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDev, PIRP pIrp);

BOOLEAN AttachAllKbdDevice(PDRIVER_OBJECT pDrv);
extern "C" NTSTATUS ObReferenceObjectByName(PUNICODE_STRING ObjectName, ULONG Attributes, PACCESS_STATE AccessState, ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode, PVOID ParseContext, PVOID *Object);
NTSTATUS IrpRead(PDEVICE_OBJECT pDev, PIRP pIrp);
NTSTATUS IrpReadCompletionRoutine(PDEVICE_OBJECT pDev, PIRP pIrp, PVOID pContext);

