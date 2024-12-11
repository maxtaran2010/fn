#include <iostream>

#include <Windows.h>
#include <string>
#include <tchar.h>
#include <TlHelp32.h>


static DWORD get_process_id(const wchar_t* process_name) {
	DWORD process_id = 0;
	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snap_shot == INVALID_HANDLE_VALUE) {
		return process_id;
	}

	PROCESSENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));
	
	if (Process32FirstW(snap_shot, &entry) == TRUE) {
		if (_wcsicmp(process_name, entry.szExeFile) == 0) {
			process_id = entry.th32ProcessID;
		}
		else {
			while (Process32NextW(snap_shot, &entry) == TRUE) {
				if (_wcsicmp(process_name, entry.szExeFile) == 0) {
					process_id = entry.th32ProcessID;
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);
	return process_id;

}



static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
	std::uintptr_t baseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to create snapshot: " << GetLastError() << std::endl;
		return 0;
	}

	MODULEENTRY32W moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32W);

	if (Module32FirstW(hSnapshot, &moduleEntry)) {
		// The first module is always the main executable module of the process.
		baseAddress = (std::uintptr_t)moduleEntry.modBaseAddr;
		std::cout << "Module name: ";
		std::wcout << moduleEntry.szModule << L"\n";
	}
	else {
		std::cerr << "Failed to retrieve module information: " << GetLastError() << std::endl;
	}

	CloseHandle(hSnapshot);
	return baseAddress;

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

	bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
		Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);
		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	template <class T>
	T read_memory(HANDLE driver_handle, const std::uintptr_t addr) {
		T temp = {};

		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(temp);


		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;

	}

	template <class T>
	void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value) {
		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	std::uintptr_t get_process_base(HANDLE driver_handle, wchar_t* module_name) {
		std::uintptr_t base = 0;
		Request r;
		r.buffer = &base;
		r.target = reinterpret_cast<PVOID>(module_name);
		r.size = sizeof(base);
		r.return_size = 0;
		DeviceIoControl(driver_handle, codes::get_base, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return base;
	}
}

int main() {
	const DWORD pid = get_process_id(L"FortniteLauncher.exe");
	//const DWORD pid = get_process_id(L"notepad.exe");
	if (pid == 0) {
		std::cout << "not found \n";
	}
	const HANDLE driver = CreateFile(L"\\\\.\\GidraDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver::attach_to_process(driver, pid) == true) {
		std::cout << "successfull attachment\n";
		uint64_t process_base = driver::get_process_base(driver, (wchar_t*)L"a");
		//ULONGLONG process_base = 0x0;
		std::cout << "Process base(new): " << std::hex << process_base << std::endl;
		for (int i = 0; i < 2500; i++) {
			__int32 c = driver::read_memory<__int32>(driver, process_base + (i * 0x1000) + 0x250);
			std::cout << "0x" << std::hex << c << std::endl;
		}
	}
	std::cout << "finish\n";
	std::cin.get();
	CloseHandle(driver);
	return 0;
}