#ifndef CT_C24_LW_FLOATING_POINT_NORDCODER_UTILS_H
#define CT_C24_LW_FLOATING_POINT_NORDCODER_UTILS_H

#define HALF_PRECISION_MANTISSA_LENGTH 10
#define SINGLE_PRECISION_MANTISSA_LENGTH 23
#define HALF_PRECISION_MAX_EXPONENT 31
#define SINGLE_PRECISION_MAX_EXPONENT 255
#define HALF_PRECISION_EXPONENT_BIAS 15
#define SINGLE_PRECISION_EXPONENT_BIAS 127
#define IS_ZERO(num) ((num).exponent == 0 && (num).mantissa == 0)
#define IS_POS_INF(num, format)                                                                                        \
	((num).sign == 0 && (num).mantissa == 0 &&                                                                         \
	 (num).exponent == (format == 'h' ? HALF_PRECISION_MAX_EXPONENT : SINGLE_PRECISION_MAX_EXPONENT))
#define IS_NEG_INF(num, format)                                                                                        \
	((num).sign == 1 && (num).mantissa == 0 &&                                                                         \
	 (num).exponent == (format == 'h' ? HALF_PRECISION_MAX_EXPONENT : SINGLE_PRECISION_MAX_EXPONENT))
#define IS_INF(num, format)                                                                                            \
	((num).mantissa == 0 && (num).exponent == (format == 'h' ? HALF_PRECISION_MAX_EXPONENT : SINGLE_PRECISION_MAX_EXPONENT))
#define IS_NAN(num, format)                                                                                            \
	((format == 'f' && (num).exponent == SINGLE_PRECISION_MAX_EXPONENT && (num).mantissa != 0) ||                      \
	 (format == 'h' && (num).exponent == HALF_PRECISION_MAX_EXPONENT && (num).mantissa != 0))
#define IS_SUBNORMAL(num) ((num).exponent == 0 && (num).mantissa != 0)
#define IS_EXPONENT_OVERFLOW(num, format)                                                                              \
	((num).exponent >= (format == 'h' ? HALF_PRECISION_MAX_EXPONENT : SINGLE_PRECISION_MAX_EXPONENT))
#define IS_EXPONENT_UNDERFLOW(num) ((num).exponent < 0)
#define MAX_EXP(format) (format == 'h' ? HALF_PRECISION_MAX_EXPONENT : SINGLE_PRECISION_MAX_EXPONENT)
#define MANTISSA_LENGTH(format) (format == 'h' ? HALF_PRECISION_MANTISSA_LENGTH : SINGLE_PRECISION_MANTISSA_LENGTH)
#define EXPONENT_BIAS(format) (format == 'h' ? HALF_PRECISION_EXPONENT_BIAS : SINGLE_PRECISION_EXPONENT_BIAS)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
	int8_t sign;
	int32_t exponent;
	uint32_t mantissa;
} custom_float;

void right_shift(custom_float *num, uint8_t shift);
uint64_t get_bit_mask(uint8_t length);
uint8_t get_ith_bit(uint32_t num, uint8_t index_from_the_end);
uint8_t get_binary_length(uint64_t num);
bool is_mantissa_overflow(custom_float num, char format);
uint8_t count_mantissa_zeros(uint32_t mantissa, char format);
custom_float handle_subnormal(custom_float num, char format, bool to_print);
void check_handle_subnormal(custom_float *first, custom_float *second, char format, bool to_print);
void equalize_exponents(custom_float *first_f, custom_float *second_f, uint32_t *rounding_memory, uint8_t *first_rounding_memory_digit);
bool compare(custom_float num1, custom_float num2);
bool equals(custom_float num1, custom_float num2);
uint64_t custom_abs(int64_t num);
void move_while_ith_not_one(custom_float *num, uint8_t i);

#endif	  // CT_C24_LW_FLOATING_POINT_NORDCODER_UTILS_H
