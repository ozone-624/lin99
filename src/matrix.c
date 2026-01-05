#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lin99/vector.h"
#include "lin99/matrix.h"

int mtxcreate(matrix_t* pm_Matrix, void* (*pfn_AllocateMemory)(size_t), void (*pfn_FreeMemory)(void*)) {
	if(pm_Matrix == NULL) {
		printf("NULL REFERENCE PASSED!\n");
		return -1;
  	}
 
  	if (pm_Matrix->sz_Width == 0 ||
	pm_Matrix->sz_Height    == 0 ||
  	pm_Matrix->sz_ElementSize  == 0) {
  		return -1;  
	}

	// We can't use aligned_alloc here since this project is intended to be strictly C99, and aligned_alloc wasn't introduced until C11
	pm_Matrix->pfn_Allocate = ((pfn_AllocateMemory != NULL) || (pm_Matrix->pfn_Allocate != NULL)) ? pfn_AllocateMemory : zalloc;
	pm_Matrix->pfn_Free = ((pfn_FreeMemory != NULL) || (pm_Matrix->pfn_Free != NULL)) ? pfn_FreeMemory : free;

	pm_Matrix->sz_ElementCount = pm_Matrix->sz_Width * pm_Matrix->sz_Height;
	if (pm_Matrix->sz_ElementCount < pm_Matrix->sz_Width) {
		printf("MULTIPLICATION OVERFLOW WHEN CALCULATING ELEMENT COUNT\n");
		return -1;
	}

	pm_Matrix->sz_BufferSize = pm_Matrix->sz_ElementSize * pm_Matrix->sz_ElementCount;
	pm_Matrix->p_StorageBuffer = pm_Matrix->pfn_Allocate(pm_Matrix->sz_BufferSize);

	if (!CHECK_ALLOCATION(pm_Matrix->p_StorageBuffer) || pm_Matrix->sz_BufferSize < pm_Matrix->sz_ElementCount) {
		pm_Matrix->pfn_Free(pm_Matrix->p_StorageBuffer);
		printf("MULTIPLICATION OVERFLOW WHEN CALCULATING BUFFER SIZE\n");
		return -1;
	}

	return 0;
}

int mtxmemchk(const matrix_t* cpm_Matrix) {
  	if (cpm_Matrix->p_StorageBuffer != NULL         &&
  	cpm_Matrix->sz_ElementSize      != 0        	&&
	cpm_Matrix->sz_Width    	!= 0        	&&
	cpm_Matrix->sz_Height	    	!= 0            &&
  	cpm_Matrix->sz_ElementCount 	!= 0         	&&
  	cpm_Matrix->sz_BufferSize   	!= 0         	&&
  	cpm_Matrix->s32_Type       	!= TYPE_NULL    &&
  	cpm_Matrix->pfn_Allocate   	!= NULL      	&&
  	cpm_Matrix->pfn_Free       	!= NULL) {
    		return 0;
  	}

  	return -1;
}

// Read/Write operations will go here
void mtxreadraw(void* p_Destination, const matrix_t* cpm_Matrix, const size_t csz_RawIdx) {
	if (cpm_Matrix == NULL) {
		printf("NULL REFERENCE PASSED!\n");
		return;
	}

	if (csz_RawIdx < cpm_Matrix->sz_ElementCount) {
		memcpy(p_Destination, (uint8_t*)cpm_Matrix->p_StorageBuffer + csz_RawIdx * cpm_Matrix->sz_ElementSize, cpm_Matrix->sz_ElementSize);
		return;
	}

	printf("RAW INDEX EXCEEDED ELEMENT COUNT!\n");
	return;
}

void mtxread(void* p_Destination, const matrix_t* cpm_Matrix, const size_t csz_RowIdx, const size_t csz_ColIdx) {
	if (cpm_Matrix == NULL) {
		printf("NULL REFERENCE PASSED!\n");
		return;
	}

	// Use our mtxreadraw function so we don't have to maintain two functions that fulfill the same job.
	mtxreadraw(p_Destination, cpm_Matrix, csz_RowIdx + cpm_Matrix->sz_Height * csz_ColIdx);
	return;
}

// Add write commands here.

void mtxdstry(matrix_t* pm_Matrix) {
	if (pm_Matrix == NULL) {
		printf("NULL REFERENCED PASSED!\n");
		return;
	}

	if (pm_Matrix->p_StorageBuffer != NULL) {\
		pm_Matrix->pfn_Free(pm_Matrix->p_StorageBuffer);
		pm_Matrix->sz_BufferSize = 0;
	}
	return;
}
