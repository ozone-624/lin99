//TODO: Global zalloc use instead of calling zalloc every operation
//TODO: Add overflow protection on pfn_*-based operations
//TODO: Validate pfn_*'s in vctcreate (good luck)
//TODO: Ensure vector_t supports non-abelian algebraic structures

/*
 * matrix.h
 *
 * A pure C99 linear algebra header for type-generic matrices.
 *
 * Features:
 * - Type-generic matrix representation using void pointers
 * - Element-wise operations (addition, subtraction, multiplication, division)
 * - Determinants, inverses, and transposes
 * - Custom memory management and arithmetic logic via function pointers
 *
 * This design allows support for multiple numerical types (e.g., float, int, double)
 * and allocation strategies by plugging in appropriate arithmetic and memory function sets.
 *
 * Hungarian Notation Key:
 * - pv_  : pointer to vector_t
 * - cpv_ : const pointer to vector_t
 * - pm_  : pointer to matrix_t
 * - cpm_ : const pointer to matrix_t
 * - psz_ : pointer to size_t
 * - csz_ : const size_t
 * - pfn_ : function pointer
 * - pu8_ : pointer to unsigned 8-bit integers
 *
 * The provided matrix.h and matrix.c files are explicitly designed to work with the implementations used in these files.
 *
 */


#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Inherit the enumerated values already defined in vector.h
#include "vector.h"

// Create a generalized operation definition that allows the user to specify a custom name, type, operator, and allocation method.
/**
 * Writing custom operation functions.
 *
 * Imagine your matrix_t is using a type which does not have easy-to-use operations provided by the C99 standard.
 * You would need a way of defining your own operations which could be used by matrix_t.
 *
 * On top of what is already provided by matrix.h, the ability to create and utilize custom operation definitions
 * is also possible thanks to the use of callback functions.  This is to serve as a tutorial for creating a simple
 * callback function.
 *
 * matrix.h utilizes callback functions in a way that is comparable to the preexisting structure of functions like
 * mtxadd, mtxwrite, etc.  To write your own callback function, it must have exactly three parameters:
 *  - Parameter 1 (void*) - This is your result from the operation.  For addition, this would be the address to the location of the sum.
 *  - Parameter 2 (const void*) - This is your left operand location.  It is to be treated solely as readonly.
 *  - Parameter 3 (const void*) - This is your right operand location. It, too, is to be treated as readonly.
 *
 * YOU DO NOT HAVE TO WORRY ABOUT MEMORY MANAGEMENT IN THESE CALLBACKS, matrix.h handles that for you.
 *
 * To write operators, simply modify the dereferenced and interpreted value of the result (*(your_type*)your_result)
 * Once the operation is finished, call "return;."  Remember, this function does not return anything, all data storage is
 * handled through the parameters.
 *
 * For an example of how arithmetic callbacks should be written, please see the GENERAL_OP_DEFINITION macro
 */

/**
 * matrix_t - Generic matrix-style container with generic element arithmetic and memory operations.
 *
 * Members:
 * - s32_Type: Provided value specifying the type of element.  May be predefined or user provided.
 * - p_StorageBuffer: Address to location of matrix data in memory.
 * - sz_BufferSize: Total size of the buffer, calculated as sz_ElementSize * sz_ElementCount.
 * - sz_ElementSize: Size (in bytes) of each element in the matrix.
 * - sz_Width: Width of matrix.
 * - sz_Height: Height of matrix.
 * - sz_ElementCount: Total number of elements in matrix, calculated as sz_Height * sz_Width;
 * - pfn_ElementAdd/pfn_ElementSubtract/pfn_ElementMultiply/pfn_ElementDivide: User-provided function callbacks for arithmetic operations (may be done easily with provided macros).
 * - pfn_Allocate/pfn_Free: User-provided memory allocation callbacks that may be either automatically filled by vctcreate or manually-set to use custom memory allocation tools.
 */
typedef struct __matrix_t {
	TYPE s32_Type;

	void* p_StorageBuffer;
	size_t sz_BufferSize;
	size_t sz_ElementSize;
	size_t sz_Width;
	size_t sz_Height;
	size_t sz_ElementCount;

	void (*pfn_ElementAdd)(void*, const void*, const void*);
	void (*pfn_ElementSubtract)(void*, const void*, const void*);
	void (*pfn_ElementMultiply)(void*, const void*, const void*);
	void (*pfn_ElementDivide)(void*, const void*, const void*);

	void* (*pfn_Allocate)(size_t);
	void  (*pfn_Free)(void*);
} matrix_t;


/**
 * mtxcreate - Allocates and stores a new matrix_t instance with the given length and element size.
 *
 * Parameters:
 * - pm_Matrix: Pointer to the location in memory where the new matrix type is stored.
 * - pfn_AllocateMemory: Callback function to memory allocation.
 * - pfn_FreeMemory: Callback function to memory deallocation.
 *
 * Returns:
 * - On success: 0
 * - On failure: -1
 */
int mtxcreate(matrix_t* pm_Matrix, void* (*pfn_AllocateMemory)(size_t), void (*pfn_FreeMemory)(void*));

/**
 * MAKE_MATRIX - Macro designed to streamline the process of creating matrix_t types.
 *
 * Parameters:
 *  - name: Name of matrix_t variable.
 *  - type: Type used by each element in the matrix (uint8_t, float, int32_t, etc.).
 *  - width: Width of matrix.
 *  - height: Height of matrix.
 *  - pfn_add/pfn_sub/pfn_mul/pfn_div: Callback functions that represent the operations between the elements' types.
 */
#define MAKE_MATRIX(name, type, width, height, type_enum, pfn_add, pfn_sub, pfn_mul, pfn_div) \
matrix_t name              	= {}; \
name.s32_Type              	= type_enum; \
name.sz_ElementSize        	= sizeof(type); \
name.sz_Width               = width; \
name.sz_Height              = height; \
name.sz_ElementCount       	= width * height; \
name.pfn_ElementAdd        	= pfn_add; \
name.pfn_ElementSubtract   	= pfn_sub; \
name.pfn_ElementMultiply   	= pfn_mul; \
name.pfn_ElementDivide     	= pfn_div; \
\
mtxcreate(&name, NULL, NULL);

/**
 * MAKE_MATRIX_FAST - The easiest and fastest way to create matrix_t types directly from matrix.h.
 *
 * Parameters:
 *  - name: Name of the matrix_t variable.
 *  - type: Type used by each element in the matrix (uint8_t, float, int32_t, etc.).
 *  - width: Width of matrix.
 *  - height: Height of matrix.
 *  - abbr: Abbreviated notation for the provided type (FP32 for float, S8 for int8_t, etc.)
 *
 * Recommendations:
 *  - Use the associated USE_ARITHMETIC_OP_SET_* macro to avoid the need to type all operation implementations.
 */
#define MAKE_MATRIX_FAST(name, type, width, height, abbr) MAKE_MATRIX(name, type, width, height, TYPE_##abbr, Add##abbr, Subtract##abbr, Multiply##abbr, Divide##abbr)


/**
 * mtxmemchk - Check if internal memory is valid for use in operations.
 *
 * Parameters:
 *  - cpm_Matrix: Constant pointer to a matrix_t variable.
 *
 * Examines:
 *  - cpm_Matrix->p_StorageBuffer
 *  - cpm_Matrix->sz_ElementSize
 *  - cpm_Matrix->sz_Width
 *  - cpm_Matrix->sz_Height
 * 	- cpm_Matrix->sz_ElementCount
 *	- cpm_Matrix->sz_BufferSize
 *	- cpm_Matrix->s32_Type
 * 	- cpm_Matrix->pfn_Allocate
 *	- cpm_Matrix->pfn_Free
 *
 * Returns:
 *  - Success: 0
 *  - Failure: -1
 */
int mtxmemchk(const matrix_t* cpm_Matrix);


/**
 * mtxreadraw - Copy data from one element in a vector to a destination labeled by a raw index.
 *
 * Parameters:
 *  - p_Destination: Address of the read value is copied to.
 *  - cpm_Matrix: Constant pointer to a matrix_t type that vctread copies data from.
 *  - csz_RawIdx: Direct value of local address in memory, allows for easy iteration through all elements of matrix.
 */
void mtxreadraw(void* p_Destination, const matrix_t* cpm_Matrix, const size_t csz_RawIdx);

/**
 * mtxread - Copy data from one element in a vector to a provided destination.
 *
 * Parameters:
 *  - p_Destination: Address of the read value is copied to.
 *  - cpm_Matrix: Constant pointer to a matrix_t type that vctread copies data from.
 *  - csz_RowIdx: Index of row in cpm_Matrix that is copied to p_Destination.
 *  - csz_ColIdx: Index of column in cpm_Matrix that is copied to p_Destination.
 */
void mtxread(void* p_Destination, const matrix_t* cpm_Matrix, const size_t csz_RowIdx, const size_t csz_ColIdx);

// Add mtxwriteraw here

/**
 * vctwrite - Copy data from a provided address to an element in the vector.
 *
 * Parameters:
 *  - pm_Matrix: Pointer to the matrix_t that is written to by mtxwrite.
 *  - csz_RowIdx: Index of element row that is modified by mtxwrite.
 *  - csz_ColIdx: Index of element column that is modified by mtxwrite.
 *  - p_Data: Address in memory that mtxwrite copies from.
 */
void mtxwrite(matrix_t* pm_Matrix, const size_t csz_RowIdx, const size_t csz_ColIdx, void* p_Data);

// OPERATORS GO HERE //

/**
 * mtxdstry - Deallocates a matrix and its internal buffer using its designated pfn_Free member.
 *
 * Parameters:
 * - pm_Matrix: pointer to matrix_t to be freed
 */
void mtxdstry(matrix_t* pm_Matrix);

#endif // MATRIX_H_


