/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Copyright: 2021 KUNBUS GmbH
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tss2/tss2_esys.h>
#include <tss2/tss2_mu.h>
#include <tss2/tss2_tcti.h>
#include <tss2/tss2_tcti_device.h>

#include "debug.h"

#define TSSWG_INTEROP 1
#define TSS_SAPI_FIRST_FAMILY 2
#define TSS_SAPI_FIRST_LEVEL 1
#define TSS_SAPI_FIRST_VERSION 108

static TSS2_RC generate_tcti(const char * const dev_path, TSS2_TCTI_CONTEXT **tcti_ctx)
{
	size_t size;
	TSS2_RC rc;

	/*to get the size of TSS2_TCTI_DEVICE_CONTEXT */
	rc = Tss2_Tcti_Device_Init(NULL, &size, NULL);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("get the size of TSS2_TCTI_DEVICE_CONTEXT failed\n");
		goto err;
	}

	*tcti_ctx = (TSS2_TCTI_CONTEXT*) calloc(1, size);
	if (*tcti_ctx == NULL) {
		err_print("get memory for tcti context failed\n");
		rc = TSS2_BASE_RC_GENERAL_FAILURE;
		goto err;
	}

	rc = Tss2_Tcti_Device_Init(*tcti_ctx, &size, dev_path);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Tss2_Tcti_Device_Init failed\n");
		goto err_tcti;
	}
	return TSS2_RC_SUCCESS;

err_tcti:
	free(*tcti_ctx);
err:
	return rc;
}

static void free_tcti(TSS2_TCTI_CONTEXT *ctx)
{
	if (ctx) {
		Tss2_Tcti_Finalize(ctx);
		free(ctx);
	}
	return;
}

static unsigned int read_ek(const char *const dev_path, uint8_t * const serial)
{
	const unsigned int len = 8;
	TSS2_TCTI_CONTEXT *tcti_context = NULL;
	ESYS_CONTEXT *esys_context = NULL;
	TSS2_ABI_VERSION abiVersion = {TSSWG_INTEROP,
					TSS_SAPI_FIRST_FAMILY,
					TSS_SAPI_FIRST_LEVEL,
					TSS_SAPI_FIRST_VERSION};
	TPM2B_PUBLIC *outPublic;
	TPM2B_CREATION_DATA *creationData;
	TPM2B_DIGEST *creationHash;
	TPMT_TK_CREATION *creationTicket;
	TPM2B_SENSITIVE_CREATE inSensitivePrimary = {
		.size = 0,
		.sensitive = {
			.userAuth = {
				.size = 0,
				.buffer = {0},
			 },
			.data = {
				.size = 0,
				.buffer = {0},
			 },
		},
	};
	TPM2B_PUBLIC inPublic = {
		.publicArea = {
			.nameAlg = TPM2_ALG_SHA256,
			.type = TPM2_ALG_RSA,
			.objectAttributes = TPMA_OBJECT_RESTRICTED
					| TPMA_OBJECT_ADMINWITHPOLICY
					| TPMA_OBJECT_DECRYPT
					| TPMA_OBJECT_FIXEDTPM
					| TPMA_OBJECT_FIXEDPARENT
					| TPMA_OBJECT_SENSITIVEDATAORIGIN,
			.parameters = {
				.rsaDetail = {
					.exponent = 0,
					.symmetric = {
						.algorithm = TPM2_ALG_AES,
						.keyBits = {.aes = 128},
						.mode = {.aes = TPM2_ALG_CFB},
					},
					.scheme = {.scheme = TPM2_ALG_NULL},
					.keyBits = 2048
				},
			},
			.authPolicy = {
				.size = 32,
				.buffer = {
				0x83, 0x71, 0x97, 0x67, 0x44, 0x84, 0xB3, 0xF8,
				0x1A, 0x90, 0xCC, 0x8D, 0x46, 0xA5, 0xD7, 0x24,
				0xFD, 0x52, 0xD7, 0x6E, 0x06, 0x52, 0x0B, 0x64,
				0xF2, 0xA1, 0xDA, 0x1B, 0x33, 0x14, 0x69, 0xAA
				}
			},
			.unique = {.rsa = {.size = 256}}
		},
	};
	TPM2B_AUTH authValue = { .size = 0, .buffer = {0} };
	TPM2B_DATA outsideInfo = { .size = 0, .buffer = {0} };
	TPML_PCR_SELECTION creationPCR = { .count = 0 };
	ESYS_TR session_handle = ESYS_TR_NONE;
	TPMT_SYM_DEF symmetric = {.algorithm = TPM2_ALG_NULL};
	ESYS_TR signHandle;
	TSS2_RC rc;

	if (len > TPM2_MAX_RSA_KEY_BYTES) {
		rc = TSS2_TPM_RC_LAYER | TSS2_BASE_RC_BAD_VALUE;
		goto ret;
	}

	rc = generate_tcti(dev_path, &tcti_context);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("generate_tcti FAILED!\n");
		goto ret;
	}
	rc = Esys_Initialize(&esys_context, tcti_context, &abiVersion);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_Initialize FAILED! Response Code : 0x%x", rc);
		goto ret_tcti;
	}
	rc = Esys_Startup(esys_context, TPM2_SU_CLEAR);
	if (rc != TSS2_RC_SUCCESS && rc != TPM2_RC_INITIALIZE) {
		err_print("Esys_Startup FAILED! Response Code : 0x%x", rc);
		goto ret_esys;
	}
	rc = Esys_SetTimeout(esys_context, TSS2_TCTI_TIMEOUT_BLOCK);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_SetTimeout FAILED! Response Code : 0x%x", rc);
		goto ret_esys;
	}

	rc = Esys_TR_SetAuth(esys_context, ESYS_TR_RH_OWNER, &authValue);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_TR_SetAuth FAILED! Response Code : 0x%x", rc);
		goto ret_esys;
	}
	/*clear*/
	rc = Esys_Clear(esys_context,
			ESYS_TR_RH_PLATFORM,
			ESYS_TR_PASSWORD,
			ESYS_TR_NONE,
			ESYS_TR_NONE);

	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_Clear FAILED! Response Code : 0x%x", rc);
		goto ret_esys;
	}

	rc = Esys_StartAuthSession(esys_context,
			ESYS_TR_NONE,
			ESYS_TR_NONE,
			ESYS_TR_NONE,
			ESYS_TR_NONE,
			ESYS_TR_NONE,
			NULL,
			TPM2_SE_HMAC,
			&symmetric,
			TPM2_ALG_SHA256,
			&session_handle);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_StartAuthSession FAILED! Response : 0x%x", rc);
		goto ret_esys;
	}

	/*generate ek*/
	rc = Esys_CreatePrimary(esys_context,
			ESYS_TR_RH_ENDORSEMENT,
			session_handle,
			ESYS_TR_NONE,
			ESYS_TR_NONE,
			&inSensitivePrimary,
			&inPublic,
			&outsideInfo,
			&creationPCR,
			&signHandle,
			&outPublic,
			&creationData,
			&creationHash,
			&creationTicket);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_CreatePrimary FAILED! Response Code : 0x%x", rc);
		goto ret_flush;
	}

	/*get ek pub from outPublic*/
	memcpy(serial, outPublic->publicArea.unique.rsa.buffer, len);

ret_flush:
	rc = Esys_FlushContext(esys_context, session_handle);
	if (rc != TSS2_RC_SUCCESS) {
		err_print("Esys_FlushContext FAILED! Response Code : 0x%x", rc);
		goto ret_esys;
	}
ret_esys:
	Esys_Finalize(&esys_context);
ret_tcti:
	free_tcti(tcti_context);
ret:
	return rc;
}

int tpm2_serial(const char *const dev_path, uint8_t * const serial)
{
	unsigned int rc;
	rc = read_ek(dev_path, serial);
	return (rc != 0) ? -1 : 0;
}
