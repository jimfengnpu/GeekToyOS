#include <lib/types.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <lib/error.h>

// from 6.828 course
/*
 * Print a number (base <= 16) in reverse order,
 * using specified putch function and associated pointer putdat.
 */
// change due to gcc __divdi3(ull, u32) failed, so just write one for u long long

static u64 udiv(unsigned long long num, unsigned div, unsigned *rem)
{
	// quot = (num * 2^n)/(div * 2^n)

}

static void printnum(void (*putch)(int, void*), void *putdat,
	unsigned long num, unsigned base, int width, int padc)
{
	// first recursively print all preceding (more significant) digits
	if (num >= base) {
		printnum(putch, putdat, num / base, base, width - 1, padc);
	} else {
		// print any needed pad characters before first digit
		while (--width > 0)
			putch(padc, putdat);
	}

	// then print this (the least significant) digit
	// bluesea
	// 这个做法相当巧妙！
	putch("0123456789abcdef"[num % base], putdat);
}


// modified due to va_arg/va_list in x86_64 builtin gcc with asm
// pass as pointer will cause dereference failed and access illegal addr
void vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list ap)
{
	register const char *p;
	register int ch, err;
	unsigned long long num;
	int base, lflag, width, precision, altflag, sgnflag;
	char padc;

	while (1) {
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0')
				return;
			putch(ch, putdat);
		}

		// Process a %-escape sequence
		padc = ' ';
		width = -1;
		precision = -1;
		lflag = 0;
		sgnflag = 0;
		altflag = 0;
	reswitch:
		switch (ch = *(unsigned char *) fmt++) {

		// flag to pad on the right
		case '-':
			padc = '-';
			goto reswitch;

		// flag to pad with 0's instead of spaces
		case '0':
			padc = '0';
			goto reswitch;

		// width field
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			for (precision = 0; ; ++fmt) {
				precision = precision * 10 + ch - '0';
				ch = *fmt;
				if (ch < '0' || ch > '9')
					break;
			}
			goto process_precision;

		case '*':
			precision = va_arg(ap, int);
			goto process_precision;

		case '.':
			if (width < 0)
				width = 0;
			goto reswitch;

		case '#':
			altflag = 1;
			goto reswitch;

		process_precision:
			if (width < 0){
				width = precision;
				precision = -1;
			}
			goto reswitch;

		// long flag (doubled for long long)
		case 'l':
			if(lflag < 2) lflag++;
			goto reswitch;

		// character
		case 'c':
			putch(va_arg(ap, int), putdat);
			break;

		// error message
		case 'e':
			err = va_arg(ap, int);
			if (err < 0)
				err = -err;
			if (err >= MAXERROR || (p = error_string[err]) == NULL)
				printfmt(putch, putdat, "error %d", err);
			else
				printfmt(putch, putdat, "%s", p);
			break;

		// string
		case 's':
			if ((p = va_arg(ap, char *)) == NULL)
				p = "(null)";
			if (width > 0 && padc != '-')
				for (width -= strnlen(p, precision); width > 0; width--)
					putch(padc, putdat);
			for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0); width--)
				if (altflag && (ch < ' ' || ch > '~'))
					putch('?', putdat);
				else
					putch(ch, putdat);
			for (; width > 0; width--)
				putch(' ', putdat);
			break;

		// (signed) decimal
		case 'd':
			sgnflag = 1;
			base = 10;
			goto number;

		// unsigned decimal
		case 'u':
			base = 10;
			goto number;

		// (unsigned) octal
		case 'o':
			// Replace this with your code.
			// bluesea
			base = 8;
			goto number;
			break;

		// pointer
		case 'p':
			putch('0', putdat);
			putch('x', putdat);
			lflag = 3;
			base = 16;
			goto number;

		// (unsigned) hexadecimal
		case 'x':
			base = 16;

		//bluesea
		//用printnum封装不同进制下的打印
		number:
			switch (lflag)
			{
				case 0: num = va_arg(ap, unsigned int); break;// %u/d
				case 1: num = va_arg(ap, unsigned long); break;// %l
				case 2: num = va_arg(ap, unsigned long long); break;// %ll
				case 3: num = (unsigned long long)va_arg(ap, void *); break;// %p
				default:
					break;
			}
			if(sgnflag){
				long long sgned = (long long)num;
				if(sgned < 0){
					putch('-', putdat);
					num = -sgned;
				}
			}
			printnum(putch, putdat, num, base, width, padc);
			break;

		// escaped '%' character
		case '%':
			putch(ch, putdat);
			break;

		// unrecognized escape sequence - just print it literally
		default:
			putch('%', putdat);
			for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing */;
			break;
		}
	}
}


void
printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintfmt(putch, putdat, fmt, ap);
	va_end(ap);
}

struct sprintbuf {
	char *buf;
	char *ebuf;
	int cnt;
};

static void
sprintputch(int ch, struct sprintbuf *b)
{
	b->cnt++;
	if (b->buf < b->ebuf)
		*b->buf++ = ch;
}

int
vsnprintf(char *buf, int n, const char *fmt, va_list ap)
{
	struct sprintbuf b = {buf, buf+n-1, 0};

	if (buf == NULL || n < 1)
		return -E_INVAL;

	// print the string to the buffer
	vprintfmt((void*)sprintputch, &b, fmt, ap);

	// null terminate the buffer
	*b.buf = '\0';

	return b.cnt;
}

int
snprintf(char *buf, int n, const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = vsnprintf(buf, n, fmt, ap);
	va_end(ap);

	return rc;
}