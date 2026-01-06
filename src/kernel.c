/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128
#define RAM_END 0x8000000

/* Idle loop - the NULL process */
void null_idle(void)
{
    while (1) {
        yield();
    }
}

void test(void)
{
    while (1)
    {
        serial_puts("test running\n");
        yield();
    }
}
void print_hex(uint32_t val)
{
    char hex[] = "0123456789ABCDEF";
    serial_puts("0x");

    for (int i = 28; i >= 0; i -= 4)
    {
        serial_putc(hex[(val >> i) & 0xF]);
    }
}

void test2(void)
{
    while (1)
    {
        serial_puts("Medium running\n");
        yield();
    }
}
void test3(void)
{
    while (1)
    {
        serial_puts("High running\n");
        yield();
    }
}
void blocker(void)
{
    serial_puts("Blocking now...\n");
    block_current();

    serial_puts("Woke up!\n");

    while (1)
    {
        yield();
    }
}

int receiver_pid;
int count = 0;

void sender(void)
{
    // int count = 0;
    while (1)
    {
        serial_puts("Sender sending message\n");
        send(receiver_pid, count);
        count++;
        yield();
    }
}

void receiver(void)
{
    while (1)
    {
        int m = receive();
        serial_puts("Received message\n");
        serial_putc('0' + m);
        serial_puts("\n");
        yield();
    }
}

void empty_process(void)
{
    serial_puts("EMPTY RUNNING\n");
    while (1) {
        yield();
    }
}

void ctx_test1(void)
{
    int x = 0;

    while (1) {
        x++;
        serial_puts("test1: ");
        serial_putc('0' + (x % 10));
        serial_puts("\n");
        yield();
    }
}

void ctx_test2(void)
{
    int y = 100;

    while (1) {
        y++;
        serial_puts("test2: ");
        serial_putc('0' + (y % 10));
        serial_puts("\n");
        yield();
    }
}


extern char __kernel_end;

void kmain(void)
{
    char input[MAX_INPUT];
    int pos = 0;
    /* Initialize hardware */
    serial_init();
    serial_puts("Boot OK!\n");
    
    /* Initialize memory and processes */
    meminit(&__kernel_end, (void *)RAM_END);
    process_init();
    
    /* Create test processes */
    process_create(empty_process, "empty");
    process_create(ctx_test1, "test1");
    process_create(ctx_test2, "test2");
    
    /* Initialize scheduler */
    scheduler_init();
    serial_puts("Scheduler initialized.\n");
    
    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Running null process...\n\n");
    
    /* Main loop - the "null process" */
    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;
        
        /* Read input line */
        while (1) {
            char c = serial_getc();
            
            /* Handle Enter key */
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);  /* Echo character */
            }
        }
        
        /* Echo back the input */
        if (pos > 0) {
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
        }
        /* Run the scheduler from here on out - never return */
        schedule();
    }
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
