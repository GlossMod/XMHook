#pragma once
#include <Windows.h> // for windows api
#include <cstdint>	 // uintptr_t
#include <mutex>	 // mutex

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#include "injector/injector.hpp"

#if defined(_WIN64) || defined(__x86_64__)
#include "hde/hde64.h"
#elif defined(_WIN32) || defined(__i386__)
#include "hde/hde32.h"
#endif

#if defined(_WIN64) || defined(__x86_64__)
#define XM_BUF_SIZE 32
#define JMP_INST_SIZE 5
#define CALL_INST_SIZE 5

#elif defined(_WIN32) || defined(__i386__)
#define XM_BUF_SIZE 32
#define JMP_INST_SIZE 5
#define CALL_INST_SIZE 5
#endif

#define IS_JMP_INST(inst) (inst == 0xE9)
#define IS_CALL_INST(inst) (inst == 0xE8)

typedef void *xm_hook_handle;
typedef void *xm_hook_func;
typedef void *xm_hook_ptr_t;
typedef uintptr_t xm_hook_addr_t;

std::mutex g_hook_mutex;

typedef struct
{
	xm_hook_func func;
	xm_hook_func new_func;
	char inst_backup[JMP_INST_SIZE]; // backup the original instruction
	uint8_t inst_backup_len;		 // backup the original instruction length

	union
	{
		char orig_func[XM_BUF_SIZE]; // original function
		xm_hook_ptr_t prev_func;	 // previous hook function

	} old_func;

} xm_hook_info_t;

xm_hook_handle XM_InlineHook(xm_hook_func func, xm_hook_func new_func, xm_hook_func *old_func)
{
	if (func == nullptr || new_func == nullptr)
	{
		return nullptr;
	}
	std::lock_guard<std::mutex> lock(g_hook_mutex);

	// initialize hook info
	xm_hook_info_t *handle = new xm_hook_info_t();
	handle->func = func;
	handle->new_func = new_func;
	handle->inst_backup_len = 0;

	xm_hook_addr_t inst_addr = reinterpret_cast<xm_hook_addr_t>(func);
	bool is_hook = false;
	xm_hook_ptr_t branch_inst_addr = nullptr;
#if defined(_WIN64) || defined(__x86_64__)
	hde64s hde;
#elif defined(_WIN32) || defined(__i386__)
	hde32s hde;
#endif

	while (handle->inst_backup_len < JMP_INST_SIZE)
	{
		xm_hook_ptr_t addr = reinterpret_cast<xm_hook_ptr_t>(inst_addr + handle->inst_backup_len);
#if defined(_WIN64) || defined(__x86_64__)
		handle->inst_backup_len += hde64_disasm(addr, &hde);
#elif defined(_WIN32) || defined(__i386__)
		handle->inst_backup_len += hde32_disasm(addr, &hde);
#endif
		if (hde.flags & F_ERROR)
		{
			return nullptr;
		}

		// printf("opcode: %02X, len: %d\n", hde.opcode, hde.len);
		if (IS_CALL_INST(hde.opcode) || IS_JMP_INST(hde.opcode))
		{
			if (handle->inst_backup_len == JMP_INST_SIZE)
			{
				is_hook = true;
				break;
			}
			branch_inst_addr = addr;
			break;
		}
	}

	DWORD old_protect;
	VirtualProtect(func, handle->inst_backup_len, PAGE_EXECUTE_READWRITE, &old_protect); // set func access permission

	injector::WriteMemoryRaw(handle->inst_backup, func, handle->inst_backup_len, false); // backup the original instruction

	if (is_hook)
	{
		handle->old_func.prev_func = injector::GetBranchDestination(func, false).get();
		*old_func = old_func ? handle->old_func.prev_func : nullptr; // return previous hook function
	}
	else
	{
		injector::ProtectMemory(handle->old_func.orig_func, XM_BUF_SIZE, PAGE_EXECUTE_READWRITE); // set old func access permission

		for (uint8_t i = 0; i < handle->inst_backup_len;)
		{
			if (inst_addr + i == reinterpret_cast<xm_hook_addr_t>(branch_inst_addr))
			{
				xm_hook_ptr_t dest = injector::GetBranchDestination(branch_inst_addr, false).get();
				injector::MakeCALL(handle->old_func.orig_func + i, dest, false); // fix call instruction
				i += CALL_INST_SIZE;
			}
			else
			{
				handle->old_func.orig_func[i] = injector::ReadMemory<uint8_t>(inst_addr + i); // build original function
				i++;
			}
		}

		xm_hook_addr_t old = inst_addr + handle->inst_backup_len;
		injector::MakeJMP(handle->old_func.orig_func + handle->inst_backup_len, old, false); // jump to the original function
		*old_func = old_func ? handle->old_func.orig_func : nullptr;						 // return original function
	}

	injector::MakeJMP(func, new_func, false); // jump to the new function

	VirtualProtect(func, handle->inst_backup_len, old_protect, nullptr); // restore func access permission
	return handle;
}

void XM_Unhook(xm_hook_handle handle)
{
	if (handle)
	{
		std::lock_guard<std::mutex> lock(g_hook_mutex);

		xm_hook_info_t *info = static_cast<xm_hook_info_t *>(handle);
		DWORD old_protect;
		VirtualProtect(info->func, info->inst_backup_len, PAGE_EXECUTE_READWRITE, &old_protect); // set func access permission
		injector::WriteMemoryRaw(info->func, info->inst_backup, info->inst_backup_len, false);
		VirtualProtect(info->func, info->inst_backup_len, old_protect, nullptr); // restore func access permission

		delete info;
	}
}
