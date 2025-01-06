#pragma once
#include "spoofer.h"
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <TlHelp32.h>

static DWORD get_process_id(const wchar_t* process_name) {
	SPOOF_FUNC;
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

std::uintptr_t imagebase;

namespace driver {
	namespace codes {
		constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG get_base = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x699, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	HANDLE driver_handle;

	struct Request {
		HANDLE process_id;
		PVOID target;
		PVOID buffer;
		SIZE_T size;
		SIZE_T return_size;
		uintptr_t sex;
	};

	bool attach_to_process(const DWORD pid) {
		SPOOF_FUNC;
		Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);
		r.sex = 0x72734824;
		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	template <class T>
	T read_memory(const std::uintptr_t addr) {
		SPOOF_FUNC;
		T temp = {};

		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(temp);
		r.sex = 0x72734824;


		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;

	}

	template <class T>
	void write_memory(const std::uintptr_t addr, const T& value) {
		SPOOF_FUNC;
		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);
		r.sex = 0x72734824;

		DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	std::uintptr_t get_process_base() {
		SPOOF_FUNC;
		std::uintptr_t base = 0;
		Request r;
		r.buffer = &base;
		r.size = sizeof(base);
		r.return_size = 0;
		r.sex = 0x72734824;
		DeviceIoControl(driver_handle, codes::get_base, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		imagebase = base;
		return base;
	}

	bool init_drv(const wchar_t* pname) {
		SPOOF_FUNC;
		const DWORD pidd = get_process_id(pname);
		if (pidd == 0) {
			std::cout << "Proc not found\n";
			return false;
		}
		driver_handle = CreateFile(L"\\\\.\\GidraDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (not driver_handle) {
			std::cout << "Driver not found\n";
		}
		bool status = driver::attach_to_process(pidd);
		std::cout << status << '\n';
		return status;
	}
}

bool is_valid(const uint64_t adress)
{
	SPOOF_FUNC;

	if (adress <= 0x400000 || adress == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(adress) == nullptr || adress >
		0x7FFFFFFFFFFFFFFF) {
		return false;
	}
	return true;
}

template <typename T>
T read(std::uintptr_t address) {

	SPOOF_FUNC;
	if (driver::driver_handle) {
		if (is_valid(address)) {
			return driver::read_memory<T>(address);
		}
		else {
			return T();
		}
	}
}

template <typename T>
T write(std::uintptr_t address, T buffer) {
	SPOOF_FUNC;
	if (driver::driver_handle) {
		return driver::write_memory<T>(address, buffer);
	}
}

bool init(const wchar_t* x) {
	return driver::init_drv(x);
}

std::uintptr_t pbase() {
	return driver::get_process_base();
}

void stop() {
	CloseHandle(driver::driver_handle);
}
