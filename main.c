#include "IO.h"
#include "operations.h"
#include "return_codes.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int a, b;
	uint8_t rounding;
	uint8_t check = 0;
	if (argc > 3)
	{
		char format = *argv[1];
		if (strlen(argv[1]) == 1)
			check++;
		check += sscanf(argv[2], "%hhi", &rounding);
		check += sscanf(argv[3], "%x", &a);
		if (check != 3)
		{
			fprintf(stderr, "incorrect input");
			return ERROR_ARGUMENTS_INVALID;
		}
		if (format != 'f' && format != 'h')
		{
			fprintf(stderr, "incorrect format");
			return ERROR_ARGUMENTS_INVALID;	   // 20
		}
		if (!(rounding >= 0 && rounding < 4))
		{
			fprintf(stderr, "incorrect rounding");
			return ERROR_ARGUMENTS_INVALID;
		}
		if (argc > 4)
		{
			char op = *argv[4];
			if (strlen(argv[4]) == 1)
				check++;
			check += sscanf(argv[5], "%x", &b);
			if (check != 5)
			{
				fprintf(stderr, "incorrect input");
				return ERROR_ARGUMENTS_INVALID;
			}
			custom_float normal1;
			get_normal_representation(&normal1, a, format);
			custom_float normal2;
			get_normal_representation(&normal2, b, format);
			custom_float result;
			switch (op)
			{
			case '+':
				result = sum(normal1, normal2, rounding, format);
				break;
			case '-':
				result = sub(normal1, normal2, rounding, format);
				break;
			case '*':
				result = mul(normal1, normal2, rounding, format);
				break;
			case '/':
				result = division(normal1, normal2, rounding, format);
				break;
			default:
				fprintf(stderr, "incorrect operation");
				return ERROR_ARGUMENTS_INVALID;
			}
			print_result(result, format);
		}
		else
		{
			custom_float result;
			get_normal_representation(&result, a, format);
			print_result(result, format);
		}
	}
	else
	{
		fprintf(stderr, "input must contain an least 3 args");
		return ERROR_ARGUMENTS_INVALID;
	}
	return SUCCESS;
}
