#include "IO.h"

void print_num(uint8_t sign, int32_t exponent, uint32_t mantissa, char format)
{
	if (sign == 1)
		printf("-");
	printf("0x1.");
	for (uint8_t i = 0; i < count_mantissa_zeros(mantissa, format); i++)
		printf("0");
	printf("%xp", (mantissa << (format == 'h' ? 2 : 1)));
	if (exponent >= 0)
		printf("+");
	printf("%d", exponent);
}

void print_zero(uint8_t sign, char format)
{
	if (sign)
		printf("-");
	if (format == 'h')
		printf("0x0.000p+0");
	else if (format == 'f')
		printf("0x0.000000p+0");
}

void print_result(custom_float result, char format)
{
	if (IS_INF(result, format))
	{
		printf(result.sign == 1 ? "-" : "");
		printf("inf");
		return;
	}
	else if (IS_NAN(result, format))
	{
		printf("nan");
		return;
	}
	else if (IS_ZERO(result))
	{
		print_zero(result.sign, format);
		return;
	}
	else if (IS_SUBNORMAL(result))
	{
		result = handle_subnormal(result, format, true);
		print_num(result.sign, result.exponent, result.mantissa, format);
		return;
	}
	print_num(result.sign, result.exponent - (format == 'h' ? 15 : 127), result.mantissa, format);
}

void get_normal_representation(custom_float *result, int hex_num, char format)
{
	result->sign = ((hex_num >> (format == 'h' ? 15 : 31)) & 0x1);
	result->exponent =
		((hex_num >> (format == 'h' ? HALF_PRECISION_MANTISSA_LENGTH : SINGLE_PRECISION_MANTISSA_LENGTH)) &
		 (format == 'h' ? 0x1F : 0xFF));
	result->mantissa = (hex_num & (format == 'h' ? get_bit_mask(10) : get_bit_mask(23)));
}
