#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include "uart.h"

#pragma weak _write
#pragma weak _read

int _write (int fd __unused, char *ptr, int len)
{
    int i;
	for(i=0;i<len;i++)
	{
		uart_putc(ptr[i]);
		if(ptr[i] == '\n')
			uart_putc('\r');
	}
	return i;
}

int _read(int fd __unused, char* ptr, int len)
{
	int i;
	for(i=0;i<len;i++)
		ptr[i] = uart_getc();
	return i;
}

/* _exit */
__attribute__((__used__)) void _exit(int status __unused) {
	while(1);
}

/* close */
__attribute__((__used__)) int _close(int file __unused) {
	return -1;
}

/* fstat */
__attribute__((__used__)) int _fstat(int file __unused, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

__attribute__((__used__)) int _getpid(void) {
	return 1;
}

__attribute__((__used__)) int _isatty(int file __unused) {
	return 1;
}

__attribute__((__used__)) int _kill(int pid __unused, int sig __unused) {
	errno = EINVAL;
	return -1;
}

__attribute__((__used__)) int _lseek(int file __unused, int ptr __unused, int dir __unused) {
	return 0;
}

__attribute__((__used__)) void *_sbrk(int incr) {
	extern char _heap_start;
	extern char _heap_end;
	static unsigned char *heap = (unsigned char *)(uintptr_t)(&_heap_start);
	unsigned char *prev_heap;
	prev_heap = heap;
	if((uintptr_t)(heap + incr) > (uintptr_t)&_heap_end)
	{
		_write(1, "Heap Overflow!\n\r", 16);
		while(1) asm volatile("");
	}
	heap += incr;
	return prev_heap;
}

/* environment */
char *__env[1] = { 0 };
char **environ = __env;

__attribute__((__used__)) __attribute__((__used__)) int link(char *old __unused, char *new __unused) {
	errno = EMLINK;
	return -1;
}

__attribute__((__used__)) __attribute__((__used__)) int _open(const char *name __unused, int flags __unused, int mode __unused) {
	return -1;
}

/* execve */
// int execve(char *name __unused, char **argv __unused, char **env __unused) {
// 	errno = ENOMEM;
// 	return -1;
// }

/* fork */
__attribute__((__used__)) __attribute__((__used__)) int fork(void) {
	errno = EAGAIN;
	return -1;
}


__attribute__((__used__)) __attribute__((__used__)) int stat (const char *__restrict __path __unused, struct stat *__restrict __sbuf ) {
	__sbuf->st_mode = S_IFCHR;
	return 0;
}

// int times(struct tms *buf) {
//   return -1;
// }

__attribute__((__used__)) __attribute__((__used__)) int unlink(char *name __unused) {
	errno = ENOENT;
	return -1;
}

__attribute__((__used__)) __attribute__((__used__)) int wait(int *status __unused) {
	errno = ECHILD;
	return -1;
}

// typedef void (*ptr_func_t)();
// extern char __preinit_array_start;
// extern char __preinit_array_end;

// extern char __init_array_start;
// extern char __init_array_end;

// extern char __fini_array_start;
// extern char __fini_array_end;

// /** Call constructors for static objects
//  */
// void call_init_array() {
//     uintptr_t* func = (uintptr_t*)&__preinit_array_start;
//     while (func < (uintptr_t*)&__preinit_array_end) {
// 		(*(ptr_func_t)(*func))();
//         func++;
//     }

//     func = (uintptr_t*)&__init_array_start;
//     while (func < (uintptr_t*)&__init_array_end) {
//         (*(ptr_func_t)(*func))();
//         func++;
//     }
// }

// /** Call destructors for static objects
//  */
// void call_fini_array() {
//     ptr_func_t array = (ptr_func_t)&__fini_array_start;
//     while (array < (ptr_func_t)&__fini_array_end) {
//         (*array)();
//         array++;
//     }
// }



__attribute__((__used__)) void _fini()
{
	return;
}