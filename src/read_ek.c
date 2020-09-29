#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <tss2/tss2_esys.h>
#include <tss2/tss2_mu.h>
#include <tss2/tss2_tcti.h>
#include <tss2/tss2_tcti_device.h>

#define TSSWG_INTEROP 1
#define TSS_SAPI_FIRST_FAMILY 2
#define TSS_SAPI_FIRST_LEVEL 1
#define TSS_SAPI_FIRST_VERSION 108
#define LOG_ERROR printf


TSS2_TCTI_CONTEXT *generate_tcti()
{

    TSS2_TCTI_CONTEXT *tcti_ctx = NULL;

    size_t size;
	/*to get the size of TSS2_TCTI_DEVICE_CONTEXT */
    TSS2_RC rc = Tss2_Tcti_Device_Init(NULL, &size, NULL);
    if (rc != TPM2_RC_SUCCESS) {
        LOG_ERROR("try to get the size of TSS2_TCTI_DEVICE_CONTEXT failed\n");
        goto err;
    }

    tcti_ctx = (TSS2_TCTI_CONTEXT*) calloc(1, size);
    if (tcti_ctx == NULL) {
        LOG_ERROR("oom");
        goto err;
    }

    rc = Tss2_Tcti_Device_Init(tcti_ctx, &size, NULL);
    if (rc != TPM2_RC_SUCCESS) {
        LOG_ERROR("Tss2_Tcti_Device_Init failed\n");
        goto err;
    }

    return tcti_ctx;

err:
    free(tcti_ctx);
    return NULL;
}


int read_ek(char *buf, int len)
{	
    TSS2_RC rc;
    
    TSS2_TCTI_CONTEXT *tcti_context = NULL;    
    ESYS_CONTEXT *esys_context = NULL;
    TSS2_ABI_VERSION abiVersion = { TSSWG_INTEROP, TSS_SAPI_FIRST_FAMILY, TSS_SAPI_FIRST_LEVEL, TSS_SAPI_FIRST_VERSION };
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
	
    TPM2B_DIGEST auth_policy = {
	    .size = 32,
	    .buffer = {
		    0x83, 0x71, 0x97, 0x67, 0x44, 0x84, 0xB3, 0xF8, 0x1A, 0x90, 0xCC,
		    0x8D, 0x46, 0xA5, 0xD7, 0x24, 0xFD, 0x52, 0xD7, 0x6E, 0x06, 0x52,
		    0x0B, 0x64, 0xF2, 0xA1, 0xDA, 0x1B, 0x33, 0x14, 0x69, 0xAA
	    }
    };
	
    TPM2B_PUBLIC inPublic = {
	    .publicArea = {
		    .nameAlg = TPM2_ALG_SHA256,
		    .type = TPM2_ALG_RSA,
		    .objectAttributes = TPMA_OBJECT_RESTRICTED | TPMA_OBJECT_ADMINWITHPOLICY 
					| TPMA_OBJECT_DECRYPT | TPMA_OBJECT_FIXEDTPM 
					| TPMA_OBJECT_FIXEDPARENT | TPMA_OBJECT_SENSITIVEDATAORIGIN,			
		    .parameters = {
			    .rsaDetail = {
				    .exponent = 0,
				    .symmetric = {
					    .algorithm = TPM2_ALG_AES,
					    .keyBits = { .aes = 128 },
					    .mode = { .aes = TPM2_ALG_CFB },
				    },
				    .scheme = { .scheme = TPM2_ALG_NULL },
				    .keyBits = 2048
			    },
		    },
		    .unique = { .rsa = { .size = 256 } }
	    },
    };
    inPublic.publicArea.authPolicy = auth_policy;

    TPM2B_AUTH authValue = {
	    .size = 0,
	    .buffer = {}
    };

    TPM2B_DATA outsideInfo = {
	    .size = 0,
	    .buffer = {},
    };

    TPML_PCR_SELECTION creationPCR = {
	    .count = 0,
    };

    ESYS_TR session_handle = ESYS_TR_NONE;

    TPMT_SYM_DEF symmetric = {.algorithm = TPM2_ALG_NULL};
	
	ESYS_TR signHandle;	


	if (len > TPM2_MAX_RSA_KEY_BYTES) 
		return -1;
	
	tcti_context = generate_tcti();
	if (tcti_context == NULL) {
        LOG_ERROR("generate_tcti FAILED!\n");
        return 1;
    }
	
    rc = Esys_Initialize(&esys_context, tcti_context, &abiVersion);
    if (rc != TSS2_RC_SUCCESS) {
        LOG_ERROR("Esys_Initialize FAILED! Response Code : 0x%x", rc);
        return 1;
    }
    rc = Esys_Startup(esys_context, TPM2_SU_CLEAR);
    if (rc != TSS2_RC_SUCCESS && rc != TPM2_RC_INITIALIZE) {
        LOG_ERROR("Esys_Startup FAILED! Response Code : 0x%x", rc);
        return 1;
    }

    rc = Esys_SetTimeout(esys_context, TSS2_TCTI_TIMEOUT_BLOCK);
    if (rc != TSS2_RC_SUCCESS) {
        LOG_ERROR("Esys_SetTimeout FAILED! Response Code : 0x%x", rc);
        return 1;
    }
        

    rc = Esys_TR_SetAuth(esys_context, ESYS_TR_RH_OWNER, &authValue);
    if (rc != TSS2_RC_SUCCESS) {
	LOG_ERROR("Esys_TR_SetAuth FAILED! Response Code : 0x%x", rc);
	return 1;
    }
    /*clear*/
    rc = Esys_Clear(esys_context,
		    ESYS_TR_RH_PLATFORM, 
		    ESYS_TR_PASSWORD, 
		    ESYS_TR_NONE, 
		    ESYS_TR_NONE);

    if (rc != TSS2_RC_SUCCESS) {
	    LOG_ERROR("Esys_Clear FAILED! Response Code : 0x%x", rc);
	    return 1;
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
	    LOG_ERROR("Esys_StartAuthSession FAILED! Response Code : 0x%x", rc);
	    return 1;
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
	    LOG_ERROR("Esys_CreatePrimary FAILED! Response Code : 0x%x", rc);
	    return 1;
    }

    rc = Esys_FlushContext(esys_context, session_handle);
    if (rc != TSS2_RC_SUCCESS) {
	    LOG_ERROR("Esys_FlushContext FAILED! Response Code : 0x%x", rc);
	    return 1;
    }

    /*get ek pub from outPublic*/
    memcpy(buf, outPublic->publicArea.unique.rsa.buffer, len);

    Esys_Finalize(&esys_context);
	
    //TODO: tcti finalize;

    return 0;

}

