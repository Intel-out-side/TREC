#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project Ready_to_measure_can.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	6
#define CUST_BYTE_NUM_IN	6
#define TOT_BYTE_NUM_ROUND_OUT	8
#define TOT_BYTE_NUM_ROUND_IN	8


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
		int8_t      fx_m2s;
		int8_t      fy_m2s;
		int8_t      fz_m2s;
		int8_t      tx_m2s;
		int8_t      ty_m2s;
		int8_t      tz_m2s;
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		int8_t      fx_s2m;
		int8_t      fy_s2m;
		int8_t      fz_s2m;
		int8_t      tx_s2m;
		int8_t      ty_s2m;
		int8_t      tz_s2m;
	}Cust;
} PROCBUFFER_IN;

#endif