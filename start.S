.text  
.global _start  
_start:  
  
//r0 是寄存器地址
//r1 是要写入寄存器的值
//str则是将r1的值存入寄存器中 
//1.关闭看门狗WTCON=0
    ldr r0, =0x53000000  
    mov r1, #0  
    str r1, [r0]  
@关闭看门狗  
@数据手册：WTCON 0x53000000 R/W Watchdog timer control register  
  

//2.设置FCLK、HCLK、PCLK比例
    ldr r0, =0x4c000014  
    mov r1, #0x03;            @ FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1  
    str r1, [r0]  
  
//3.设置cpu总线模式为异步总线模式
    mrc p15, 0, r1, c1, c0, 0       /* read */   
    orr r1, r1, #0xc0000000         /* set asynchronous bus mode */  
    mcr p15, 0, r1, c1, c0, 0       /* write */  
  
//4.设置MPLL为200MHz
    ldr r0, =0x4c000004  
    ldr r1, =((0x5c<<12)|(0x01<<4)|(0x02)) @MPLL:200MHz  
    str r1, [r0]  
@设置系统时钟  
@如果HDIVN不是0，CPU的总线模式应该设置成asynchronous bus mode  
  
//5.使能高速缓冲
    mrc p15, 0, r0, c1, c0, 0   @ read control register  
    orr r0, r0, #(1<<12)  
    mcr p15, 0, r0, c1, c0, 0   @ write back  
@使能高速缓存，为系统提速，此段可不要，但程序执行速度要慢  
  
//6.设置SDRAM相关的寄存器，13个寄存器的配置见本文最底部config
    ldr r0, =0x48000000   @MEM_CTL_BASE  
    adr r1, config     /* sdram config address */  
    add r3, r0, #(52)       @13*4  
1:  
    ldr r2, [r1], #4  
    str r2, [r0], #4  
    cmp r0, r3  
    bne 1b  
@初始化SDRAM  
@根据数据手册对与SDRAM有关的13个寄存器进行配置  

//7.从nandflash中复制出bootloader代码(重定位)
    ldr sp, =0x34000000  
    bl nand_init //调用flash初始化函数 
  
    mov r0, #0 //flash源地址 
    ldr r1, =_start  //sdrom目的地址
    ldr r2, =__bss_start  
    sub r2, r2, r1  //数据长度
      
    bl copy_code_to_sdram //调用flash读函数,三个参数分别为flash源地址，sdrom目的地址，复制长度 
    bl clear_bss //调用清除boss内存段函数 
@把bootloader本身的代码从nandflash复制到它的链接地址去     

//8.跳转到主函数      
    ldr lr, =halt  
    ldr pc, =main  
halt:  
    b halt  
@跳转到main函数执行  
  
config:  
    .long 0x22011110     @BWSCON  
    .long 0x00000700     @BANKCON0  
    .long 0x00000700     @BANKCON1  
    .long 0x00000700     @BANKCON2  
    .long 0x00000700     @BANKCON3    
    .long 0x00000700     @BANKCON4  
    .long 0x00000700     @BANKCON5  
    .long 0x00018005     @BANKCON6  
    .long 0x00018005     @BANKCON7  
    .long 0x008C04F4     @REFRESH  
    .long 0x000000B1     @BANKSIZE  
    .long 0x00000030     @MRSRB6  
    .long 0x00000030     @MRSRB7
