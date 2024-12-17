#include <ntifs.h>
#include <windef.h>
#include <intrin.h>


static const UINT64 pm = (~0xfull << 8) & 0xfffffffffull;

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
	NTKERNELAPI
		PVOID
		PsGetProcessSectionBaseAddress(
			__in PEPROCESS Process
		);
}

void debug_print(PCSTR text) {
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}


UINT64 GetProcessCr3(PEPROCESS Process) {
	if (!Process) return 0;
	uintptr_t process_dirbase = *(uintptr_t*)((UINT8*)Process + 0x28);
	if (process_dirbase == 0)
	{
		ULONG user_diroffset = 0x0388;
		process_dirbase = *(uintptr_t*)((UINT8*)Process + user_diroffset);
	}
	if ((process_dirbase >> 0x38) == 0x40)
	{
		uintptr_t SavedDirBase = 0;
		bool Attached = false;
		if (!Attached)
		{
			KAPC_STATE apc_state{};
			KeStackAttachProcess(Process, &apc_state);
			SavedDirBase = __readcr3();
			KeUnstackDetachProcess(&apc_state);
			Attached = true;
		}
		if (SavedDirBase) return SavedDirBase;

	}
	return process_dirbase;
}

NTSTATUS ReadPhysicalMemory(PVOID TargetAddress, PVOID Buffer, SIZE_T Size, SIZE_T* BytesRead) {
	MM_COPY_ADDRESS CopyAddress = { 0 };
	CopyAddress.PhysicalAddress.QuadPart = (LONGLONG)TargetAddress;
	return MmCopyMemory(Buffer, CopyAddress, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);

}


UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress) {
	DirectoryTableBase &= ~0xf;

	UINT64 PageOffset = VirtualAddress & ~(~0ul << 12);
	UINT64 PteIndex = ((VirtualAddress >> 12) & (0x1ffll));
	UINT64 PtIndex = ((VirtualAddress >> 21) & (0x1ffll));
	UINT64 PdIndex = ((VirtualAddress >> 30) & (0x1ffll));
	UINT64 PdpIndex = ((VirtualAddress >> 39) & (0x1ffll));

	SIZE_T ReadSize = 0;
	UINT64 PdpEntry = 0;
	ReadPhysicalMemory(PVOID(DirectoryTableBase + 8 * PdpIndex), &PdpEntry, sizeof(PdpEntry), &ReadSize);
	if (~PdpEntry & 1)
		return 0;

	UINT64 PdEntry = 0;
	ReadPhysicalMemory(PVOID((PdpEntry & pm) + 8 * PdIndex), &PdEntry, sizeof(PdEntry), &ReadSize);
	if (~PdEntry & 1)
		return 0;

	if (PdEntry & 0x80)
		return (PdEntry & (~0ull << 42 >> 12)) + (VirtualAddress & ~(~0ull << 30));

	UINT64 PtEntry = 0;
	ReadPhysicalMemory(PVOID((PdEntry & pm) + 8 * PtIndex), &PtEntry, sizeof(PtEntry), &ReadSize);
	if (~PtEntry & 1)
		return 0;

	if (PtEntry & 0x80)
		return (PtEntry & pm) + (VirtualAddress & ~(~0ull << 21));

	VirtualAddress = 0;
	ReadPhysicalMemory(PVOID((PtEntry & pm) + 8 * PteIndex), &VirtualAddress, sizeof(VirtualAddress), &ReadSize);
	VirtualAddress &= pm;

	if (!VirtualAddress)
		return 0;

	return VirtualAddress + PageOffset;
}



namespace driver {
	namespace codes {
		constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG get_base = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x699, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}
	struct Request {
		HANDLE process_id;
		PVOID target;
		PVOID buffer;
		SIZE_T size;
		SIZE_T return_size;
	};

	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}

	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}

	ULONG64 FindMin(INT32 A, SIZE_T B) {
		INT32 BInt = (INT32)B;
		return (((A) < (BInt)) ? (A) : (BInt));
	}


	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		debug_print("[+] Device control started");

		NTSTATUS status = STATUS_UNSUCCESSFUL;
		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		auto request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr) {
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		static PEPROCESS target_process = nullptr;
		const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
		switch (control_code)
		{
		case codes::attach:

			status = PsLookupProcessByProcessId(request->process_id, &target_process);
			break;
		case codes::read:
			if (target_process != nullptr) {
				ULONGLONG ProcessBase = GetProcessCr3(target_process);
				ObDereferenceObject(target_process);

				SIZE_T Offset = NULL;
				SIZE_T TotalSize = request->size;
				INT64 PhysicalAddress = TranslateLinearAddress(ProcessBase, (ULONG64)request->target + Offset);
				if (!PhysicalAddress)
					break;

				ULONG64 FinalSize = FindMin(PAGE_SIZE - (PhysicalAddress & 0xFFF), TotalSize);
				SIZE_T BytesRead = NULL;

				ReadPhysicalMemory(PVOID(PhysicalAddress), (PVOID)((ULONG64)request->buffer + Offset), FinalSize, &BytesRead);
				status = STATUS_SUCCESS;
				//status = MmCopyVirtualMemory(target_process, request->target, PsGetCurrentProcess(), request->buffer, request->size, KernelMode, &request->return_size);
			}
			break;
		case codes::write:
			if (target_process != nullptr) {
				status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer, target_process, request->target, request->size, KernelMode, &request->return_size);
			}
			break;
		case codes::get_base:
			if (target_process != nullptr) {
				ULONGLONG base_address = (ULONGLONG)PsGetProcessSectionBaseAddress(target_process);
				*reinterpret_cast<PVOID*>(request->buffer) = reinterpret_cast<PVOID>(base_address);
				request->return_size = sizeof(PVOID);
				status = STATUS_SUCCESS;
			}
			break;
		default:
			break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(Request);

		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return status;
	}
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);
	debug_print("[+] Driver initialization started");

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\GidraDriver");

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
	if (status != STATUS_SUCCESS) {
		debug_print("[-] failed to create device");
		return status;
	}

	debug_print("[+] Device created.");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\GidraDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (status != STATUS_SUCCESS) {
		debug_print("[-] failed to create symlink");
		return status;
	}

	debug_print("[+] Debug link created.");

	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	return status;
}

NTSTATUS DriverEntry() {
	debug_print("[+] Hi from kernel");

	UNICODE_STRING driver_name = {};

	RtlInitUnicodeString(&driver_name, L"\\Driver\\GidraDriver");

	return IoCreateDriver(&driver_name, &driver_main);
}