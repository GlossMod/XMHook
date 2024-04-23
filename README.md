# XMHook

一个简易的 Hook 库.

之前在 XMDS 大佬的指导下完成的. 

现在单独拿出来开源.

里面还自带了一个简易个 `Logger` 库.

### 使用方法

```cpp
#include <XMHook.hpp>


// 原始函数指针
void *(*loadFile_step2)(void *, void *, int) = nullptr;

// 代理函数
void *loadFile_step2_fun(void *a, wchar_t *b, int c)
{
    // ... 做点什么

    return loadFile_step2(a, b, c); // 返回原内容
}


void Load()
{
    // hook loadFile_step2
    auto loadFile_step2_hook = new XMHook::Hook();
    loadFile_step2_hook->HookByPattern(
        "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F1 44 89 41 08",
        (LPVOID)loadFile_step2_fun, 128, 15);
    loadFile_step2 = (void *(*)(void *, void *, int))loadFile_step2_hook->original;
}

```

### 感谢

- [XMDS](https://github.com/XMDS)
- [Hooking.Patterns](https://github.com/ThirteenAG/Hooking.Patterns) By ThirteenAG
- [Injector](https://github.com/thelink2012/injector) By thelink2012