#ifndef _CALDATA_H_
#define _CALDATA_H_

/*
    2412  0x5F0  0x5F6  0x5F7  0x5F8  0x5F9  0x5FA  0x5FB 
    2442  0x5FE  0x604  0x605  0x606  0x607  0x608  0x609 
    2472  0x60C  0x612  0x613  0x614  0x615  0x616  0x617 
*/

/*
 *See  AR6003_CAL_DATA_PER_FREQ_OLPC_EXPANDED
 * host/tools/systemtools/devlib_ar6003/ar6003/mEepStruct6003.h
 */
typedef struct _calData {
    s8    olpcGainDelta;                     //----- for all rate. 
    u8   pad[3];
    s16   olpcGainDelta_t10_G_0;             //----- =  olpcGainDelta *5. 
    u8   desiredScaleCck_t10;               //----- for 11b rate. 
    u8   desiredScale6M_t10;                //----- for 6M,9M,12M,18M. 
    u8   desiredScale36M_t10;               //----- for 24M, 36M. 
    u8   desiredScale54M_t10;               //----- for 48M, 54M. 
    u8   desiredScaleMCS0HT20_t10;          //----- for MCS0, MCS1. 
    u8   desiredScaleMCS7HT20_t10;          //----- for MCS2 ~MCS7. 
}__attribute__ ((packed)) calData;

typedef struct _offsetTable {
    u16 channel;
    u16 offset_start;
}__attribute__ ((packed)) offsetTable;

#define EEPROM_SIZE 			2048

void update2GCalData(u8 *buf, calData *ncal);
void dump2GCalData(u8 *buf);

#endif /*_CALDATA_H_*/
