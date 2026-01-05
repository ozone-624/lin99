//TODO: Global zalloc use instead of calling zalloc every operation
//TODO: Add overflow protection on pfn_*-based operations
//TODO: Validate pfn_*'s in vctcreate (good luck)
//TODO: Add cross products
//TODO: Start matrix_t type in matrix.h
//TODO: Ensure vector_t supports non-abelian algebraic structures

/*
 * vector.h
 *
 * A pure C99 linear algebra header for type-generic vectors.
 *
 * Features:
 * - Type-generic vector representation using void pointers
 * - Element-wise operations (addition, subtraction, multiplication, division)
 * - Dot product, normalization, scalar scaling
 * - Custom memory management and arithmetic logic via function pointers
 *
 * This design allows support for multiple numerical types (e.g., float, int, double)
 * and allocation strategies by plugging in appropriate arithmetic and memory function sets.
 *
 * Hungarian Notation Key:
 * - pv_  : pointer to vector_t
 * - cpv_ : const pointer to vector_t
 * - psz_ : pointer to size_t
 * - csz_ : const size_t
 * - pfn_ : function pointer
 * - pu8_ : pointer to unsigned 8-bit integers
 *
 * The provided matrix.h and matrix.c files are explicitly designed to work with the implementations used in these files.
 *
 */


#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Use #define so that users can easily create their own type enums without going here
typedef int TYPE;
#define TYPE_NULL 0
#define TYPE_S8 	1
#define TYPE_U8 	2
#define TYPE_S16	3
#define TYPE_U16	4
#define TYPE_S32	5
#define TYPE_U32	6
#define TYPE_S64	7
#define TYPE_U64	8
#define TYPE_SZ 	9
//#define TYPE_MY_TYPE ...

// Use negative integers to represent non-integer types
#define TYPE_FP8  	-1
#define TYPE_FP16   -2
#define TYPE_FP32   -3
#define TYPE_FP64   -4

#define CHECK_ALLOCATION(buffer) (buffer != NULL)

// Create a generalized operation definition that allows the user to specify a custom name, type, operator, and allocation method.
/**
 * Writing custom operation functions.
 *
 * Imagine your vector_t is using a type which does not have easy-to-use operations provided by the C99 standard.
 * You would need a way of defining your own operations which could be used by vector_t.
 *
 * On top of what is already provided by vector.h, the ability to create and utilize custom operation definitions
 * is also possible thanks to the use of callback functions.  This is to serve as a tutorial for creating a simple
 * callback function.
 *
 * vector.h utilizes callback functions in a way that is comparable to the preexisting structure of functions like
 * vctadd, vctwrite, etc.  To write your own callback function, it must have exactly three parameters:
 *  - Parameter 1 (void*) - This is your result from the operation.  For addition, this would be the address to the location of the sum.
 *  - Parameter 2 (const void*) - This is your left operand location.  It is to be treated solely as readonly.  
 *  - Parameter 3 (const void*) - This is your right operand location. It, too, is to be treated as readonly.
 *
 * YOU DO NOT HAVE TO WORRY ABOUT MEMORY MANAGEMENT IN THESE CALLBACKS, vector.h handles that for you.
 *
 * To write operators, simply modify the dereferenced and interpreted value of the result (*(your_type*)your_result)
 * Once the operation is finished, call "return;."  Remember, this function does not return anything, all data storage is
 * handled through the parameters.
 *
 * For an example of how arithmetic callbacks should be written, please see the GENERAL_OP_DEFINITION macro
 */
#define GENERAL_OP_DEFINITION(name, type, op) \
void name(void* p_Result, const void* cp_A, const void* cp_B) { \
	*(type*)p_Result = *(type*)cp_A op *(type*)cp_B; \
	return; \
}

#define ARITHMETIC_OP_SET(type, abbr) \
GENERAL_OP_DEFINITION(Add##abbr, type, +) \
GENERAL_OP_DEFINITION(Subtract##abbr, type, -) \
GENERAL_OP_DEFINITION(Multiply##abbr, type, *) \
GENERAL_OP_DEFINITION(Divide##abbr, type, /)

// Create a set of type-specific arithmetic operations which can be individually used by the user
#define USE_ARITHMETIC_OP_SET_S8 ARITHMETIC_OP_SET(int8_t, S8)
#define USE_ARITHMETIC_OP_SET_U8 ARITHMETIC_OP_SET(uint8_t, U8)
#define USE_ARITHMETIC_OP_SET_S16 ARITHMETIC_OP_SET(int16_t, S16)
#define USE_ARITHMETIC_OP_SET_U16 ARITHMETIC_OP_SET(uint16_t, U16)
#define USE_ARITHMETIC_OP_SET_S32 ARITHMETIC_OP_SET(int32_t, S32)
#define USE_ARITHMETIC_OP_SET_U32 ARITHMETIC_OP_SET(uint32_t, U32)
#define USE_ARITHMETIC_OP_SET_S64 ARITHMETIC_OP_SET(int64_t, S64)
#define USE_ARITHMETIC_OP_SET_U64 ARITHMETIC_OP_SET(uint64_t, U64)

#define USE_ARITHMETIC_OP_SET_FP32 ARITHMETIC_OP_SET(float, FP32)
#define USE_ARITHMETIC_OP_SET_FP64 ARITHMETIC_OP_SET(double, FP64)

#define ARITHMETIC_OP_DEF(type) \
GENERAL_OP_DEFINITION(add, type, +) \
GENERAL_OP_DEFINITION(sub, type, -) \
GENERAL_OP_DEFINITION(mul, type, *) \
GENERAL_OP_DEFINITION(div, type, /)

/**
 * zalloc - One-argument access to the calloc() function.
 *
 * Parameters:
 *  - sz_Size: Size of requested allocation
 *
 * Returns:
 *  - On success: Pointer to the beginning of the allocation
 *  - On failure: NULL
 */
static void* zalloc(size_t sz_Size) {
  return calloc(sz_Size, 1);
}

/**
 * vector_t - Generic vector container with generic element arithmetic and memory operations.
 *
 * Members:
 * - s32_Type: Provided value specifying the type of element.  May be predefined or user provided.
 * - p_StorageBuffer: Address to location of vector data in memory.
 * - sz_BufferSize: Total size of the buffer, calculated as sz_ElementSize * sz_ElementCount.
 * - sz_ElementSize: Size (in bytes) of each element in the vector.
 * - sz_ElementCount: Number of elements in vector.
 * - pfn_ElementAdd/pfn_ElementSubtract/pfn_ElementMultiply/pfn_ElementDivide: User-provided function callbacks for arithmetic operations (may be done easily with provided macros).
 * - pfn_Allocate/pfn_Free: User-provided memory allocation callbacks that may be either automatically filled by vctcreate or manually-set to use custom memory allocation tools.
 */
typedef struct __vector_t {
	TYPE s32_Type;

	void* p_StorageBuffer;
	size_t sz_BufferSize;
	size_t sz_ElementSize;
	size_t sz_ElementCount;

	void (*pfn_ElementAdd)(void*, const void*, const void*);
	void (*pfn_ElementSubtract)(void*, const void*, const void*);
	void (*pfn_ElementMultiply)(void*, const void*, const void*);
	void (*pfn_ElementDivide)(void*, const void*, const void*);

	void* (*pfn_Allocate)(size_t);
	void  (*pfn_Free)(void*);
} vector_t;


/**
 * vctcreate - Allocates and returns a new vector_t instance with the given length and element size.
 *
 * Parameters:
 * - pv_Vector: Pointer to the location in memory where the new vector type is stored.
 * - pfn_AllocateMemory: Callback function to memory allocation.
 * - pfn_FreeMemory: Callback function to memory deallocation.
 *
 * Returns:
 * - On success: 0
 * - On failure: -1
 */
int vctcreate(vector_t* pv_Vector, void* (*pfn_AllocateMemory)(size_t), void (*pfn_FreeMemory)(void*));

/**
 * MAKE_VECTOR - Macro designed to streamline the process of creating vector_t types.
 *
 * Parameters:
 *  - name: Name of vector_t variable.
 *  - type: Type used by each element in the vector (uint8_t, float, int32_t, etc.).
 *  - size: Number of elements in the vector.
 *  - pfn_add/pfn_sub/pfn_mul/pfn_div: Callback functions that represent the operations between the elements' types.
 */
#define MAKE_VECTOR(name, type, size, type_enum, pfn_add, pfn_sub, pfn_mul, pfn_div) \
vector_t name              	= {}; \
name.s32_Type              	= type_enum; \
name.sz_ElementSize        	= sizeof(type); \
name.sz_ElementCount       	= size; \
name.pfn_ElementAdd        	= pfn_add; \
name.pfn_ElementSubtract   	= pfn_sub; \
name.pfn_ElementMultiply   	= pfn_mul; \
name.pfn_ElementDivide     	= pfn_div; \
\
vctcreate(&name, NULL, NULL);

/**
 * MAKE_VECTOR_FAST - The easiest and fastest way to create vector_t types directly from vector.h.
 *
 * Parameters:
 *  - name: Name of the vector_t variable.
 *  - type: Type used by each element in the vector (uint8_t, float, int32_t, etc.).
 *  - size: Number of elements in the vector.
 *  - abbr: Abbreviated notation for the provided type (FP32 for float, S8 for int8_t, etc.)
 *
 * Recommendations:
 *  - Use the associated USE_ARITHMETIC_OP_SET_* macro to avoid the need to type all operation implementations.
 */
#define MAKE_VECTOR_FAST(name, type, size, abbr) MAKE_VECTOR(name, type, size, TYPE_##abbr, Add##abbr, Subtract##abbr, Multiply##abbr, Divide##abbr)


/**
 * vctmemchk - Check if internal memory is valid for use in operations.
 *
 * Parameters:
 *  - cpv_Vector: Constant pointer to a vector_t variable.
 *
 * Examines:
 *  - cpv_Vector->p_StorageBuffer
 *  - cpv_Vector->sz_ElementSize
 * 	- cpv_Vector->sz_ElementCount
 *	- cpv_Vector->sz_BufferSize
 *	- cpv_Vector->s32_Type
 * 	- cpv_Vector->pfn_Allocate
 *	- cpv_Vector->pfn_Free
 *
 * Returns:
 *  - Success: 0
 *  - Failure: -1
 */
int vctmemchk(const vector_t* cpv_Vector);


/**
 * vctread - Copy data from one element in a vector to a provided destination.
 *
 * Parameters:
 *  - p_Destination: Address of the read value is copied to.
 *  - cpv_Vector: Constant pointer to a vector_t type that vctread copies data from.
 *  - csz_Idx: Index of element in cpv_Vector that is copied to p_Destination.
 */
void vctread(void* p_Destination, const vector_t* cpv_Vector, const size_t csz_Idx);

/**
 * vctwrite - Copy data from a provided address to an element in the vector.
 *
 * Parameters:
 *  - pv_Vector: Pointer to the vector_t that is written to by vctwrite.
 *  - csz_Idx: Index of element that is modified by vctwrite.
 *  - p_Data: Address in memory that vctwrite copies from.
 */
void vctwrite(vector_t* pv_Vector, const size_t csz_Idx, void* p_Data);


/**
 * vctcmp - Check if two vectors are suitable for an operation
 *
 * Parameters:
 *  - cpv_A: Constant pointer to a vector_t variable.
 *  - cpv_B: Constant pointer to a second vector_t variable.
 *
 * Examines:
 *  - s32_Type
 * 	- sz_ElementCount
 *	- pfn_Add
 *	- pfn_Subtract
 * 	- pfn_Multiply
 *	- pfn_Divide
 *
 * Returns:
 *  - Suitable: 0
 *  - Unsuitable: 1
 *  - Failure: -1
 */
int vctcmp(const vector_t* cpv_A, const vector_t* cpv_B);


/**
 * vctadd/vctsub/vctelemul/vctelediv - Performs element-wise arithmetic operations.
 *
 * Requirements:
 * - Parameters must be non-NULL
 * - Vectors must be the same length and element size.
 * - Respective arithmetic callback (e.g., pfn_ElementAdd for vctadd) must be set.
 * - Vectors must use the same arithmetic callbacks
 */

#define ELEMENTWISE_OP_DEC(fn_Name) \
 void fn_Name(vector_t* pv_Result, const vector_t* cpv_A, const vector_t* cpv_B);

ELEMENTWISE_OP_DEC(vctadd)
ELEMENTWISE_OP_DEC(vctsub)
ELEMENTWISE_OP_DEC(vctelemul)
ELEMENTWISE_OP_DEC(vctelediv)

// TODO: Add cross-product support here (only works for two and three-dimensional vectors)
// void vctcross(...) {...}


/**
 * vctdot - Dot products for vector_t types.
 *
 * Parameters:
 *  - p_Product: Pointer to a space in memory that is read as the same type as each of the elements in the provided vector_t types.
 *  - cpv_A: Constant pointer to a vector_t type.
 *  - cpv_B: Constant pointer to a second vector_t type.
 */
void vctdot(void* p_Product, const vector_t* cpv_A, const vector_t* cpv_B);


/**
 * vctscale/vctscaleinv - Element-wise scaling for vector_t types.
 *
 * Parameters:
 *  - pv_Scaled: Pointer to a vector_t that will hold the scaled copy of the original vector.
 *  - cpv_Vector: Constant pointer to a vector_t type that represents the vector before being scaled.
 *  - cp_Scalar: Constant pointer to a space in memory that represents the scalar value of the operation.
 *
 * vctscaleinv will scale the vector by the inverse of the interpreted value of cp_Scalar.
 */
#define SCALE_OP_DEC(fn_Name) \
void fn_Name(void* pv_Scaled, const vector_t* cpv_Vector, const void* cp_Scalar);

SCALE_OP_DEC(vctscale)
SCALE_OP_DEC(vctscaleinv)


/**
 * vctmagsq - Get the squared magnitude of a vector_t type.
 *
 * Parameters:
 *  - p_Magnitude: Pointer to a space in memory where the calculated value is stored.
 *  - cpv_Vector: Constant pointer to a vector_t type whose magnitude is to be calculated.
 */
void vctmagsq(void* p_Magnitude, const vector_t* cpv_Vector);

/**
 * vctnorm - Get the normalized vector from a provided vector_t type.
 *
 * Parameters:
 *  - pv_Normalized: Pointer to a vector_t type that will represent the normalized vector.
 *  - cpv_Vector: Constant vector pointer whose normalized representation is to be calculated.
 *  - pfn_SquareRoot: Square root function callback which will be used when taking the square root of the magnitude.
 *
 * pfn_SquareRoot:
 *  - Must return calculated value through first argument.
 *  - Must read provided value through second argument.
 */
void vctnorm(vector_t* pv_Normalized, const vector_t* cpv_Vector, void (*pfn_SquareRoot)(void*, const void*));


/**
 * vctdstry - Deallocates a vector and its internal buffer using its designated pfn_Free member.
 *
 * Parameters:
 * - pv_vector: pointer to vector_t to be freed
 */
void vctdstry(vector_t* pv_Vector);

#endif // VECTOR_H_
