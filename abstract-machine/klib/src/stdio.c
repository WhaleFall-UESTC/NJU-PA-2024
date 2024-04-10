#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <unistd.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFMAX 512
#define _unused

#define ZEROPAD 1
#define SIGN    2
#define PLUS    4
#define SPACE   8
#define LEFT    16
#define SPECIAL 32
#define SMALL   64

static char *number(char *str, int num, int base, int size, int precision, int type) {
  if (base < 2 || base > 36) return 0;

  char c, sign, tmp[36];
  int i = 0;
  const char *digits = (type & SMALL) ? \
    "0123456789abcdefghijklmnopqrstuvwxyz" : \
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  if (type & LEFT) type &= ~ZEROPAD;

  c = (type & ZEROPAD) ? '0' : ' ';

  if ((type & SIGN) && num < 0) sign = '-', num = -num;
  else sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);

  size -= sign ? 1 : 0;               // 有符号占一位
  if (type & SPECIAL) {                  
    if (base == 16) size -= 2;        // 0x占2位
    else if (base == 8) size -= 1;    // 0占1位
  }

  if (num == 0) tmp[i++] = '0';
  else do {
    tmp[i++] = digits[num % base];
    num /= base;
  } while (num != 0);

  precision = (i >precision) ? 1 : precision;
  size -= precision;

  if (!(type & (ZEROPAD | LEFT))) 
    while (size --> 0) 
      *str++ = ' ';

  if (sign) *str++ = sign;

  if (type & SPECIAL) {
    if (base == 8) *str++ = '0';
    else if (base == 16) *str++ = '0', *str++ = digits[33];
  }

  if (!(type & LEFT))
    while (size --> 0) *str++ = c;

  while (i < precision--) *str++ = '0';
  while (i-- > 0) *str++ = tmp[i];
  while (size --> 0) *str++ = ' ';

  return str;
}

#define is_digit(c) (c >= '0' && c <= '9')

static int skip_atoi(const char **s) {
  int i = 0;
  while (is_digit(**s))
    i = i * 10 + *((*s)++) - '0';
  return i;
}


int printf(const char *fmt, ...) {
  int i;
  char buf[BUFMAX];
  va_list args;

  va_start(args, fmt);
  write(1, buf, i = vsnprintf(buf, BUFMAX, fmt, args));
  va_end(args);
  return i;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, BUFMAX, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(out, fmt, args);
  va_end(args);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int i = vsnprintf(out, n, fmt, args);
  va_end(args);
  return i;
}

#define plus fmt++,cnt++

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int len, i, cnt = 0;
  char *str, *s, *ip;
  int flags;            // for number()

  for (str = out; *fmt && cnt < n; plus) {
    if (*fmt != '%') {
      *str++ = *fmt;
      continue;
    }

    flags = 0;
    repeat:
      plus;
      switch (*fmt) {
        case '+': flags |= PLUS;    goto repeat;
        case '-': flags |= LEFT;    goto repeat;
        case '#': flags |= SPECIAL; goto repeat;
        case '0': flags |= ZEROPAD; goto repeat;
      }
    
    int field_width = -1;
    if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
    else if (*fmt == '*') {
      plus;
      field_width = va_arg(ap, int);
      if (field_width < 0) {
        field_width = -field_width;
        flags |= LEFT;
      }
    }
    
    int precision = -1;
    if (*fmt == '.') {
      plus;
      if (is_digit(*fmt)) precision = skip_atoi(&fmt);
      else if (*fmt == '*') {
        precision = va_arg(ap, int);
      }
      if (precision < 0) precision = 0;
    }

    // int qualifier = -1;
    if (*fmt == 'h' || *fmt == '1' || *fmt == 'L') {
      // qualifier = *fmt;
      plus;
    }

    switch (*fmt) {
      case 'c':
        if (!(flags & LEFT))
          while (field_width --> 0) *str++ = ' ';
        *str++ = (unsigned char) va_arg(ap, int);
        while (field_width --> 0) *str++ =' ';
        break;

      case 's':
        s = va_arg(ap, char *);
        if (!s) s = "(null)";
        len = strlen(s);
        if (!(flags & LEFT))
          while (field_width --> 0) *str++ = ' ';
        for (i = 0; i < len; i++) *str++ = *s++;
        while (len < field_width--) *str++ = ' ';
        break;

      case 'o':
        str = number(str, va_arg(ap, unsigned long), 8, 
         field_width, precision, flags);
        break;

      case 'p':
        if (field_width == -1) {
          field_width = 8;
          flags |= ZEROPAD;
        }
        str = number(str, (unsigned long) va_arg(ap, void *), 16, 
         field_width, precision, flags);
        break;
      
      case 'x':
        flags |= SMALL;
      case 'X':
        str = number(str, va_arg(ap, unsigned long), 16, 
         field_width, precision, flags);
        break;

      case 'd':
      case 'i':
        flags |= SIGN;
      case 'u':
        str = number(str, va_arg(ap, unsigned long), 10, 
         field_width, precision, flags);
        break;

      case 'n':
        ip = (char *) va_arg(ap, int *);
        *ip = (str - out);
        break;

      default:
        if (*fmt != '%')
          *str++ = '%';
        if (*fmt)
          *str++ = *fmt;
        else
          fmt--, cnt--;
        break;
    }
  }
  *str = 0;
  return str - out;
}

#endif
