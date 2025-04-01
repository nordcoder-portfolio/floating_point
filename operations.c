#include "operations.h"

enum
{
	TOWARDS_ZERO = 0,
	TIES_TO_EVEN = 1,
	TOWARDS_POS_INF = 2,
	TOWARDS_NEG_INF = 3
};

enum
{
	MULTIPLICATION = 0,
	DIVISION = 1,
	SUBTRACTION = 2,
	ADDITION = 3
};

enum
{
	NUM = 0,
	NaN = 1,
	POS_INF = 2,
	NEG_INF = 3
};

custom_float handle_exponent_overflow(uint8_t rounding, custom_float num, char format)
{
	if ((rounding == TOWARDS_ZERO) || (rounding == TOWARDS_POS_INF && num.sign == 1) || (rounding == TOWARDS_NEG_INF && num.sign == 0))
	{
		custom_float result = { num.sign, MAX_EXP(format) - 1, (1 << (MANTISSA_LENGTH(format))) - 1 };
		return result;
	}
	else if (rounding == TIES_TO_EVEN || (rounding == TOWARDS_POS_INF && num.sign == 0) || (rounding == TOWARDS_NEG_INF && num.sign == 1))
	{
		custom_float result = { num.sign, MAX_EXP(format), 0 };
		return result;
	}
	return num;
}

custom_float handle_exponent_underflow(custom_float num, char format, uint8_t rounding)
{
	if (custom_abs(num.exponent) > MANTISSA_LENGTH(format) + 1)
	{
		num.mantissa = 0;
		num.exponent = 0;
		return num;
	}
	num.mantissa += 1 << (MANTISSA_LENGTH(format));
	uint32_t rounding_data = num.mantissa & get_bit_mask(custom_abs(num.exponent - 1));
	uint8_t first_rounding_data_digit = (num.mantissa >> custom_abs(num.exponent)) & 1;
	right_shift(&num, num.exponent * -1);
	num.mantissa >>= 1;
	return round_custom_float(num, rounding, rounding_data, first_rounding_data_digit, format);
}

void handle_zero_exponent(custom_float *first, custom_float *second, uint64_t mantissa_length)
{
	if (first->exponent == 0)
	{
		move_while_ith_not_one(first, mantissa_length);
		first->mantissa -= 1 << mantissa_length;
	}
	if (second->exponent == 0)
	{
		move_while_ith_not_one(second, mantissa_length);
		second->mantissa -= 1 << mantissa_length;
	}
}

uint8_t check_special_cases(custom_float first, custom_float second, uint8_t opcode, char format)
{
	if (IS_NAN(first, format) || IS_NAN(second, format))
		return NaN;
	switch (opcode)
	{
	case MULTIPLICATION:
		if ((IS_ZERO(first) && IS_INF(second, format)) || (IS_INF(first, format) && IS_ZERO(second)))
			return NaN;
		else if (IS_INF(first, format) || IS_INF(second, format))
			return first.sign == second.sign ? POS_INF : NEG_INF;
		else
			return NUM;
	case DIVISION:
		if ((IS_ZERO(first) && IS_ZERO(second)) || (IS_INF(first, format) && IS_INF(second, format)))
			return NaN;
		else if (((IS_INF(first, format) || IS_INF(second, format)) && !(!IS_INF(first, format) && IS_INF(second, format)) ||
				  (!IS_ZERO(first) && IS_ZERO(second))))
			return first.sign == second.sign ? POS_INF : NEG_INF;
		else
			return NUM;
	case SUBTRACTION:
		if ((IS_POS_INF(first, format) && IS_POS_INF(second, format)) || (IS_NEG_INF(first, format) && IS_NEG_INF(second, format)))
			return NaN;
		else if ((IS_NEG_INF(first, format) || IS_POS_INF(second, format)))
			return NEG_INF;
		else if ((IS_POS_INF(first, format) || IS_NEG_INF(second, format)))
			return POS_INF;
		else
			return NUM;
	case ADDITION:
		if ((IS_NEG_INF(first, format) && IS_POS_INF(second, format)) || (IS_POS_INF(first, format) && IS_NEG_INF(second, format)))
			return NaN;
		else if (IS_POS_INF(first, format) || IS_POS_INF(second, format))
			return POS_INF;
		else if (IS_NEG_INF(first, format) || IS_NEG_INF(second, format))
			return NEG_INF;
		else
			return NUM;
	default:
		return NUM;
	}
}

custom_float handle_special_cases(custom_float first, custom_float second, uint8_t opcode, char format)
{
	uint8_t special_code = check_special_cases(first, second, opcode, format);
	switch (special_code)
	{
	case NUM:
	{
		custom_float result = { 0, 0, 0 };
		return result;
	}
	case NaN:
	{
		custom_float result = { 0, MAX_EXP(format), 1 };
		return result;
	}
	case POS_INF:
	{
		custom_float result = { 0, MAX_EXP(format), 0 };
		return result;
	}
	case NEG_INF:
	{
		custom_float result = { 1, MAX_EXP(format), 0 };
		return result;
	}
	default:
	{
		custom_float result = { 0, 0, 0 };
		return result;
	}
	}
}

custom_float round_custom_float(custom_float num, uint8_t rounding, uint32_t rounding_data, uint8_t first_rounding_data_digit, char format)
{
	switch (rounding)
	{
	case TOWARDS_ZERO:
		break;
	case TIES_TO_EVEN:
		if (first_rounding_data_digit == 1 &&
			((rounding_data > (1 << (get_binary_length(rounding_data) - 1))) || (num.mantissa & 1) == 1))
			num.mantissa++;
		break;
	case TOWARDS_POS_INF:
		if (rounding_data != 0)
			num.mantissa += num.sign ^ 1;
		break;
	case TOWARDS_NEG_INF:
		if (rounding_data != 0)
			num.mantissa += num.sign;
		break;
	default:
		return num;
	}
	if (is_mantissa_overflow(num, format))
	{
		num.mantissa &= (get_bit_mask(MANTISSA_LENGTH(format)));
		num.exponent += 1;
	}
	if (IS_EXPONENT_UNDERFLOW(num))
	{
		num = handle_exponent_underflow(num, format, rounding);
		if (IS_ZERO(num) && rounding == TOWARDS_POS_INF)
			num.mantissa += num.sign ^ 1;
		if (IS_ZERO(num) && rounding == TOWARDS_NEG_INF)
			num.mantissa += num.sign;
		return num;
	}
	if (IS_EXPONENT_OVERFLOW(num, format))
		return handle_exponent_overflow(rounding, num, format);
	return num;
}

custom_float mul(custom_float first, custom_float second, uint8_t rounding, char format)
{
	custom_float result = handle_special_cases(first, second, MULTIPLICATION, format);
	if (!IS_ZERO(result))
		return result;
	result.sign = first.sign ^ second.sign;
	if (IS_ZERO(first) || IS_ZERO(second))
	{
		result.exponent = 0;
		result.mantissa = 0;
		return result;
	}
	if (first.exponent == 0 || second.exponent == 0)
		result.exponent = 1;
	const uint8_t mantissa_length = (MANTISSA_LENGTH(format));
	const uint8_t exponent_bias = EXPONENT_BIAS(format);
	check_handle_subnormal(&first, &second, format, false);
	first.mantissa += (1 << mantissa_length);
	second.mantissa += (1 << mantissa_length);
	uint64_t result_mantissa = (uint64_t)(first.mantissa) * (second.mantissa);
	uint8_t binary_length = get_binary_length(result_mantissa);
	uint8_t length_to_shift = binary_length - (mantissa_length + 1);
	uint32_t rounding_memory = result_mantissa & (get_bit_mask(length_to_shift));
	uint8_t first_rounding_memory_digit = (result_mantissa >> (length_to_shift - 1)) & 1;
	result_mantissa >>= length_to_shift;
	result.mantissa = result_mantissa & (get_bit_mask(mantissa_length));
	result.exponent += first.exponent + second.exponent + (binary_length - (mantissa_length * 2) - 1) - exponent_bias;
	if (result.exponent < 0)
	{
		first_rounding_memory_digit = (result.mantissa >> (custom_abs(result.exponent - 1) - 1)) & 1;
		rounding_memory += (result.mantissa & get_bit_mask(custom_abs(result.exponent - 1)));
	}
	if (result.exponent == 0)
	{
		first_rounding_memory_digit = result_mantissa & 1;
		rounding_memory += first_rounding_memory_digit;
		result.mantissa += (1 << mantissa_length);
		result.mantissa >>= 1;
	}
	return round_custom_float(result, rounding, rounding_memory, first_rounding_memory_digit, format);
}

custom_float division(custom_float first, custom_float second, uint8_t rounding, char format)
{
	custom_float result = handle_special_cases(first, second, DIVISION, format);
	if (!IS_ZERO(result))
		return result;
	const uint8_t mantissa_length = (MANTISSA_LENGTH(format));
	const uint8_t exponent_bias = EXPONENT_BIAS(format);
	result.sign = first.sign ^ second.sign;
	if (IS_ZERO(first) && !IS_ZERO(second) || !IS_INF(first, format) && IS_INF(second, format))
	{
		result.exponent = 0;
		result.mantissa = 0;
		return result;
	}
	handle_zero_exponent(&first, &second, mantissa_length);
	uint64_t first_mantissa = (uint64_t)((first.mantissa + (1 << mantissa_length))) << (64 - mantissa_length - 1);
	uint64_t second_mantissa = ((second.mantissa + (1 << mantissa_length)));
	uint64_t result_mantissa = (first_mantissa / second_mantissa);
	uint8_t binary_length = get_binary_length(result_mantissa);
	uint8_t length_to_shift = binary_length - (mantissa_length + 1);
	uint64_t rounding_data = result_mantissa & (get_bit_mask(length_to_shift));
	uint8_t first_rounding_memory_digit = (result_mantissa >> (length_to_shift - 1)) & 1;
	result_mantissa >>= length_to_shift;
	result.mantissa = result_mantissa & (get_bit_mask(mantissa_length));
	result.exponent = first.exponent - second.exponent + exponent_bias - 1;
	if (rounding_data > 0 && rounding_data == get_bit_mask(get_binary_length(rounding_data) - 1) + 1)
		rounding_data += 1;
	if (binary_length == (format == 'h' ? 54 : 41))
		result.exponent += 1;
	if (result.exponent == 0)
	{
		first_rounding_memory_digit = result_mantissa & 1;
		result.mantissa += (1 << mantissa_length);
		result.mantissa >>= 1;
	}
	else if (result.exponent < 0)
	{
		if (custom_abs(result.exponent) > MANTISSA_LENGTH(format) + 1)
		{
			if (rounding == TOWARDS_POS_INF)
				result.mantissa = result.sign ^ 1;
			else if (rounding == TOWARDS_NEG_INF)
				result.mantissa = result.sign;
			else
				result.mantissa = 0;
			result.exponent = 0;
			return result;
		}
		result.mantissa += 1 << (MANTISSA_LENGTH(format));
		first_rounding_memory_digit = (result.mantissa >> (custom_abs(result.exponent))) & 1;
		rounding_data += (result.mantissa & get_bit_mask(custom_abs(result.exponent - 1)));
		right_shift(&result, result.exponent * -1);
		result.mantissa >>= 1;
	}
	if (IS_ZERO(result) && rounding == TOWARDS_POS_INF && rounding_data == 0)
		result.mantissa += result.sign ^ 1;
	if (IS_ZERO(result) && rounding == TOWARDS_NEG_INF && rounding_data == 0)
		result.mantissa += result.sign;
	return round_custom_float(result, rounding, rounding_data, first_rounding_memory_digit, format);
}

custom_float sub(custom_float first, custom_float second, uint8_t rounding, char format)
{
	custom_float result = handle_special_cases(first, second, SUBTRACTION, format);
	if (!IS_ZERO(result))
		return result;
	if (first.sign == 1 && second.sign == 1)
	{
		first.sign = 0;
		second.sign = 0;
		return sub(second, first, rounding, format);
	}
	else if (first.sign != second.sign)
	{
		second.sign ^= 1;
		return (sum(first, second, rounding, format));
	}
	const uint8_t mantissa_length = (MANTISSA_LENGTH(format));
	if (equals(first, second))
	{
		result.sign = rounding == TOWARDS_NEG_INF ? 1 : 0;
		result.exponent = 0;
		result.mantissa = 0;
		return result;
	}
	if (IS_ZERO(first))
	{
		second.sign = IS_ZERO(second) ? 0 : 1;
		return second;
	}
	else if (IS_ZERO(second))
		return first;
	result.sign = compare(first, second) ? 0 : 1;
	handle_zero_exponent(&first, &second, mantissa_length);
	int64_t first_mantissa = (int64_t)first.mantissa + (1 << mantissa_length);
	int64_t second_mantissa = (int64_t)second.mantissa + (1 << mantissa_length);
	if (custom_abs(first.exponent - second.exponent) > mantissa_length + 1)
	{
		if (first.exponent > second.exponent)
		{
			if (first.mantissa == 0 &&
				(rounding == TOWARDS_ZERO || rounding == TOWARDS_NEG_INF ||
				 custom_abs(first.exponent - second.exponent) == mantissa_length + 2 && second.mantissa != 0) &&
				rounding != TOWARDS_POS_INF)
			{
				result.exponent = first.exponent - 1;
				result.mantissa = get_bit_mask(mantissa_length);
				return result;
			}
			else
			{
				result.exponent = first.exponent;
				result.mantissa = first.mantissa;
			}
		}
		else
		{
			if (second.mantissa == 0 &&
				(rounding == TOWARDS_ZERO || rounding == TOWARDS_POS_INF ||
				 custom_abs(first.exponent - second.exponent) == mantissa_length + 2 && first.mantissa != 0 && rounding != TOWARDS_NEG_INF))
			{
				result.exponent = second.exponent - 1;
				result.mantissa = get_bit_mask(mantissa_length);
				return result;
			}
			else
			{
				result.exponent = second.exponent;
				result.mantissa = second.mantissa;
			}
		}
		if (rounding == TOWARDS_POS_INF)
			result.mantissa -= result.sign;
		else if (rounding == TOWARDS_NEG_INF)
			result.mantissa -= first.exponent > second.exponent;
		else if (rounding == TOWARDS_ZERO)
			result.mantissa -= 1;
		return result;
	}
	else
	{
		if (first.exponent > second.exponent)
		{
			first_mantissa <<= (first.exponent - second.exponent);
			first.exponent = second.exponent;
		}
		else
		{
			second_mantissa <<= (second.exponent - first.exponent);
			second.exponent = first.exponent;
		}
	}
	uint64_t result_mantissa = custom_abs(first_mantissa - second_mantissa);
	uint8_t result_length = get_binary_length(result_mantissa);
	uint32_t rounding_data = 0;
	uint8_t first_rounding_data_digit = 0;
	result.exponent = first.exponent;
	if (result_length > mantissa_length + 1)
	{
		rounding_data = result_mantissa & (get_bit_mask(result_length - mantissa_length - 1));
		first_rounding_data_digit = (result_mantissa >> (result_length - mantissa_length - 2)) & 1;
		result_mantissa >>= (result_length - mantissa_length - 1);
		result.exponent += (result_length - mantissa_length - 1);
	}
	while (((result_mantissa >> mantissa_length) & 1) != 1)
	{
		result_mantissa <<= 1;
		result.exponent -= 1;
	}
	result.mantissa = result_mantissa & get_bit_mask(mantissa_length);
	if (result.exponent == 0)
	{
		first_rounding_data_digit = result_mantissa & 1;
		result.mantissa += (1 << mantissa_length);
		result.mantissa >>= 1;
	}
	return round_custom_float(result, rounding, rounding_data, first_rounding_data_digit, format);
}

custom_float sum(custom_float first, custom_float second, uint8_t rounding, char format)
{
	const uint8_t mantissa_length = MANTISSA_LENGTH(format);
	custom_float result = handle_special_cases(first, second, ADDITION, format);
	if (!IS_ZERO(result))
		return result;
	result.sign = first.sign;
	if (IS_ZERO(first) || (IS_ZERO(second)))
		return IS_ZERO(first) ? second : first;
	if (first.sign == second.sign)
	{
		bool was_zero = false;
		if (first.exponent == 0 && second.exponent == 0)
			was_zero = true;
		handle_zero_exponent(&first, &second, mantissa_length);
		if (custom_abs(first.exponent - second.exponent) > mantissa_length + 1)
		{
			if (first.exponent > second.exponent)
			{
				result.exponent = first.exponent;
				result.mantissa = first.mantissa;
			}
			else
			{
				result.exponent = second.exponent;
				result.mantissa = second.mantissa;
			}
			if (rounding == TOWARDS_POS_INF)
				result.mantissa += result.sign ^ 1;
			else if (rounding == TOWARDS_NEG_INF)
				result.mantissa += result.sign;
			return result;
		}
		first.mantissa += 1 << mantissa_length;
		second.mantissa += 1 << mantissa_length;
		uint32_t rounding_data = 0;
		uint8_t first_rounding_data_digit = 0;
		equalize_exponents(&first, &second, &rounding_data, &first_rounding_data_digit);
		result.mantissa = first.mantissa + second.mantissa;
		uint8_t result_length = get_binary_length(result.mantissa);
		uint8_t mantissa_shift = (result_length <= mantissa_length ? 0 : result_length - (mantissa_length + 1));
		if (mantissa_shift != 0)
		{
			first_rounding_data_digit = ((result.mantissa >> (mantissa_shift - 1))) & 1;
			rounding_data += ((result.mantissa & get_bit_mask(mantissa_shift)) << mantissa_shift);
			if (rounding_data > 0 && rounding_data == get_bit_mask(get_binary_length(rounding_data) - 1) + 1)
				rounding_data += rounding == TIES_TO_EVEN ? -1 : 1;
			result.mantissa >>= mantissa_shift;
		}

		result.mantissa = (result.mantissa & (get_bit_mask(mantissa_length)));
		result.exponent = first.exponent + mantissa_shift;
		result.sign = first.sign;
		if (was_zero && result_length <= mantissa_length)
			result.exponent -= 1;

		return round_custom_float(result, rounding, rounding_data, first_rounding_data_digit, format);
	}
	else
	{
		if (second.sign == 1)
		{
			second.sign = 0;
			return sub(first, second, rounding, format);
		}
		else
		{
			first.sign = 0;
			return sub(second, first, rounding, format);
		}
	}
}
