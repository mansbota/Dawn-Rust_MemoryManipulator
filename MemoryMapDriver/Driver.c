#include "ntos.h"

#define WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x12, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define READ_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x13, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define SET_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x14, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ALLOCATE_POOL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x15, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FREE_POOL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x16, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define DRIVER_ENTRY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x17, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

const WCHAR symbolName[] = L"\\DosDevices\\aksaDev";
const WCHAR driverName[] = L"\\Device\\aksaDev";

PDEVICE_OBJECT DeviceObject;

NTSTATUS Unload(PDRIVER_OBJECT pDriverObject);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp);

typedef NTSTATUS(_stdcall* driverEntryFncPtr)();

//PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING fullImageName, HANDLE procID, PIMAGE_INFO imageInfo)
//{
//	WCHAR tempName[1024];
//
//	memcpy(tempName, fullImageName->Buffer, fullImageName->Length);
//
//	if (wcsstr(tempName, L"UnityPlayer.dll")) {
//
//		PID = procID;
//
//		moduleBase = imageInfo->ImageBase;
//	}
//	else if (wcsstr(tempName, L"GameAssembly.dll")) {
//
//		assemblyBase = imageInfo->ImageBase;
//	}
//}

NTSTATUS Unload(PDRIVER_OBJECT pDriverObj)
{
	UNICODE_STRING symbolLink;

	DbgPrint("Unload called!\n");
	RtlInitUnicodeString(&symbolLink, symbolName);

	//PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
	IoDeleteSymbolicLink(&symbolLink);
	IoDeleteDevice(pDriverObj->DeviceObject);
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

typedef struct _INFO_STRUCT
{
	ULONGLONG sourceAddress;
	ULONGLONG targetAddress;
	SIZE_T size;
	ULONG sourcePID;
	
} INFO_STRUCT, *PINFO_STRUCT;

NTSTATUS IOCTL(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);

	PVOID buffer = irp->AssociatedIrp.SystemBuffer;

	PINFO_STRUCT data = (PINFO_STRUCT)buffer;

	try
	{
		switch (stack->Parameters.DeviceIoControl.IoControlCode)
		{
			case ALLOCATE_POOL:
			{
				ULONGLONG address = (ULONGLONG)ExAllocatePool(NonPagedPool, data->size);

				data->targetAddress = address;

				break;
			}
			case FREE_POOL:
			{
				ExFreePool(data->targetAddress);

				break;
			}
			case WRITE_MEMORY:
			{
				PEPROCESS sourceProcess;

				status = PsLookupProcessByProcessId(data->sourcePID, &sourceProcess);

				if (status != STATUS_SUCCESS)
					break;

				SIZE_T bytes = 0;

				status = MmCopyVirtualMemory(sourceProcess, data->sourceAddress, PsGetCurrentProcess(), data->targetAddress, data->size, KernelMode, &bytes);

				ObDereferenceObject(sourceProcess);

				break;
			}
			case READ_MEMORY:
			{
				PEPROCESS sourceProcess;

				status = PsLookupProcessByProcessId(data->sourcePID, &sourceProcess);

				if (status != STATUS_SUCCESS)
					break;

				SIZE_T bytes = 0;

				status = MmCopyVirtualMemory(PsGetCurrentProcess(), data->targetAddress, sourceProcess, data->sourceAddress, data->size, KernelMode, &bytes);
				
				ObDereferenceObject(sourceProcess);

				break;
			}
			case SET_MEMORY:
			{
				memset(data->targetAddress, data->sourcePID, data->size);

				break;
			}
			case DRIVER_ENTRY:
			{
				driverEntryFncPtr function = (driverEntryFncPtr)(data->sourceAddress);

				NTSTATUS status = function();

				data->targetAddress = status;

				break;
			}
			default:
			{
				status = STATUS_INVALID_PARAMETER;

				break;
			}
		}
	}
	except(EXCEPTION_EXECUTE_HANDLER) {

		status = STATUS_INVALID_ADDRESS;

		return GetExceptionCode();
	}

	irp->IoStatus.Status = status;

	irp->IoStatus.Information = sizeof(INFO_STRUCT);

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("Driver Loaded!\n");

	//PsSetLoadImageNotifyRoutine(ImageLoadCallback);

	UNICODE_STRING dev, dos;

	RtlInitUnicodeString(&dev, driverName);   //driver name
	RtlInitUnicodeString(&dos, symbolName);   //driver symbol

	IoCreateDevice(DriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	IoCreateSymbolicLink(&dos, &dev);

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOCTL;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
	DriverObject->DriverUnload = Unload;

	DeviceObject->Flags |= DO_DIRECT_IO;
	DeviceObject->Flags &= DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}