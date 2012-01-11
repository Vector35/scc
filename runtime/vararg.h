#ifndef __LIBC__VARARG_H__
#define __LIBC__VARARG_H__

typedef void* va_list;

#define __va_arg_size(arg) ((sizeof(arg) + (sizeof(size_t) - 1)) & (~(sizeof(size_t) - 1)))
#define va_start(list, arg) list = (void*)((size_t)&arg + __va_arg_size(arg))
#define va_arg(list, type) (list = (void*)((size_t)list + __va_arg_size(type)), *((type*)((size_t)list - __va_arg_size(type))))
#define va_end(list)

#endif

