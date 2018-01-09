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
	pDrv->MajorFunction[IRP_MJ_READ] = IrpRead;

	// 1. 
	BOOLEAN bRet = AttachAllKbdDevice(pDrv);
	if (!bRet)
	{
		KdPrint(("Err at AttachAllKbdDevice() !\n"));
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

		IoDetachDevice(pDevEx->pNextLayerDev);
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
	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(((PDEVICE_EXTENTION)(pDev->DeviceExtension))->pNextLayerDev, pIrp);
}

#pragma code_seg("INIT")
BOOLEAN AttachAllKbdDevice(PDRIVER_OBJECT pDrv)
{
	// 0. get keyboard driver object
	// 1. attach all devices(vertical device stack top) of keyboard driver object
	// 1.1 create device object
	// 1.2 attach device object
	// 1.3 set DEVICE_OBJECT fields


	// go
	NTSTATUS nStatus = STATUS_SUCCESS;

	// 0. get keyboard driver object
	LPCWSTR wcsKbdDriverName = L"\\Driver\\kbdclass";
	UNICODE_STRING usKbdDriverName{};
	RtlInitUnicodeString(&usKbdDriverName, wcsKbdDriverName);
	PDRIVER_OBJECT pKbdDrv{};
	nStatus = ObReferenceObjectByName(&usKbdDriverName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (PVOID*)&pKbdDrv);
	if (!NT_SUCCESS(nStatus))
		return false;

	// 1. attach all devices(vertical device stack top) of keyboard driver object
	PDEVICE_OBJECT pKbdDev = pKbdDrv->DeviceObject;
	if (!pKbdDev)
		return false;

	for (int i = 0; pKbdDev; i++, pKbdDev = pKbdDev->NextDevice)
	{
		// 1.1 create device object
		UNICODE_STRING usIndex{};
		WCHAR wcsIndex[] = L"xoxoxoxo";
		RtlInitUnicodeString(&usIndex, wcsIndex);
		RtlIntegerToUnicodeString(i, 0x10, &usIndex);
		//usIndex.Buffer[usIndex.Length / 2] = 0;
		UNICODE_STRING usDevPostfixName{};
		WCHAR wcsDevPostfixName[] = L"KbdFilterDriverxoxoxoxo";
		RtlInitUnicodeString(&usDevPostfixName, wcsDevPostfixName);
		usDevPostfixName.Length -= 16;
		//usDevPostfixName.Buffer[usDevPostfixName.Length / 2] = 0;
		RtlAppendUnicodeStringToString(&usDevPostfixName, &usIndex);

		nStatus = CreateDevice(pDrv, usDevPostfixName.Buffer);
		if (!NT_SUCCESS(nStatus))
			ASSERT(0);

		// 1.2 attach device object
		PDEVICE_OBJECT pOrgDevStackTopDev{};
		nStatus = IoAttachDeviceToDeviceStackSafe(pDrv->DeviceObject, pKbdDev, &pOrgDevStackTopDev);
		if (!NT_SUCCESS(nStatus))
			ASSERT(0);

		// 1.3 set DEVICE_OBJECT fields
		PDEVICE_OBJECT pDev = pDrv->DeviceObject;
		pDev->DeviceType = pOrgDevStackTopDev->DeviceType;
		pDev->Characteristics = pOrgDevStackTopDev->Characteristics;
		pDev->StackSize = pOrgDevStackTopDev->StackSize + 1;
		pDev->Flags |= pOrgDevStackTopDev->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);

		PDEVICE_EXTENTION pDevExt = (PDEVICE_EXTENTION)pDev->DeviceExtension;
		pDevExt->pNextLayerDev = pOrgDevStackTopDev;
	}

	// end
	return true;
}

#pragma code_seg("PAGE") 
NTSTATUS IrpRead(PDEVICE_OBJECT pDev, PIRP pIrp)
{
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine(pIrp, IrpReadCompletionRoutine, pDev, true, true, true);

	return IoCallDriver(((DEVICE_EXTENTION*)pDev->DeviceExtension)->pNextLayerDev, pIrp);
}

#pragma code_seg("") 
NTSTATUS IrpReadCompletionRoutine(PDEVICE_OBJECT pDev, PIRP pIrp, PVOID pContext)
{
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	if (pIrp->PendingReturned)
		IoMarkIrpPending(pIrp);

	if (NT_SUCCESS(pIrp->IoStatus.Status))
	{
		PKEYBOARD_INPUT_DATA pMdlAddr = (PKEYBOARD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;
		ULONG ulCount = pIrp->IoStatus.Information;
		pStack = 0;
	}

	return pIrp->IoStatus.Status;
}
