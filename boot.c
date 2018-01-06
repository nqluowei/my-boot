    #include "setup.h"  
      
    extern void uart0_init(void);  
    extern void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);  
    extern void puts(char *str);  
    extern void puthex(unsigned int val);  
      
    static struct tag *params;  
      
    void setup_start_tag(void)  
    {  
        params = (struct tag *)0x30000100;  
      
        params->hdr.tag = ATAG_CORE;  
        params->hdr.size = tag_size (tag_core);  
      
        params->u.core.flags = 0;  
        params->u.core.pagesize = 0;  
        params->u.core.rootdev = 0;  
      
        params = tag_next (params);  
    }  
      
    void setup_memory_tags(void)  
    {  
        params->hdr.tag = ATAG_MEM;  
        params->hdr.size = tag_size (tag_mem32);  
          
        params->u.mem.start = 0x30000000;  
        params->u.mem.size  = 64*1024*1024;  
          
        params = tag_next (params);  
    }  
      
    int strlen(char *str)  
    {  
        int i = 0;  
        while (str[i])  
        {  
            i++;  
        }  
        return i;  
    }  
      
    void strcpy(char *dest, char *src)  
    {  
        while ((*dest++ = *src++) != '\0');  
    }  
      
    void setup_commandline_tag(char *cmdline)  
    {  
        int len = strlen(cmdline) + 1;  
          
        params->hdr.tag  = ATAG_CMDLINE;  
        params->hdr.size = (sizeof (struct tag_header) + len + 3) >> 2;  
      
        strcpy (params->u.cmdline.cmdline, cmdline);  
      
        params = tag_next (params);  
    }  
      
    void setup_end_tag(void)  
    {  
        params->hdr.tag = ATAG_NONE;  
        params->hdr.size = 0;  
    }  
      
    //start.S运行结束后进入主函数  
    int main(void)  
    {  
        void (*theKernel)(int zero, int arch, unsigned int params);  
      
        uart0_init();  
          
        /* copy kernel to memory from nandflash */  
        puts("MINI2440 simple bootloader:\n\r");  
        puts("\n\r");  
        puts("copy kernel to memory from nandflash ...\n\r");  
        nand_read(0x60000, (unsigned char *)0x30008000, 0x500000);  
        //nandflash起始地址0x60000，长度0x500000，待存入的ram内存起始地址30008000  
        puts("\n\r");  
      
        /* set params */  
        puts("Setting params ...\n\r");  
        setup_start_tag();  
        setup_memory_tags();  
        setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");  
        setup_end_tag();  
        puts("\n\r");  
        /* boot */  
        puts("Boot ...\n\r");  
        theKernel = (void (*)(int, int, unsigned int))0x30008000;  
        theKernel(0, 1999, 0x30000100);    
        /*  
         *  mov r0, #0 
         *  ldr r1, =1999 
         *  ldr r2, =0x30000100 
         *  mov pc, #0x30008000  
         */  
      
        puts("Error!\n\r");  
        /* error */  
      
        return -1;  
    }  
