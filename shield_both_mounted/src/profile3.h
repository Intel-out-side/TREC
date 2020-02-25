#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project GetThisDone.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	48
#define CUST_BYTE_NUM_IN	48
#define TOT_BYTE_NUM_ROUND_OUT	48
#define TOT_BYTE_NUM_ROUND_IN	48


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
		double      ref_fx;
		double      ref_fy;
		double      ref_fz;
		double      ref_tx;
		double      ref_ty;
		double      ref_tz;
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		double      fx;
		double      fy;
		double      fz;
		double      tx;
		double      ty;
		double      tz;
	}Cust;
} PROCBUFFER_IN;

#endif