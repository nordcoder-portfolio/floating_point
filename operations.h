#ifndef CT_C24_LW_FLOATING_POINT_NORDCODER_OPERATIONS_H
#define CT_C24_LW_FLOATING_POINT_NORDCODER_OPERATIONS_H

#include "utils.h"

custom_float handle_exponent_overflow(uint8_t rounding, custom_float num, char format);
custom_float handle_exponent_underflow(custom_float num, char format, uint8_t rounding);
uint8_t check_special_cases(custom_float num, custom_float second, uint8_t opcode, char format);
custom_float handle_special_cases(custom_float first, custom_float second, uint8_t opcode, char format);
custom_float round_custom_float(custom_float num, uint8_t rounding, uint32_t rounding_data, uint8_t first_rounding_data_digit, char format);
custom_float sum(custom_float first, custom_float second, uint8_t rounding, char format);
custom_float sub(custom_float first, custom_float second, uint8_t rounding, char format);
custom_float mul(custom_float first, custom_float second, uint8_t rounding, char format);
custom_float division(custom_float first, custom_float second, uint8_t rounding, char format);

#endif	  // CT_C24_LW_FLOATING_POINT_NORDCODER_OPERATIONS_H
