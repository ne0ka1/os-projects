# 自定义内存管理库

`libmem` 是轻量级的內存分配器，提供了动态内存分配和释放的基本功能，通过链表内存池模拟了标准 C 函数 `malloc` 和 `free` 的行为。

优点：
1. 自定义分配策略：实现了首次适配(first-fit)分配算法。在 128 MB 固定內存池中分配內存块。
2. 内存对齐与安全：使用 8 字节对齐，提高内存访问效率。防止分配 0 字节内存，并处理无效的内存地址。
3. 错误处理：提供了详尽的错误代码，包括释放未分配的内存，释放非当前进程所有的内存。
4. 内存可视化，提供了 `memoryMap()`函数，可以打印出已分配和空闲内存区域的可视化表格。

使用方法：
- 初始化: `myInitializeMemory()`
- 分配内存: `myMalloc(size)`
- 释放内存: `myFree(ptr)`
- 打印内存分布: `memoryMap()`
