/* security.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 *
 */
#include "security.h"

RC Secure_Check(void *certificate, unsigned long *image)
{
	uint32_t output_data;
	sec_msg_handle_t h_req;
	sec_msg_handle_t h_resp;

	sec_msg_t * q = NULL;
	sec_msg_t * p = NULL;
	
	sec_serv_status_t stat;
	
	if(certificate == NULL || image == NULL)
		return SEC_ERR_INPUT;
	
	/* Memory init */
	mem_malloc_init();
	
	 /* Calculate size of input message */
    uint16_t in_msg_data_size = sec_msg_param_size(sizeof(void *)) +
                            sec_msg_param_size(sizeof(unsigned long *));
					
	/* Calculate size of output message structure */
    uint16_t out_msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
									 sec_msg_param_size(sizeof(uint32_t));
	
	/* Allocate memory for input message */
	q = sec_msg_alloc(&h_req,
					in_msg_data_size,
                    SEC_MSG_OBJECT_ID_NONE,
                    0,
                    SEC_MSG_BYTE_ORDER_LE);

	/* Allocate memory for output message */
	p = sec_msg_alloc(&h_resp,
                    out_msg_data_size,
                    SEC_MSG_OBJECT_ID_NONE,
                    0,
                    SEC_MSG_BYTE_ORDER_LE);
	
	/* Write CERTIFICATE_COMMON info to input message */
	sec_msg_param_write32(&h_req,
                        (uint32_t)certificate,
                        SEC_MSG_PARAM_ID_NONE);		
	
	/* Write image address to input message */
	sec_msg_param_write32(&h_req,
                        (uint32_t) image,
                        SEC_MSG_PARAM_ID_NONE);
						
	/* give initial values to output message */
	sec_msg_param_write32(&h_resp, 0, SEC_MSG_PARAM_ID_NONE);	/* service status */
	sec_msg_param_write32(&h_resp, 0, SEC_MSG_PARAM_ID_NONE);	/* data identifier */
#if 1	
	/* Register protected data certificate */
	stat = sec_dispatcher(SEC_SERV_PROT_DATA_REGISTER,
                                           0, 
                                           0,	/* spare */
                                           p,   /* pointer to response (output) message */
                                           q);  /* pointer to input message */
	
	/* rewind the out message handle since we used it to add parameters */
	sec_msg_rewind(&h_resp);
	
	sec_msg_param_read32(&h_resp, &output_data);	
#endif	
		
	sec_msg_free(q);
	sec_msg_free(p);
	
	if(output_data == SEC_SERV_STATUS_OK)
		return SEC_OK;
	
	return output_data;
}

/** Secure_WDT_clear - Clear secure watch dog timer 
*/
RC Secure_WDT_clear(void)
{
	uint32_t interval, output_data;
	sec_msg_handle_t h_req;			/* Request message */
	sec_msg_handle_t h_resp;		/* Response message */

	sec_msg_t * q = NULL;					/* Input message */
	sec_msg_t * p = NULL;				/* Output message */

	uint32_t stat;

	/* Memory init */
	mem_malloc_init();

	 /* Calculate size of input message */
	unsigned short in_msg_data_size = sec_msg_param_size(sizeof(uint32_t));

	/* Calculate size of output message structure */
	unsigned short out_msg_data_size = sec_msg_param_size(sizeof(uint32_t)) +
									 sec_msg_param_size(sizeof(uint32_t));

	/* Allocate memory for input message */
	q = sec_msg_alloc(&h_req,
					in_msg_data_size,
					SEC_MSG_OBJECT_ID_NONE,
					0,
					SEC_MSG_BYTE_ORDER_LE);

	/* Allocate memory for output message */
	p = sec_msg_alloc(&h_resp, 
					  out_msg_data_size,
					  SEC_MSG_OBJECT_ID_NONE,
					  0,
					  SEC_MSG_BYTE_ORDER_LE);
	
	/* Write clear_sec_watchdog_value info to input message */
	sec_msg_param_write32(&h_req, (uint32_t)0, SEC_MSG_PARAM_ID_NONE);

	/* give initial values to output message */
	sec_msg_param_write32(&h_resp, (uint32_t)0, SEC_MSG_PARAM_ID_NONE);	/* service status */
	sec_msg_param_write32(&h_resp, (uint32_t)0, SEC_MSG_PARAM_ID_NONE);	/* data identifier */

	/* Register protected data certificate */
	stat = sec_dispatcher(SEC_SERV_INTEGRITY_CHECK,
									  0,
									  0,	/* spare */
									  p,	/* pointer to response (output) message */
									  q);	/* pointer to input message */
	
	/* rewind the out message handle since we used it to add parameters */
	sec_msg_rewind(&h_resp);
	sec_msg_param_read32(&h_resp, &output_data); /* service status */
	sec_msg_param_read32(&h_resp, &interval); /* interval time */

	sec_msg_free(q);
	sec_msg_free(p);

	if(output_data == SEC_SERV_STATUS_OK)
	{
		return interval;
	}
	
	return -output_data;
}

