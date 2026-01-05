#include <stdio.h>
#include <stdlib.h>

#include <lin99/matrix.h>

USE_ARITHMETIC_OP_SET_FP32

#define VECTOR_LEN 32

int main(void) {
	MAKE_VECTOR_FAST(vf32_MyVector, float, VECTOR_LEN, FP32)

	vctdstry(&vf32_MyVector);

	return EXIT_SUCCESS;
}
