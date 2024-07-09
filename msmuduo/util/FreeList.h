#ifndef _MS_FREE_LIST
#define _MS_FREE_LIST

/*
* 
* https://github.com/chenBright/tinySTL/blob/master/tinySTL/alloc.h
* https://github.com/willyycn/MyTinySTL/blob/master/MyTinySTL/alloc.h
动态分区分配在当系统有很大的内存块的时候，分配内存必须要有一个策略。

1）首次适应：地址递增，顺序查找，第一个能满足的即分配给进程。

2）最佳适应：容量递增，找到第一个能满足要求的空闲分区。

3）最坏适应：容量递减，找到第一个能满足要求的分区。

4）邻近适应：循环首次适应算法。
*/




#endif