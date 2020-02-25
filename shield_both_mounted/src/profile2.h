#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project Ready_to_measure_can.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	12
#define CUST_BYTE_NUM_IN	12
#define TOT_BYTE_NUM_ROUND_OUT	12
#define TOT_BYTE_NUM_ROUND_IN	12


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
		uint8_t     fx_m2s;
		uint8_t     fy_m2s;
		uint8_t     fz_m2s;
		uint8_t     tx_m2s;
		uint8_t     ty_m2s;
		uint8_t     tz_m2s;
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		uint8_t     fx_s2m;
		uint8_t     fy_s2m;
		uint8_t     fz_s2m;
		uint8_t     tx_s2m;
		uint8_t     ty_s2m;
		uint8_t     tz_s2m;
	}Cust;
} PROCBUFFER_IN;

#endif
