#ifndef ARGS_H
#define ARGS_H

#define va_list __builtin_va_list
#define va_start(v,l)  __builtin_va_start(v,l)
#define va_end(va) __builtin_va_end(va)
#define va_arg(a,type) __builtin_va_arg(a,type)

#endif /* ARGS_H */

