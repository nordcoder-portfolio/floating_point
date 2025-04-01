#ifndef CT_C24_LW_FLOATING_POINT_NORDCODER_IO_H
#define CT_C24_LW_FLOATING_POINT_NORDCODER_IO_H

#include "utils.h"

void print_num(uint8_t sign, int32_t exponent, uint32_t mantissa, char format);
void print_zero(uint8_t sign, char format);
void print_result(custom_float result, char format);
void get_normal_representation(custom_float *result, int hex_num, char format);

#endif	  // CT_C24_LW_FLOATING_POINT_NORDCODER_IO_H
