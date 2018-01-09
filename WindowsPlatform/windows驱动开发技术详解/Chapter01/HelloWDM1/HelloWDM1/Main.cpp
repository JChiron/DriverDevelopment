#include "Main.h"

#pragma INITCODE
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDri, PUNICODE_STRING pusRegistryPath)
{
	// go ********************************************************************************************
	KdPrint(("Enter DriverEntry() !\n"));

	DbgBreakPoint();

	// 1. set driver object field ********************************************************************************************
	pDri->DriverExtension->AddDevice = AddDevice;
	pDri->DriverUnload = DriverUnload;
	pDri->MajorFunction[IRP_MJ_PNP] = Pnp;
	pDri->MajorFunction[IRP_MJ_CREATE] =
		pDri->MajorFunction[IRP_MJ_CLOSE] =
		pDri->MajorFunction[IRP_MJ_READ] =
		pDri->MajorFunction[IRP_MJ_WRITE] = 
		pDri->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchRoutine;

	// end ********************************************************************************************
	KdPrint(("Leave DriverEntry() !\n"));

	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
VOID DriverUnload(PDRIVER_OBJECT pDri)
{
	// go ********************************************************************************************
	KdPrint(("Enter DriverUnload() !\n"));
	PAGED_CODE();

	// end ********************************************************************************************
	KdPrint(("Leave DriverUnload() !\n"));
}

#pragma PAGEDCODE
NTSTATUS AddDevice(PDRIVER_OBJECT pDri, PDEVICE_OBJECT pPDO)
{
	// go ********************************************************************************************
	KdPrint(("Enter AddDevice() !\n"));
	PAGED_CODE();

	NTSTATUS nStatus = STATUS_SUCCESS;
	UINT32 uSize = 0;
	WCHAR* wczTemp = 0;

	// 1. create FDO ********************************************************************************************
	PDEVICE_OBJECT pFDO = 0;
	UNICODE_STRING usDevName;
	uSize = sizeof(L"\\Device\\usDevNameHelloWDM1");
	wczTemp = (WCHAR*)ExAllocatePool(PagedPool, uSize);
	RtlCopyMemory(wczTemp, L"\\Device\\usDevNameHelloWDM1", uSize);
	RtlInitUnicodeString(&usDevName, wczTemp);
	nStatus = IoCreateDevice(pDri, sizeof(DEVICE_EXTENTION), &usDevName, FILE_DEVICE_UNKNOWN, 0, 0, &pFDO);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Error occur at IoCreateDevice() !\n"));
		return nStatus;
	}

	// 2. create symbol link ********************************************************************************************
	UNICODE_STRING usSymName;
	uSize = sizeof(L"\\Global??\\usSymNameHelloWDM1");
	wczTemp = (WCHAR*)ExAllocatePool(PagedPool, uSize);
	RtlCopyMemory(wczTemp, L"\\Global??\\usSymNameHelloWDM1", uSize);
	RtlInitUnicodeString(&usSymName, wczTemp);
	nStatus = IoCreateSymbolicLink(&usSymName, &usDevName);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("Error occur at IoCreateSymbolicLink() !\n"));
		return nStatus;
	}

	// 3. set field of FDO ********************************************************************************************
	pFDO->Flags |= DO_BUFFERED_IO;
	PDEVICE_EXTENTION pDevEx = (PDEVICE_EXTENTION)pFDO->DeviceExtension;
	pDevEx->pOwnerDev = pFDO;
	pDevEx->usOwnerDevName = usDevName;
	pDevEx->usOwnerSymName = usSymName;
	pDevEx->pLowerLayerDev = IoAttachDeviceToDeviceStack(pFDO, pPDO);

	// end ********************************************************************************************
	KdPrint(("Leave AddDevice() !\n"));

	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS Pnp(PDEVICE_OBJECT pFDO, PIRP pIrp)
{
	// go ********************************************************************************************
	KdPrint(("Enter Pnp() !\n"));
	PAGED_CODE();

	// 1. create FDO ********************************************************************************************
	PDEVICE_EXTENTION pDevEx = (PDEVICE_EXTENTION)pFDO->DeviceExtension;
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	static NTSTATUS(*FunTable[])(PDEVICE_EXTENTION pDevEx, PIRP pIrp) =
	{
		DefaultPnpHandler,		// IRP_MN_START_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_REMOVE_DEVICE
		RemoveDevice,			// IRP_MN_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_RELATIONS
		DefaultPnpHandler,		// IRP_MN_QUERY_INTERFACE
		DefaultPnpHandler,		// IRP_MN_QUERY_CAPABILITIES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_TEXT
		DefaultPnpHandler,		// IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// 
		DefaultPnpHandler,		// IRP_MN_READ_CONFIG
		DefaultPnpHandler,		// IRP_MN_WRITE_CONFIG
		DefaultPnpHandler,		// IRP_MN_EJECT
		DefaultPnpHandler,		// IRP_MN_SET_LOCK
		DefaultPnpHandler,		// IRP_MN_QUERY_ID
		DefaultPnpHandler,		// IRP_MN_QUERY_PNP_DEVICE_STATE
		DefaultPnpHandler,		// IRP_MN_QUERY_BUS_INFORMATION
		DefaultPnpHandler,		// IRP_MN_DEVICE_USAGE_NOTIFICATION
		DefaultPnpHandler		// IRP_MN_SURPRISE_REMOVAL
	};

	// 2. unknown minor function code ********************************************************************************************
	UINT32 uFunNum = pStack->MinorFunction;
	if (uFunNum >= NUM_OF_ARRAY_ELEMENT(FunTable))
	{
		KdPrint(("Leave Pnp() !\n"));
		return DefaultPnpHandler(pDevEx, pIrp);
	}

	// 3. known minor function code ********************************************************************************************
#if DBG
	static char* FunName[] =
	{
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE",
		"IRP_MN_CANCEL_REMOVE_DEVICE",
		"IRP_MN_STOP_DEVICE",
		"IRP_MN_QUERY_STOP_DEVICE",
		"IRP_MN_CANCEL_STOP_DEVICE",
		"IRP_MN_QUERY_DEVICE_RELATIONS",
		"IRP_MN_QUERY_INTERFACE",
		"IRP_MN_QUERY_CAPABILITIES",
		"IRP_MN_QUERY_RESOURCES",
		"IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
		"IRP_MN_QUERY_DEVICE_TEXT",
		"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
		"",
		"IRP_MN_READ_CONFIG",
		"IRP_MN_WRITE_CONFIG",
		"IRP_MN_EJECT",
		"IRP_MN_SET_LOCK",
		"IRP_MN_QUERY_ID",
		"IRP_MN_QUERY_PNP_DEVICE_STATE",
		"IRP_MN_QUERY_BUS_INFORMATION",
		"IRP_MN_DEVICE_USAGE_NOTIFICATION",
		"IRP_MN_SURPRISE_REMOVAL"
	};

	KdPrint(("PNP Request (%s)\n", FunName[uFunNum]));
#endif // DBG

	// end ********************************************************************************************
	KdPrint(("Leave Pnp() !\n"));
	return (*FunTable[uFunNum])(pDevEx, pIrp);
}

#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENTION pDevEx, PIRP pIrp)
{
	// go ********************************************************************************************
	KdPrint(("Enter DefaultPnpHandler() !\n"));
	PAGED_CODE();

	// end ********************************************************************************************
	KdPrint(("Leave DefaultPnpHandler() !\n"));

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevEx->pLowerLayerDev, pIrp);
}

#pragma PAGEDCODE
NTSTATUS RemoveDevice(PDEVICE_EXTENTION pDevEx, PIRP pIrp)
{
	// go ********************************************************************************************
	KdPrint(("Enter DefaultPnpHandler() !\n"));
	PAGED_CODE();
	NTSTATUS nStatus = STATUS_SUCCESS;

	// 1.  ********************************************************************************************
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	nStatus = DefaultPnpHandler(pDevEx, pIrp);
	IoDeleteSymbolicLink(&pDevEx->usOwnerSymName);
	ExFreePool(pDevEx->usOwnerSymName.Buffer);

	// 2.  ********************************************************************************************
	if (pDevEx->pLowerLayerDev)
		IoDetachDevice(pDevEx->pLowerLayerDev);

	// 3.  ********************************************************************************************
	ExFreePool(pDevEx->usOwnerDevName.Buffer);
	IoDeleteDevice(pDevEx->pLowerLayerDev);

	// end ********************************************************************************************
	KdPrint(("Leave DefaultPnpHandler() !\n"));
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS DispatchRoutine(PDEVICE_OBJECT pFDO, PIRP pIrp)
{
	// go ********************************************************************************************
	KdPrint(("Enter DispatchRoutine() !\n"));
	PAGED_CODE();

	// 1.  ********************************************************************************************
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	// end ********************************************************************************************
	KdPrint(("Leave DispatchRoutine() !\n"));
	return STATUS_SUCCESS;
}
