#include "utils.h"

void right_shift(custom_float *num, uint8_t shift)
{
	num->exponent += shift;
	num->mantissa >>= shift;
}

uint64_t get_bit_mask(uint8_t length)
{
	return (uint64_t)((uint64_t)1 << (uint64_t)length) - 1;
}

uint8_t get_ith_bit(uint32_t num, uint8_t index_from_the_end)
{
	return (num >> index_from_the_end & 1);
}

uint8_t get_binary_length(uint64_t num)
{
	uint8_t counter = 0;
	while (num > 0)
	{
		num >>= 1;
		counter++;
	}
	return counter;
}

bool is_mantissa_overflow(custom_float num, char format)
{
	if (get_binary_length(num.mantissa) > MANTISSA_LENGTH(format))
		return true;
	return false;
}

uint8_t count_mantissa_zeros(uint32_t mantissa, char format)
{
	return (MANTISSA_LENGTH(format) - get_binary_length(mantissa)) / 4;
}

custom_float handle_subnormal(custom_float num, char format, bool to_print)
{
	int16_t cur_exponent = format == 'h' ? -14 : -126;
	while (get_ith_bit(num.mantissa, MANTISSA_LENGTH(format)) == 0)
	{
		num.mantissa <<= 1;
		cur_exponent -= 1;
	}
	num.mantissa = num.mantissa & get_bit_mask(MANTISSA_LENGTH(format));
	if (!to_print)
		cur_exponent += format == 'h' ? HALF_PRECISION_EXPONENT_BIAS - 1 : SINGLE_PRECISION_EXPONENT_BIAS - 1;
	num.exponent = cur_exponent;
	return num;
}

void check_handle_subnormal(custom_float *first, custom_float *second, char format, bool to_print)
{
	if (IS_SUBNORMAL(*first))
		*first = handle_subnormal(*first, format, to_print);
	if (IS_SUBNORMAL(*second))
		*second = handle_subnormal(*second, format, to_print);
}

void right_shift_and_memory_data_collection(custom_float *first_f, custom_float *second_f, uint32_t *rounding_memory, uint8_t *first_rounding_memory_digit)
{
	*first_rounding_memory_digit = (second_f->mantissa >> (custom_abs(first_f->exponent - second_f->exponent) - 1)) & 1;
	*rounding_memory = second_f->mantissa & (get_bit_mask(custom_abs(first_f->exponent - second_f->exponent)));
	right_shift(second_f, custom_abs(first_f->exponent - second_f->exponent));
}

void equalize_exponents(custom_float *first_f, custom_float *second_f, uint32_t *rounding_memory, uint8_t *first_rounding_memory_digit)
{
	if (first_f->exponent > second_f->exponent)
		right_shift_and_memory_data_collection(first_f, second_f, rounding_memory, first_rounding_memory_digit);
	else if (first_f->exponent < second_f->exponent)
		right_shift_and_memory_data_collection(second_f, first_f, rounding_memory, first_rounding_memory_digit);
}

bool compare(custom_float num1, custom_float num2)
{
	if (num1.exponent > num2.exponent)
	{
		return true;
	}
	else if (num1.exponent == num2.exponent)
	{
		return num1.mantissa >= num2.mantissa;
	}
	return false;
}

bool equals(custom_float num1, custom_float num2)
{
	return num1.exponent == num2.exponent && num1.mantissa == num2.mantissa && num1.sign == num2.sign;
}

uint64_t custom_abs(int64_t num)
{
	return num * (num > 0 ? 1 : -1);
}

void move_while_ith_not_one(custom_float *num, uint8_t i)
{
	while ((num->mantissa >> i & 1) == 0)
	{
		num->mantissa <<= 1;
		num->exponent -= 1;
	}
	num->exponent += 1;
}
