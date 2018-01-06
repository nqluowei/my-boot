//定义了nand_flash读函数和串口驱动函数
/* nandflash controller */  
#define NFCONF (*((volatile unsigned long *)0x4E000000))  
#define NFCONT (*((volatile unsigned long *)0x4E000004))  
#define NFCMMD (*((volatile unsigned char *)0x4E000008))  
#define NFADDR (*((volatile unsigned char *)0x4E00000C))  
#define NFDATA (*((volatile unsigned char *)0x4E000010))  
#define NFSTAT (*((volatile unsigned char *)0x4E000020))  
  
/* GPIO */  
#define GPHCON              (*(volatile unsigned long *)0x56000070)  
#define GPHUP               (*(volatile unsigned long *)0x56000078)  
  
/* UART registers*/  
#define ULCON0              (*(volatile unsigned long *)0x50000000)  
#define UCON0               (*(volatile unsigned long *)0x50000004)  
#define UFCON0              (*(volatile unsigned long *)0x50000008)  
#define UMCON0              (*(volatile unsigned long *)0x5000000c)  
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)  
#define UTXH0               (*(volatile unsigned char *)0x50000020)  
#define URXH0               (*(volatile unsigned char *)0x50000024)  
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)  
 
#define TXD0READY   (1<<2)  
  
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);  

//复制bootloader的代码到sdrom(会在statrt.S中被调用)  
void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)  
{     
      
        nand_read((unsigned int)src, dest, len);  
  
}  
//清零bss内存段中的数据  
void clear_bss(void)  
{  
    extern int __bss_start, __bss_end;  
      
    int *p = &__bss_start;  
      
    for (; p < &__bss_end; p++) *p = 0;  
}  

//nand flash初始化  
void nand_init(void)  
{  
#define TACLS   0  
#define TWRPH0  1  
#define TWRPH1  0  
    /* sequence */  
    NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);  
    /* enable controller,init ECC,disable CS */  
    NFCONT = (1<<4)|(1<<1)|(1<<0);      
}  
//nand flash片选使能  
void nand_select(void)  
{  
    NFCONT &= ~(1<<1);      
}  
//nand flash片选解除
void nand_deselect(void)  
{  
    NFCONT |= (1<<1);   
}  
//nand flash设置指令
void nand_cmd(unsigned char cmd)  
{  
    volatile int i;  
    NFCMMD = cmd;  
    for (i = 0; i < 10; i++);  
}  
//设置需要访问的flash地址
void nand_addr(unsigned int addr)  
{  
    unsigned int col  = addr % 2048;  
    unsigned int page = addr / 2048;  
    volatile int i;  
  
    NFADDR = col & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR = (col >> 8) & 0xff;  
    for (i = 0; i < 10; i++);  
      
    NFADDR  = page & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR  = (page >> 8) & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR  = (page >> 16) & 0xff;  
    for (i = 0; i < 10; i++);      
}  
//等待nand flash准备完成 
void nand_wait_ready(void)  
{  
    while (!(NFSTAT & 1));  
}  
//从nand flash数据寄存器读出数据  
unsigned char nand_data(void)  
{  
    return NFDATA;  
}  

//从FLASH读数据到RAM  
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)  
{  
    int col = addr % 2048;  
    int i = 0;  
          
    /* select */  
    nand_select();  
  
    while (i < len)  
    {  
        /* read cmd 00h */  
        nand_cmd(0x00);  
  
        /* send addr */  
        nand_addr(addr);  
  
        /* read cmd 30h */  
        nand_cmd(0x30);  
  
        /* check */  
        nand_wait_ready();  
  
        /* read data */  
        for (; (col < 2048) && (i < len); col++)  
        {  
            buf[i] = nand_data();  
            i++;  
            addr++;  
        }  
          
        col = 0;  
    }  
  
    /* deselect */        
    nand_deselect();  
}  
 
#define PCLK            50000000    // PCLK:50MHz  
#define UART_CLK        PCLK        //  UART0:PCLK  
#define UART_BAUD_RATE  115200      // baudrate  
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16))-1)  
  
/*115200,8N1,none*/
//串口0初始化115200  
void uart0_init(void)  
{  
    GPHCON  |= 0xa0;    // GPH2,GPH3:TXD0,RXD0  
    GPHUP   = 0x0c;     // GPH2,GPH3 pull up  
  
    ULCON0  = 0x03;       
    UCON0   = 0x05;       
    UFCON0  = 0x00;       
    UMCON0  = 0x00;      
    UBRDIV0 = UART_BRD; // 115200  
}  
  
//串口打印字节  
void putc(unsigned char c)  
{  
      
    while (!(UTRSTAT0 & TXD0READY));  
      
    UTXH0 = c;  
}  
  
//串口打印字符串
void puts(char *str)  
{  
    int i = 0;  
    while (str[i])  
    {  
        putc(str[i]);  
        i++;  
    }  
}
