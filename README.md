# my-boot
一个精简的bootloader用于mini2440开发板

# SDRAM物理地址

0x3200 0000 SDRAM结束地址（SDRAM共64MB）

0x3050 8000 内核结束地址（内核占用5MB）

0x3000 8000 内核起始地址

0x3000 0000 SDRAM起始地址

虚拟地址 = 物理地址 - PHYS_OFFSET + PAGE_OFFSET

# 开发板上电后，首先运行start.S
1.关闭看门狗

2.设置FCLK、HCLK、PCLK比例

3.设置CPU工作模式

4.设置系统时钟为200MHz

5.使能高速缓冲

6.初始化SDRAM

7.从nandflash中复制出bootloader代码（源地址：0目的地址：_start(0x33f8 0000)数据长度：__bss_start减_start）

8.进入主函数

# 再运行boot.c
1.初始化uart0

2.从nandflash中复制出内核代码（源地址：0x60000目的地址：0x3000 8000数据长度：0x50 0000）

3.设置参数存入params结构体

4.最后是引导内核，利用函数指针跳到内核的第一个函数地址0x3000 8000

# boot.lds
提供给连接器arm-linux-ld的连接脚本，描述了输出文件boot.bin的映射图，即输出文件各段、各文件怎么放置
一个SECTIONS命令内部包含一个或多个段，段是连接脚本的基本单元，它表示输出文件某部分怎么放置。

从0x33f80000开始：
.text表示代码段

.rodata表示文件的只读数据段

.data表示所有文件的数据段

.bss表示全局变量的bss段（从_bss_start开始）

# init.c
主要定义了flash读函数和串口驱动函数，供给start.S和boot.c调用

# setup.h
主要定义了内核参数结构体
