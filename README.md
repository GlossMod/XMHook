# XMHook

一个简易的 Hook 库.

由 XMDS 大佬完成, 

### 使用示例

```cpp

#pragma once
#include <cstdio>
#include <iostream>
#include <thread>
#include <XMHook.hpp>

static void *(*old_malloc)(_In_ _CRT_GUARDOVERFLOW size_t _Size) = nullptr;
static void *__cdecl proxy_malloc(_In_ _CRT_GUARDOVERFLOW size_t _Size)
{
    printf("hook 1: malloc (%d)\n", _Size);
    return old_malloc(_Size);
}

static void *(*old_malloc2)(_In_ _CRT_GUARDOVERFLOW size_t _Size) = nullptr;
static void *__cdecl proxy_malloc2(_In_ _CRT_GUARDOVERFLOW size_t _Size)
{
    printf("hook 2: malloc (%d)\n", _Size);
    return old_malloc2(_Size);
}

static void Hook_malloc()
{
    printf("Start hook malloc:\n");
    auto h = XM_InlineHook((xm_hook_func)malloc, (xm_hook_func)proxy_malloc, (xm_hook_func *)&old_malloc);
    auto h2 = XM_InlineHook((xm_hook_func)malloc, (xm_hook_func)proxy_malloc2, (xm_hook_func *)&old_malloc2);
    void *test_ptr = malloc(2024);
    free(test_ptr);
    XM_Unhook(h2);
    test_ptr = malloc(1024);
    XM_Unhook(h);
    free(test_ptr);
    printf("\n");
}

static int (*old_MessageBoxA)(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType) = nullptr;
static int proxy_MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
{
    lpText = "Hooked message box";
    lpCaption = "XMHook sample";
    return old_MessageBoxA(hWnd, lpText, lpCaption, uType);
}

static void Hook_MessageBoxA()
{
    printf("Start hook MessageBoxA:\n");
    auto h = XM_InlineHook((xm_hook_func)MessageBoxA, (xm_hook_func)proxy_MessageBoxA, (xm_hook_func *)&old_MessageBoxA);
    MessageBoxA(0, "XMHook", "XMHook Test", 0);
    XM_Unhook(h);
    printf("\n");
}

static void MultiThreadHook()
{
    printf("Start multi-thread hook malloc:\n");
    std::thread t1([]()
                   {
        XM_InlineHook((xm_hook_func)malloc, (xm_hook_func)proxy_malloc, (xm_hook_func*)&old_malloc);
        void* test_ptr = malloc(1);
        free(test_ptr); });
    std::thread t2([]()
                   {
        XM_InlineHook((xm_hook_func)malloc, (xm_hook_func)proxy_malloc2, (xm_hook_func*)&old_malloc2);
        void* test_ptr = malloc(2);
        free(test_ptr); });
    t1.join();
    t2.join();
}

int main()
{
    std::cout << "Hello XMHook!\n";

    Hook_malloc();
    MultiThreadHook();
    // Hook_MessageBoxA();

    system("pause");
    return 0;
}

```

### 感谢

- [XMDS](https://github.com/XMDS)
- [Hooking.Patterns](https://github.com/ThirteenAG/Hooking.Patterns) By ThirteenAG
- [Injector](https://github.com/thelink2012/injector) By thelink2012