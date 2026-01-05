#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lin99/vector.h"

int vctcreate(vector_t* pv_Vector, void* (*pfn_AllocateMemory)(size_t), void (*pfn_FreeMemory)(void*)) {
  if(pv_Vector == NULL) {
	printf("NULL REFERENCE PASSED!\n");
	return -1;
  }
 
  if (pv_Vector->sz_ElementCount == 0 ||
  	pv_Vector->sz_ElementSize  == 0) {
  	return -1;  
  }
 
  // We can't use aligned_alloc here since this project is intended to be strictly C99, and aligned_alloc wasn't introduced until C11
	pv_Vector->pfn_Allocate = ((pfn_AllocateMemory != NULL) || (pv_Vector->pfn_Allocate != NULL)) ? pfn_AllocateMemory : zalloc;
	pv_Vector->pfn_Free = ((pfn_FreeMemory != NULL) || (pv_Vector->pfn_Free != NULL)) ? pfn_FreeMemory : free;
 
	pv_Vector->sz_BufferSize = pv_Vector->sz_ElementSize * pv_Vector->sz_ElementCount;
	pv_Vector->p_StorageBuffer = pv_Vector->pfn_Allocate(pv_Vector->sz_BufferSize);

  if (!CHECK_ALLOCATION(pv_Vector->p_StorageBuffer) || pv_Vector->sz_BufferSize < pv_Vector->sz_ElementCount) {
	pv_Vector->pfn_Free(pv_Vector->p_StorageBuffer);
	printf("MULTIPLICATION OVERFLOW WHEN CALCULATING BUFFER SIZE\n");
  	return -1;
  }

	return 0;
}

int vctmemchk(const vector_t* cpv_Vector) {
  if (cpv_Vector->p_StorageBuffer != NULL  	&&
  	cpv_Vector->sz_ElementSize  != 0       	&&
  	cpv_Vector->sz_ElementCount != 0       	&&
  	cpv_Vector->sz_BufferSize   != 0       	&&
  	cpv_Vector->s32_Type    	!= TYPE_NULL  &&
  	cpv_Vector->pfn_Allocate	!= NULL      	&&
  	cpv_Vector->pfn_Free    	!= NULL) {
    	return 0;
  }
  return -1;
}

void vctread(void* p_Destination, const vector_t* cpv_Vector, const size_t csz_Idx) {
  if(cpv_Vector == NULL) {
	printf("NULL REFERENCE PASSED!\n");
	return;
  }
 
  if (csz_Idx < cpv_Vector->sz_ElementCount) {
	memcpy(p_Destination, (uint8_t*)cpv_Vector->p_StorageBuffer + (csz_Idx * cpv_Vector->sz_ElementSize), cpv_Vector->sz_ElementSize);
	return;
  }
  printf("INDEX EXCEEDED VECTOR SIZE!\n");
	return;
}

void vctwrite(vector_t* pv_Vector, const size_t csz_Idx, void* p_Data) {
  if(pv_Vector == NULL) {
	printf("NULL REFERENCE PASSED!\n");
	return;
  }
 
	if (csz_Idx >= pv_Vector->sz_ElementCount) {
  	printf("INDEX EXCEEDED VECTOR SIZE!\n");
    	return;
	}

	memcpy((uint8_t*)pv_Vector->p_StorageBuffer + (csz_Idx * pv_Vector->sz_ElementSize), p_Data, pv_Vector->sz_ElementSize);
	return;
}

int vctcmp(const vector_t* cpv_A, const vector_t* cpv_B) {
  if(cpv_A == NULL || cpv_B == NULL) {
	printf("NULL REFERENCE PASSED!\n");
	return -1;
  }
 
	if ((cpv_A->s32_Type != cpv_B->s32_Type)                 	||
	(cpv_A->sz_ElementCount != cpv_B->sz_ElementCount)       	||
	(cpv_A->pfn_ElementAdd != cpv_B->pfn_ElementAdd)         	||
	(cpv_A->pfn_ElementSubtract != cpv_B->pfn_ElementSubtract)   ||
	(cpv_A->pfn_ElementMultiply != cpv_B->pfn_ElementMultiply)   ||
	(cpv_A->pfn_ElementDivide != cpv_B->pfn_ElementDivide)) {
    	return 1;
	}
	return 0;
}

#define ELEMENTWISE_OP_DEF(fn_Name, pfn_Name) \
 void fn_Name(vector_t* pv_Result, const vector_t* cpv_A, const vector_t* cpv_B) { \
  if(cpv_A == NULL || cpv_B == NULL || pv_Result == NULL) { \
	printf("NULL REFERENCE PASSED!\n"); \
	return; \
  } \
  \
	if ((vctcmp(cpv_A, cpv_B) != 0) || \
	cpv_A->pfn_Name == NULL     	|| \
	vctmemchk(cpv_A) != 0       	|| \
	vctmemchk(cpv_B) != 0       	|| \
	vctmemchk(pv_Result) != 0) { \
    	printf("VECTORS NOT COMPATIBLE!\n"); \
    	return; \
	} \
  \
	uint8_t* pu8_A = (uint8_t*)cpv_A->pfn_Allocate(cpv_A->sz_ElementSize); \
	if (!CHECK_ALLOCATION(pu8_A)) { \
  	printf("MEMORY NOT FOUND!\n"); \
  	return; \
	} \
	uint8_t* pu8_B = (uint8_t*)cpv_B->pfn_Allocate(cpv_B->sz_ElementSize); \
	if (!CHECK_ALLOCATION(pu8_B)) { \
  	cpv_A->pfn_Free(pu8_A); \
  	printf("MEMORY NOT FOUND!\n"); \
  	return; \
	} \
	uint8_t* pu8_Result = (uint8_t*)pv_Result->pfn_Allocate(pv_Result->sz_ElementSize); \
	if (!CHECK_ALLOCATION(pu8_Result)) { \
  	cpv_B->pfn_Free(pu8_B); \
  	cpv_A->pfn_Free(pu8_A); \
  	printf("MEMORY NOT FOUND!\n"); \
  	return; \
	} \
	\
	for (size_t sz_Idx = 0; sz_Idx < cpv_A->sz_ElementCount; ++sz_Idx) { \
	\
    	vctread(pu8_A, cpv_A, sz_Idx); \
    	vctread(pu8_B, cpv_B, sz_Idx); \
    	cpv_A->pfn_Name(pu8_Result, pu8_A, pu8_B); \
    	\
    	vctwrite(pv_Result, sz_Idx, pu8_Result); \
	} \
  \
  cpv_A->pfn_Free(pu8_A); \
  cpv_B->pfn_Free(pu8_B); \
  pv_Result->pfn_Free(pu8_Result); \
  \
	return; \
}

ELEMENTWISE_OP_DEF(vctadd, pfn_ElementAdd)
ELEMENTWISE_OP_DEF(vctsub, pfn_ElementSubtract)
ELEMENTWISE_OP_DEF(vctelemul, pfn_ElementMultiply)
ELEMENTWISE_OP_DEF(vctelediv, pfn_ElementDivide)

void vctdot(void* p_Product, const vector_t* cpv_A, const vector_t* cpv_B) {
    if (cpv_A == NULL || cpv_B == NULL || p_Product == NULL) {
        printf("NULL REFERENCE PASSED!\n");
        return;
    }

    // Add more security checks here
    if ((vctcmp(cpv_A, cpv_B) != 0) ||
        cpv_A->pfn_ElementAdd == NULL ||
        cpv_A->pfn_ElementMultiply == NULL ||
        vctmemchk(cpv_A) != 0 ||
        vctmemchk(cpv_B) != 0) {
        printf("VECTORS NOT COMPATIBLE!\n");
        return;
    }

    uint8_t *pu8_A = (uint8_t *) cpv_A->pfn_Allocate(cpv_A->sz_ElementSize);
    if (!CHECK_ALLOCATION(pu8_A)) {
        printf("MEMORY NOT FOUND!\n");
        return;
    }
    uint8_t *pu8_B = (uint8_t *) cpv_B->pfn_Allocate(cpv_B->sz_ElementSize);
    if (!CHECK_ALLOCATION(pu8_B)) {
        cpv_A->pfn_Free(pu8_A);
        printf("MEMORY NOT FOUND!\n");
        return;
    }
    uint8_t *pu8_Product = (uint8_t *) cpv_A->pfn_Allocate(cpv_A->sz_ElementSize);
    if (!CHECK_ALLOCATION(pu8_Product)) {
        cpv_B->pfn_Free(pu8_B);
        cpv_A->pfn_Free(pu8_A);
        printf("MEMORY NOT FOUND!\n");
        return;
    }

    // Zero-initialize $p_Product so we're not adding to a non-zero value
    memset(p_Product, 0, cpv_A->sz_ElementSize);

    for (size_t sz_Idx = 0; sz_Idx < cpv_A->sz_ElementCount; ++sz_Idx) {

        vctread(pu8_A, cpv_A, sz_Idx);
        vctread(pu8_B, cpv_B, sz_Idx);
        memset(pu8_Product, 0, cpv_A->sz_ElementSize);  //Make sure $pu8_Product is 0 before we start
        cpv_A->pfn_ElementMultiply(pu8_Product, pu8_A, pu8_B);

        cpv_A->pfn_ElementAdd(p_Product, pu8_Product, p_Product);
    }

    cpv_A->pfn_Free(pu8_Product);
    cpv_B->pfn_Free(pu8_B);
    cpv_A->pfn_Free(pu8_A);

    return;
}

#define SCALE_OP_DEF(fn_Name, pfn_Name) \
void fn_Name(void* pv_Scaled, const vector_t* cpv_Vector, const void* cp_Scalar) { \
  if(cpv_Vector == NULL || cp_Scalar == NULL || pv_Scaled == NULL) { \
    printf("NULL REFERENCE PASSED!\n"); \
    return; \
  } \
  \
    if (cpv_Vector->pfn_Name  == NULL || \
        vctmemchk(cpv_Vector) != 0) { \
        printf("VECTORS NOT COMPATIBLE!\n"); \
        return; \
    } \
  \
  uint8_t* pu8_Element = (uint8_t*)cpv_Vector->pfn_Allocate(cpv_Vector->sz_ElementSize); \
  if (!CHECK_ALLOCATION(pu8_Element)) { \
    printf("MEMORY NOT FOUND!\n"); \
    return; \
  }\
  uint8_t* pu8_ScaledElement = (uint8_t*)cpv_Vector->pfn_Allocate(cpv_Vector->sz_ElementSize); \
  if (!CHECK_ALLOCATION(pu8_ScaledElement)) { \
    cpv_Vector->pfn_Free(pu8_Element); \
    printf("MEMORY NOT FOUND!\n"); \
    return; \
  }\
  \
    for (size_t sz_Idx = 0; sz_Idx < cpv_Vector->sz_ElementCount; ++sz_Idx) { \
        \
        vctread(pu8_Element, cpv_Vector, sz_Idx); \
        cpv_Vector->pfn_Name(pu8_ScaledElement, pu8_Element, cp_Scalar); \
        \
        vctwrite(pv_Scaled, sz_Idx, pu8_ScaledElement); \
    } \
  \
  cpv_Vector->pfn_Free(pu8_Element); \
  cpv_Vector->pfn_Free(pu8_ScaledElement); \
  \
    return; \
}

SCALE_OP_DEF(vctscale, pfn_ElementMultiply)
SCALE_OP_DEF(vctscaleinv, pfn_ElementDivide)

// Since there is no abstract way to take square roots AFAIK, we will return the squared magnitude
void vctmagsq(void* p_Magnitude, const vector_t* cpv_Vector) {
    if (cpv_Vector == NULL || p_Magnitude == NULL) {
        printf("NULL REFERENCE PASSED!\n");
        return;
    }

    if (cpv_Vector->pfn_ElementAdd == NULL ||
        cpv_Vector->pfn_ElementMultiply == NULL ||
        vctmemchk(cpv_Vector) != 0) {
        printf("VECTOR NOT COMPATIBLE!\n");
        return;
    }

    vctdot(p_Magnitude, cpv_Vector, cpv_Vector);

    return;
}

void vctnorm(vector_t* pv_Normalized, const vector_t* cpv_Vector, void (*pfn_SquareRoot)(void*, const void*)) {
    if (pv_Normalized == NULL || cpv_Vector == NULL) {
        printf("NULL REFERENCE PASSED!\n");
        return;
    }

    if (cpv_Vector->pfn_ElementDivide == NULL ||
        pfn_SquareRoot == NULL ||
        vctmemchk(cpv_Vector) != 0) {

        printf("VECTOR/SQUARE ROOT CALLBACK NOT COMPATIBLE!\n");
        return;
    }

    uint8_t *pu8_Magnitude = (uint8_t *) cpv_Vector->pfn_Allocate(cpv_Vector->sz_ElementSize);
    if (!CHECK_ALLOCATION(pu8_Magnitude)) {
        printf("MEMORY NOT FOUND!\n");
        return;
    }

    vctdot(pu8_Magnitude, cpv_Vector, cpv_Vector);

    if (memcmp(pu8_Magnitude, NULL, cpv_Vector->sz_ElementSize) != 0) {
        pfn_SquareRoot(pu8_Magnitude, pu8_Magnitude);  // $pu8_Magnitude becomes the square root of itself
    } else {
        cpv_Vector->pfn_Free(pu8_Magnitude);
        printf("DIVIDE BY ZERO ERROR!\n");
        return;
    }

    vctscaleinv(pv_Normalized, cpv_Vector, pu8_Magnitude);

    cpv_Vector->pfn_Free(pu8_Magnitude);

    return;
}

void vctdstry(vector_t* pv_Vector) {
    if (pv_Vector == NULL) {
        printf("NULL REFERENCE PASSED!\n");
        return;
    }

    if (pv_Vector->p_StorageBuffer != NULL) {
        pv_Vector->pfn_Free(pv_Vector->p_StorageBuffer);
        pv_Vector->sz_BufferSize = 0;
    }
    return;
}
