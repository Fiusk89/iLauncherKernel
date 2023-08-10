#include <stdio.h>

static void print(const char *data, uint32_t data_length)
{
	for (uint32_t i = 0; i < data_length; i++)
	{
		uint8_t tab_length = 4;
		uint8_t value = *data++;
		switch (value)
		{
		case '\t':
		loop:
			dos_print_char(' ', 0x0f, 0);
			tab_length--;
			if (tab_length > 0)
				goto loop;
			break;
		default:
			dos_print_char(value, 0x0f, 0);
			break;
		}
	}
}

int32_t kprintf(const char *__restrict format, ...)
{
	va_list parameters;
	va_start(parameters, format);
	int32_t written = 0;
	uint32_t amount;
	bool rejected_bad_specifier = false;
	while (*format != '\0')
	{
		if (*format != '%')
		{
		print_c:
			amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			print(format, amount);
			format += amount;
			written += amount;
			continue;
		}
		const char *format_begun_at = format;
		if (*(++format) == '%')
			goto print_c;
		if (rejected_bad_specifier)
		{
		incomprehensible_conversion:
			rejected_bad_specifier = true;
			format = format_begun_at;
			goto print_c;
		}
		char c;
		const char *s;
		uint32_t u;
		float f;
		uint32_t x;
		switch (*format++)
		{
		case 'c':
			c = (char)va_arg(parameters, int);
			print(&c, sizeof(c));
			break;
		case 's':
			s = va_arg(parameters, const char *);
			print(s, strlen(s) - 1);
			break;
		case 'u':
			u = (uint32_t)va_arg(parameters, uint32_t);
			char itoa_char[256];
			itoa(itoa_char, (int64_t)u, 255);
			print(itoa_char, strlen(itoa_char));
			break;
		case 'f':
			f = (float)va_arg(parameters, double);
			char ftoa_char[256];
			ftoa(ftoa_char, f, 255);
			print(ftoa_char, strlen(ftoa_char));
			break;
		case 'x':
			x = (uint32_t)va_arg(parameters, uint32_t);
			dos_print_hex(x, 0x0f, 0);
			break;
		default:
			goto incomprehensible_conversion;
			break;
		}
	}
	va_end(parameters);
	return written;
}