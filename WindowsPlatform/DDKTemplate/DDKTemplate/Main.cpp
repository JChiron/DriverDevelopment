#include "Main.h"


#pragma code_seg("INIT") 
//************************************
// Name: DriverEntry 2018/01/07
// Args: PDRIVER_OBJECT pDrv
// Args: PUNICODE_STRING pusRegistryPath
// Rets: extern " NTSTATUS
//************************************
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pusRegistryPath)
{
	// go
	// 0. set driver object fields
	// 1. create driver object
	// end


	// go
	DbgBreakPoint();

	KdPrint(("Enter DriverEntry() !\n"));

	NTSTATUS nStatus = STATUS_SUCCESS;

	// 0. set driver object fields
	pDrv->DriverUnload = DriverUnload;
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		pDrv->MajorFunction[i] = DispatchRoutine;

	// 1. create driver object
	nStatus = CreateDevice(pDrv, L"DDKTemplate");
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Err at CreateDevice() !\n"));
		return nStatus;
	}

	// end
	KdPrint(("Leave DriverEntry() !\n"));
	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE") 
VOID DriverUnload(PDRIVER_OBJECT pDrv)
{
	// go
	KdPrint(("Enter DriverUnload() !\n"));
	PAGED_CODE();

	// 1. delete sym & device
	PDEVICE_OBJECT pHDevStackTop = pDrv->DeviceObject;

	while (pHDevStackTop)
	{
		PDEVICE_EXTENTION pDevEx = (PDEVICE_EXTENTION)pHDevStackTop->DeviceExtension;
		pHDevStackTop = pHDevStackTop->NextDevice;

		IoDeleteSymbolicLink(pDevEx->pusSymName);
		ExFreePool(pDevEx->pusDevName);
		ExFreePool(pDevEx->pusSymName);
		IoDeleteDevice(pDevEx->pDev);
	}

	// end
	KdPrint(("Leave DriverUnload() !\n"));
}

#pragma code_seg("INIT") 
NTSTATUS CreateDevice(PDRIVER_OBJECT pDrv, LPCWSTR wcsDevPostfixName)
{
	// go
	// 0. create device object
	// 0.0 get full device name that use WCHAR type
	// 0.1 allocate and init full device name that use PUNICODE_STRING type
	// 0.2 create device object
	// 1. create symbolic link
	// 1.0 get full symbolic link name that use WCHAR type
	// 1.1 allocate and init full symbolic link name that use PUNICODE_STRING type
	// 1.2 create symbolic link
	// 2. configure device object field
	// end


	// go
	KdPrint(("Enter CreateDevice() !\n"));

	if (!pDrv || !wcsDevPostfixName)
		ASSERT(0);

	if (0xff < wcslen(wcsDevPostfixName) + wcslen(L"\\DosDevices\\usSymName") + 1)
		ASSERT(0);	// buffer overflow
		

	NTSTATUS nStatus = STATUS_SUCCESS;
	WCHAR wcsTmp[0xff]{};

	// 0.create device object
	// 0.0 get full device name that use WCHAR type
	RtlFillMemory(wcsTmp, sizeof(wcsTmp), 0);
	RtlMoveMemory(wcsTmp, L"\\Device\\usDevName", wcslen(L"\\Device\\usDevName") * 2);
	RtlMoveMemory(wcsTmp + wcslen(wcsTmp), wcsDevPostfixName, wcslen(wcsDevPostfixName) * 2 + 2);

	// 0.1 allocate and init full device name that use PUNICODE_STRING type
	PUNICODE_STRING pusDevName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, wcslen(wcsTmp) * 2 + 2 + sizeof(UNICODE_STRING));
	RtlFillMemory(pusDevName, wcslen(wcsTmp) * 2 + 2, 0);
	pusDevName->Length = wcslen(wcsTmp) * 2;
	pusDevName->MaximumLength = pusDevName->Length + 2;
	pusDevName->Buffer = (PWCH)(pusDevName + 1);
	RtlMoveMemory(pusDevName->Buffer, wcsTmp, wcslen(wcsTmp) * 2 + 2);

	// 0.2 create device object
	PDEVICE_OBJECT pDev{};
	nStatus = IoCreateDevice(pDrv, sizeof(DEVICE_EXTENTION), pusDevName, FILE_DEVICE_UNKNOWN, 0, 0, &pDev);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Err at IoCreateDevice() !\n"));
		ExFreePool(pusDevName);
		return nStatus;
	}

	// 1.create symbolic link
	// 1.0 get full symbolic link name that use WCHAR type
	RtlFillMemory(wcsTmp, sizeof(wcsTmp), 0);
	RtlMoveMemory(wcsTmp, L"\\DosDevices\\usSymName", wcslen(L"\\DosDevices\\usSymName") * 2);
	RtlMoveMemory(wcsTmp + wcslen(wcsTmp), wcsDevPostfixName, wcslen(wcsDevPostfixName) * 2 + 2);

	// 1.1 allocate and init full symbolic link name that use PUNICODE_STRING type
	PUNICODE_STRING pusSymName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, wcslen(wcsTmp) * 2 + 2 + sizeof(UNICODE_STRING));
	RtlFillMemory(pusSymName, wcslen(wcsTmp) * 2 + 2, 0);
	pusSymName->Length = wcslen(wcsTmp) * 2;
	pusSymName->MaximumLength = pusSymName->Length + 2;
	pusSymName->Buffer = (PWCH)(pusSymName + 1);
	RtlMoveMemory(pusSymName->Buffer, wcsTmp, wcslen(wcsTmp) * 2 + 2);

	// 1.2 create symbolic link
	nStatus = IoCreateSymbolicLink(pusSymName, pusDevName);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Err at IoCreateSymbolicLink() !\n"));
		ExFreePool(pusDevName);
		ExFreePool(pusSymName);
		return nStatus;
	}

	// 2.configure device object field
	pDev->Flags |= DO_DIRECT_IO;

	PDEVICE_EXTENTION pDevEx = (PDEVICE_EXTENTION)pDev->DeviceExtension;
	pDevEx->pDev = pDev;
	pDevEx->pusDevName = pusDevName;
	pDevEx->pusSymName = pusSymName;

	// end
	KdPrint(("Leave CreateDevice() !\n"));
	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE") 
NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDev, PIRP pIrp)
{
	// go
	KdPrint(("Enter DispatchRoutine() !\n"));
	PAGED_CODE();

	// 1. respond io
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IofCompleteRequest(pIrp, IO_NO_INCREMENT);

	// end
	KdPrint(("Leave DispatchRoutine() !\n"));
	return STATUS_SUCCESS;
}