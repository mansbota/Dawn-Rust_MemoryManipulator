
#include "helper.h"
#include "imports.h"
#include "cleaner.h"

#define OPERATION_READ 0x1
#define OPERATION_WRITE 0x2
#define OPERATION_GET_X64_MODULE 0x3
#define OPERATION_GET_X32_MODULE 0x4
#define OPERATION_CONNECT 0x5
#define OPERATION_DISCONNECT 0x6

typedef struct _INPUT_INFO {

	CHAR		operationType;
	DWORD		sourcePID;        //game pid
	ULONGLONG	sourceAddress;
	DWORD		targetPID;		  //this process pid
	ULONGLONG	targetAddress;
	SIZE_T		size;

} INPUT_INFO, * PINPUT_INFO;

PWORK_QUEUE_ITEM	 WorkItem					 = NULL;

const WCHAR			SharedSectionName[]			 = L"\\BaseNamedObjects\\Objekt";
const WCHAR			FinishedUMEventName[]		 = L"\\BaseNamedObjects\\BaseUMEvent";
const WCHAR			FinishedKMEventName[]		 = L"\\BaseNamedObjects\\BaseKMEvent";

HANDLE				sectionHandle				= NULL;
PVOID				sectionAddress				= NULL;

HANDLE				hFinishedUMEvent			= NULL;
HANDLE				hFinishedKMEvent			= NULL;
PKEVENT				FinishedUMEvent				= NULL;
PKEVENT				FinishedKMEvent				= NULL;

PEPROCESS			sourceProcess				= NULL;
PEPROCESS			targetProcess				= NULL;

BOOLEAN CreateSharedMemory() {

	UNICODE_STRING sectionName;
	RtlInitUnicodeString(&sectionName, SharedSectionName);

	OBJECT_ATTRIBUTES attributes;
	InitializeObjectAttributes(&attributes, &sectionName, OBJ_CASE_INSENSITIVE, NULL, NULL);

	LARGE_INTEGER maxSize;
	maxSize.QuadPart = sizeof(INPUT_INFO);

	if (!NT_SUCCESS(ZwCreateSection(&sectionHandle, SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE, &attributes, &maxSize, PAGE_READWRITE, SEC_COMMIT, NULL))) {
		
		return FALSE;
	}

	PACL pACL = NULL;

	PSECURITY_DESCRIPTOR pSecDes = NULL;

	if (!NT_SUCCESS(CreateStandardSCAndACL(&pSecDes, &pACL))) {
	
		ZwClose(sectionHandle);

		return FALSE;
	}

	if (!NT_SUCCESS(GrantAccess(sectionHandle, pACL))) {
		
		ExFreePool(pACL);

		ExFreePool(pSecDes);

		ZwClose(sectionHandle);

		return FALSE;
	}

	if (!NT_SUCCESS(GrantAccess(hFinishedKMEvent, pACL))) {
		
		ExFreePool(pACL);

		ExFreePool(pSecDes);

		ZwClose(sectionHandle);

		return FALSE;
	}

	if (!NT_SUCCESS(GrantAccess(hFinishedUMEvent, pACL))) {
		
		ExFreePool(pACL);

		ExFreePool(pSecDes);

		ZwClose(sectionHandle);

		return FALSE;
	}

	ExFreePool(pACL);

	ExFreePool(pSecDes);

	SIZE_T ulViewSize = 0;

	if (!NT_SUCCESS(ZwMapViewOfSection(sectionHandle, ZwCurrentProcess(), &sectionAddress, 0, maxSize.QuadPart, NULL, &ulViewSize, ViewUnmap, 0, PAGE_READWRITE))) {
		
		ZwClose(sectionHandle);

		return FALSE;
	}

	return TRUE;
}

BOOLEAN ReadAndProcessMemory() {

	PINPUT_INFO infoStruct = sectionAddress;

	switch (infoStruct->operationType) {

		case OPERATION_READ:
		{
			if (PsGetProcessId(sourceProcess) != infoStruct->sourcePID || PsGetProcessId(targetProcess) != infoStruct->targetPID)
				return FALSE;

			SIZE_T ReturnSize = 0;

			if (!NT_SUCCESS(MmCopyVirtualMemory(sourceProcess, infoStruct->sourceAddress, targetProcess, infoStruct->targetAddress, infoStruct->size, KernelMode, &ReturnSize)))
				return FALSE;

			return TRUE;
		}
		case OPERATION_WRITE:
		{
			if (PsGetProcessId(sourceProcess) != infoStruct->sourcePID || PsGetProcessId(targetProcess) != infoStruct->targetPID)
				return FALSE;

			SIZE_T ReturnSize = 0;

			if (!NT_SUCCESS(MmCopyVirtualMemory(targetProcess, infoStruct->targetAddress, sourceProcess, infoStruct->sourceAddress, infoStruct->size, KernelMode, &ReturnSize)))
				return FALSE;

			return TRUE;
		}
		case OPERATION_CONNECT:
		{
			if (!NT_SUCCESS(PsLookupProcessByProcessId(infoStruct->targetPID, &targetProcess)))
				return FALSE;	

			if (!NT_SUCCESS(PsLookupProcessByProcessId(infoStruct->sourcePID, &sourceProcess)))
				return FALSE;
			
			DWORD status = 1;
			
			SIZE_T ReturnSize = 0;

			if (!NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), &status, targetProcess, infoStruct->targetAddress, sizeof(DWORD), KernelMode, &ReturnSize)))
				return FALSE;

			return TRUE;
		}
		case OPERATION_DISCONNECT:
		{
			ObDereferenceObject(targetProcess);

			ObDereferenceObject(sourceProcess);

			return TRUE;
		}
		case OPERATION_GET_X64_MODULE:
		{
			LARGE_INTEGER time = { 0 };
			time.QuadPart = -250ll * 10 * 1000;     // 250 msec.

			if (PsGetProcessId(sourceProcess) != infoStruct->sourcePID || PsGetProcessId(targetProcess) != infoStruct->targetPID)
				return FALSE;

			WCHAR moduleName[256] = { 0 };

			SIZE_T ReturnSize = 0;

			if (!NT_SUCCESS(MmCopyVirtualMemory(targetProcess, infoStruct->targetAddress, PsGetCurrentProcess(), moduleName, infoStruct->size, KernelMode, &ReturnSize)))
				return FALSE;

			UNICODE_STRING moduleNameUnicode;

			RtlInitUnicodeString(&moduleNameUnicode, moduleName);

			KAPC_STATE state;

			KeStackAttachProcess(sourceProcess, &state);

			PPEB pPeb = PsGetProcessPeb(sourceProcess);

			if (!pPeb)
			{
				return FALSE;
			}

			// Wait for loader a bit
			for (INT i = 0; !pPeb->Ldr && i < 10; i++)
			{
				KeDelayExecutionThread(KernelMode, TRUE, &time);
			}

			// Still no loader
			if (!pPeb->Ldr)
			{
				return FALSE;
			}

			// Search in InLoadOrderModuleList
			for (PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink;
				pListEntry != &pPeb->Ldr->InLoadOrderModuleList;
				pListEntry = pListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

				if (RtlCompareUnicodeString(&pEntry->BaseDllName, &moduleNameUnicode, TRUE) == 0) {

					ULONGLONG address = (ULONGLONG)pEntry->DllBase;

					KeUnstackDetachProcess(&state);

					SIZE_T ReturnSize = 0;

					if (!NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), &address, targetProcess, infoStruct->sourceAddress, sizeof(ULONGLONG), KernelMode, &ReturnSize))) {

						return FALSE;
					}

					return TRUE;
				}
			}

			KeUnstackDetachProcess(&state);

			return FALSE;
		}
		case OPERATION_GET_X32_MODULE:
		{
			LARGE_INTEGER time = { 0 };
			time.QuadPart = -250ll * 10 * 1000;     // 250 msec.

			if (PsGetProcessId(sourceProcess) != infoStruct->sourcePID || PsGetProcessId(targetProcess) != infoStruct->targetPID)
				return FALSE;

			WCHAR moduleName[256] = { 0 };

			SIZE_T ReturnSize = 0;

			if (!NT_SUCCESS(MmCopyVirtualMemory(targetProcess, infoStruct->targetAddress, PsGetCurrentProcess(), moduleName, infoStruct->size, KernelMode, &ReturnSize)))
				return FALSE;

			UNICODE_STRING moduleNameUnicode;

			RtlInitUnicodeString(&moduleNameUnicode, moduleName);

			KAPC_STATE state;

			KeStackAttachProcess(sourceProcess, &state);

			PPEB32 pPeb32 = (PPEB32)PsGetProcessWow64Process(sourceProcess);

			if (pPeb32 == NULL)
			{
				return FALSE;
			}

			// Wait for loader a bit
			for (INT i = 0; !pPeb32->Ldr && i < 10; i++)
			{
				KeDelayExecutionThread(KernelMode, TRUE, &time);
			}

			// Still no loader
			if (!pPeb32->Ldr)
			{
				return FALSE;
			}

			// Search in InLoadOrderModuleList
			for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList.Flink;
				pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList;
				pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

				UNICODE_STRING ustr;

				RtlInitUnicodeString(&ustr, (PWCH)pEntry->BaseDllName.Buffer);

				if (RtlCompareUnicodeString(&ustr, &moduleNameUnicode, TRUE) == 0) {

					ULONGLONG address = (ULONGLONG)pEntry->DllBase;

					KeUnstackDetachProcess(&state);

					SIZE_T ReturnSize = 0;

					if (!NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), &address, targetProcess, infoStruct->sourceAddress, sizeof(ULONGLONG), KernelMode, &ReturnSize))) {

						return FALSE;
					}

					return TRUE;
				}
			}

			KeUnstackDetachProcess(&state);

			return FALSE;
		}
		default:
		{
			return FALSE;
		}
	}

	return FALSE;
}

void DriverThread(void* p) {

	UNICODE_STRING EventName, EventKMName;

	RtlInitUnicodeString(&EventName, FinishedUMEventName);

	FinishedUMEvent = IoCreateNotificationEvent(&EventName, &hFinishedUMEvent);

	if (!hFinishedUMEvent) {
		
		return;
	}

	KeResetEvent(FinishedUMEvent);

	RtlInitUnicodeString(&EventKMName, FinishedKMEventName);

	FinishedKMEvent = IoCreateNotificationEvent(&EventKMName, &hFinishedKMEvent);

	if (!hFinishedKMEvent) {
		
		ZwClose(hFinishedUMEvent);

		return;
	}

	if (!CreateSharedMemory()) {

		ZwClose(hFinishedUMEvent);

		ZwClose(hFinishedKMEvent);

		return;
	}

	while (TRUE) {

		ZwWaitForSingleObject(hFinishedUMEvent, FALSE, NULL);

		ReadAndProcessMemory();

		KeResetEvent(FinishedUMEvent);

		KeSetEvent(FinishedKMEvent, 0, FALSE);
	}
}

NTSTATUS DriverEntry(PDRIVER_OBJECT object, PUNICODE_STRING registry_path) {

	if (!clean_piddbcachetalbe())
		return STATUS_UNSUCCESSFUL;

	KeEnterGuardedRegion();

	UNREFERENCED_PARAMETER(object);
	UNREFERENCED_PARAMETER(registry_path);

	WorkItem = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));

	if (WorkItem != NULL) {

		ExInitializeWorkItem(WorkItem, DriverThread, WorkItem);

		ExQueueWorkItem(WorkItem, DelayedWorkQueue);
	}

	KeLeaveGuardedRegion();

	return STATUS_SUCCESS;
}