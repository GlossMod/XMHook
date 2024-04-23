#pragma once

#include <Hooking.Patterns.h>
#include <injector/injector.hpp>
#include <injector/hooking.hpp>
#include <log/Logger.hpp>

#define JMPSIZE 14

using namespace injector;
using namespace std;

namespace XMHook
{
    // 日志文件
    Logger logger("HookLog.txt");

    class Hook
    {
    public:
        LPVOID original;

        Hook()
        {
        }
        ~Hook() {}

        injector::memory_pointer_raw MakeAbsJMP(injector::memory_pointer_tr at, injector::memory_pointer_raw dest, bool vp = true)
        {
            injector::WriteMemory<uint16_t>(at, 0x25FF, vp);
            injector::WriteMemory<uint32_t>(at + sizeof(uint16_t), 0, vp);
            injector::WriteMemory<uint64_t>(at + sizeof(uint16_t) + sizeof(uint32_t), dest.as_int(), vp);
            return at.as_int() + JMPSIZE;
        }

        void HookByPattern(
            std::string_view hax_text,
            LPVOID loadFile_step2_fun,
            size_t buff_size = 128,
            size_t back_size = JMPSIZE)
        {
            hook::pattern pattern = hook::pattern(hax_text);
            if (!pattern.count_hint(1).empty())
            {
                try
                {
                    char *orig_func = new char[buff_size];
                    auto func_addr = pattern.get_first(0);

                    ProtectMemory(orig_func, buff_size, PAGE_EXECUTE_READWRITE);                         // 分配写入权限
                    memcpy(orig_func, func_addr, back_size);                                             // 备份原函数
                    original = (LPVOID)orig_func;                                                        // 赋值原始函数指针
                    MakeAbsJMP(&orig_func[back_size], (void *)((uintptr_t)func_addr + back_size), true); // 跳回原函数
                    MakeAbsJMP(func_addr, loadFile_step2_fun, true);                                     // 构造jmp在函数头部
                    // Logger::info("XMHook", " %s success", hax_text.data());
                    logger.Log(LogLevel::INFO, " %s success", hax_text.data());
                }
                catch (const std::exception &e)
                {
                    logger.Log(LogLevel::_ERROR, " %s error:: %s", hax_text.data(), e.what());
                }
            }
            else
            {
                logger.Log(LogLevel::_ERROR, " %s error", hax_text.data());
            }
        }
    };
}