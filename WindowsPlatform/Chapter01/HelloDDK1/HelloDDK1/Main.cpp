#include "Main.h"


#pragma INITCODE
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pusRegistryPath)
{
	// go ********************************************************************************************
	KdPrint(("Enter DriverEntry() !\n"));

	DbgBreakPoint();

	NTSTATUS nStatus = STATUS_SUCCESS;

	// 1. set driver object field ********************************************************************************************
	pDrv->DriverUnload = DriverUnload;
	pDrv->MajorFunction[IRP_MJ_CREATE] =
		pDrv->MajorFunction[IRP_MJ_CLOSE] =
		pDrv->MajorFunction[IRP_MJ_READ] =
		pDrv->MajorFunction[IRP_MJ_WRITE] = DefaultDispatchRoutine;

	// 2. create device object ********************************************************************************************
	nStatus = CreateDevice(pDrv, L"HelloDDK1");
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Error occur at CreateDevice() !\n"));
		return nStatus;
	}

	// end ********************************************************************************************
	KdPrint(("Leave DriverEntry() !\n"));
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
VOID DriverUnload(PDRIVER_OBJECT pDrv)
{
	// go ********************************************************************************************
	KdPrint(("Enter DriverUnload() !\n"));
	PAGED_CODE();

	// 1. delete sym & device ********************************************************************************************
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

	// end ********************************************************************************************
	KdPrint(("Leave DriverUnload() !\n"));
}

#pragma PAGEDCODE
NTSTATUS CreateDevice(PDRIVER_OBJECT pDrv, LPCWSTR wcsDevPostfixName)
{
	// 0.create device object
	// 1.create symbolic link
	// 2.configure device object field

	// go
	KdPrint(("Enter CreateDevice() !\n"));
	PAGED_CODE();

	NTSTATUS nStatus = STATUS_SUCCESS;
	WCHAR wcsTmp[0xff]{};

	// 0.create device object
	RtlFillMemory(wcsTmp, sizeof(wcsTmp), 0);
	RtlMoveMemory(wcsTmp, L"\\Device\\usDevName", wcslen(L"\\Device\\usDevName"));
	RtlMoveMemory(wcsTmp + wcslen(wcsTmp) * 2, wcsDevPostfixName, wcslen(wcsDevPostfixName) * 2 + 2);

	PUNICODE_STRING pusDevName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, wcslen(wcsTmp) * 2 + 2);
	pusDevName->Length = wcslen(wcsTmp) * 2;
	pusDevName->MaximumLength = pusDevName->Length + 2;
	pusDevName->Buffer = (PWCH)(pusDevName + 1);
	RtlMoveMemory(pusDevName->Buffer, wcsTmp, wcslen(wcsTmp) + 2);

	PDEVICE_OBJECT pDev{};
	nStatus = IoCreateDevice(pDrv, sizeof(DEVICE_EXTENTION), pusDevName, FILE_DEVICE_UNKNOWN, 0, 0, &pDev);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Error occur at IoCreateDevice() !\n"));
		return nStatus;
	}

	// 1.create symbolic link
	RtlFillMemory(wcsTmp, sizeof(wcsTmp), 0);
	RtlMoveMemory(wcsTmp, L"\\DosDevices\\usSymName", wcslen(L"\\DosDevices\\usSymName"));
	RtlMoveMemory(wcsTmp + wcslen(wcsTmp) * 2, wcsDevPostfixName, wcslen(wcsDevPostfixName) * 2 + 2);

	PUNICODE_STRING pusSymName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, wcslen(wcsTmp) * 2 + 2);
	pusSymName->Length = wcslen(wcsTmp) * 2;
	pusSymName->MaximumLength = pusSymName->Length + 2;
	pusSymName->Buffer = (PWCH)(pusSymName + 1);
	RtlMoveMemory(pusSymName->Buffer, wcsTmp, wcslen(wcsTmp) + 2);

	nStatus = IoCreateSymbolicLink(pusSymName, pusDevName);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Error occur at IoCreateSymbolicLink() !\n"));
		return nStatus;
	}

	// 2.configure device object field
	pDev->Flags |= DO_BUFFERED_IO;
	PDEVICE_EXTENTION pDevEx = (PDEVICE_EXTENTION)pDev->DeviceExtension;
	pDevEx->pDev = pDev;
	pDevEx->pusDevName = pusDevName;
	pDevEx->pusSymName = pusSymName;
	pDev->Flags |= DO_DIRECT_IO;

	// end
	KdPrint(("Leave CreateDevice() !\n"));
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS DefaultDispatchRoutine(PDEVICE_OBJECT pDev, PIRP pIrp)
{
	// go
	KdPrint(("Enter DefaultDispatchRoutine() !\n"));
	PAGED_CODE();

	// 1. respond io
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IofCompleteRequest(pIrp, IO_NO_INCREMENT);

	// end
	KdPrint(("Leave DefaultDispatchRoutine() !\n"));
	return STATUS_SUCCESS;
}