/* securechk.h
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 *
 */

#ifndef SECURECHK_H
#define SECURECHK_H

#ifdef __cplusplus
extern "C" {
#endif

enum DO_VALIDATION {
	WITHOUT_VALIDATION=0,				// The validity of the key is not confirmed
	WITH_VALIDATION						// The validity of the key is confirmed
};

enum CHECK_RESULT {
	SecureOK=0,							// The program authentication success
	DoValidationErr=0x00000001,			// Error of authentication specification of key
	VRL5NG=0x00000002,					// Error of VRL5 table
	MMFROMHalted=0xFFFFFFF1,			// No ROM clock supply
	CC5Halted=0xFFFFFFF2,				// No CC5.2 clock supply
	MERAMHalted=0xFFFFFFF4,				// No MERAM clock supply
	ICBHalted=0xFFFFFFF8,				// No ICB clock supply
	VRL5Invalid=0xF0000000,				// Error of validity of VRL5
	PublicKeyInvalid=0xF0000006,		// Error of validity of authentication key
	PublicKeyChkErr=0xF0000007,			// Internal error of validity of authentication key
	SecureNGMask=0x10000000				// Error of authentication of record N(+N:N record)
};

// Function prototype
unsigned long SecureBoot(void *pVRL, unsigned long *N_ptr, unsigned long DoValidation);
void SecureInit(void);

#ifdef __cplusplus
}
#endif
#endif
