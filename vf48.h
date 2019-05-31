/*********************************************************************

  Name:         vf48.h
  Created by:   Pierre-Andre Amaudruz / Jean-Pierre Martin

  Contents:     48 ch Flash ADC / 20-64 Msps from J.-P. Martin
  $Id: vf48.h 4463 2009-04-04 01:31:28Z olchanski $

*********************************************************************/
 
#ifndef  VF48_INCLUDE_H
#define  VF48_INCLUDE_H

#include <stdio.h>
#include <string.h>

#include "mvmestd.h"

/* Definitions */
#define VF48_MAX_CHANNELS 48

#define VF48_IDXMAX                (4096)
#define VF48_GRP_OFFSET            (12)
#define VF48_PARMA_BIT_RD          (0x80)
#define VF48_ALL_CHANNELS_ACTIVE   (0xFF)

/* VF48 driver error codes */

#define  VF48_SUCCESS              (1)
#define  VF48_ERROR                (-1)
#define  VF48_ERR_PARM             (-1)
#define  VF48_ERR_NODATA           (503)
#define  VF48_ERR_HW               (603)

/* VF48 Collector VME Registers */

#define  VF48_CSR_REG_RW           (0x0000) // see VF48_CSR_xxx below
#define  VF48_TEST_REG_RW          (0x0020)
#define  VF48_FIRMWARE_R           (0x0030)
#define  VF48_SOFTWARE_RESET_W     (0x0040) // FPGA reboot (requires hardware mod)
#define  VF48_PARAM_DATA_RW        (0x0050) // see frontend parameters
#define  VF48_PARAM_ID_W           (0x0060) // and mode bits below
#define  VF48_SOFT_TRIG_W          (0x0070)
#define  VF48_LVDS_RESET_W         (0x0080)
#define  VF48_GRP_REG_RW           (0x0090)
#define  VF48_NFRAME_R             (0x00A0)
#define  VF48_GLOBAL_RESET_W       (0x00B0)
#define  VF48_TrigConfigA_RW       (0x00C0) // see trigger config bits below
#define  VF48_TrigConfigB_RW       (0x00D0)
#define  VF48_FlashProgrammer      (0x00E0) // see srunner_vme
#define  VF48_LinkMonA             (0x0100)
#define  VF48_LinkMonB             (0x0110)
#define  VF48_FifoStat10           (0x0120)
#define  VF48_FifoStat32           (0x0130)
#define  VF48_FifoStat54           (0x0140)
#define  VF48_FifoData0            (0x0150)
#define  VF48_FifoData1            (0x0160)
#define  VF48_FifoData2            (0x0170)
#define  VF48_FifoData3            (0x0180)
#define  VF48_FifoData4            (0x0190)
#define  VF48_FifoData5            (0x01A0)
#define  VF48_DATA_FIFO_R          (0x1000)

/* VF48 CSR bit assignment */

#define  VF48_CSR_START_ACQ        (0x00000001) // CSR[0] "RUN", 0:stop, 1:start
#define  VF48_CSR_PARM_ID_RDY      (0x00000002) // CSR[1] "Param_IDReady"
#define  VF48_CSR_PARM_DATA_RDY    (0x00000004) // CSR[2] "Param_DataReady"
#define  VF48_CSR_FIFO_EMPTY       (0x00000008) // CSR[3]
#define  VF48_CSR_FIFO_INCOMPLETE  (0x00000010) // CSR[4]
#define  VF48_CSR_CRC_ERROR        (0x00000020) // CSR[5] "OR" of the 6 VF48_CSR_CRC_ERROR_MASK bits
// removed because of confusion #define  VF48_CSR_xxx              (0x00000040) // CSR[6] not used, fixed at zero
#define  VF48_CSR_EXT_TRIGGER      (0x00000080) // CSR[7] "Select_ExtTrigger"
#define  VF48_CSR_CRC_ERROR_MASK   (0x00003F00) // CSR[13..8] CRC Error on 6 LVDS links
// removed because of confusion #define  VF48_CSR_TIGC_CONF0       (0x00004000) // CSR[14] "TIGC_Config[0]"
// removed because of confusion #define  VF48_CSR_TIGC_CONF1       (0x00008000) // CSR[15] "TIGC_Config[1]"
#define  VF48_CSR_BUSY_OUT         (0x00010000) // CSR[16] "Select_BusyOut", route BUSY to NIM_OUT
#define  VF48_CSR_FIFO_DISABLE     (0x00020000) // CSR[17] "xxx", disable the VME FIFO (use PIO to read FE FIFOs directly)
#define  VF48_CSR_EVB_DISABLE      (0x00040000) // CSR[18] "xxx", disable the EVB - all data from frontends immediately goes into the bit bucket

/* VF48 TrigConfigA and B bit assignements */

/* TriggerConfig[63..0] = VF48_TrigConfigB_RW[31..0],VF48_TrigConfigA_RW[31..0]
 *
 * 0 - AuxTrigDisable
 * 1 - local trigger source select: 0: Aux_In or any FE; 1: mult trigger
 * 2 - trigger select: 0: local trigger; 1: TIGC trigger
 * 3 - Nim_Aux_Out select: 0: local trigger; 1: Sys_Clk; if VF48_CSR_BUSY_OUT is set: BUSY
 * 39..4 - GroupThreshold[5..0][5..0]
 * 57..40 - FEgroup[2..0][5..0]
 * 63..58 - GroupCoincidence[5..0]
 *
*/


/* VF48 Frontend Parameters */

#define  VF48_PEDESTAL              (1)    // unused
#define  VF48_HIT_THRESHOLD         (2)    // Threshold[15..0]
#define  VF48_CLIP_DELAY            (3)    // unused
#define  VF48_PRE_TRIGGER           (4)    // PreTrig[15..0]
#define  VF48_SEGMENT_SIZE          (5)    // Segment_Size[15..0]
#define  VF48_K_COEF                (6)    // K[15..0]
#define  VF48_L_COEF                (7)    // L[15..0]
#define  VF48_M_COEF                (8)    // M[15..0]
#define  VF48_ACTIVE_CH_MASK        (9)    // ChannelMask[15..0]
#define  VF48_MBIT1                 (10)   // ModeBits[15..0]
#define  VF48_MBIT2                 (11)   // ModeBits[31..16]
#define  VF48_LATENCY               (12)   // Latency[15..0]
#define  VF48_FIRMWARE_ID           (13)   // FirmwareID[31..16]
#define  VF48_ATTENUATOR            (14)   // unused
#define  VF48_TRIG_THRESHOLD        (15)   // TriggerThreshold[15..0]
#define  VF48_FIRMWARE_ID_LOW       (16)   // FirmwareID[15..0]
#define  VF48_INTEG_DELAY           (17)   // IntegDelay[15..0]

/* VF48 Frontend ModeBits assignement */

#define  VF48_MBITS_0                  (0x00000001) // EnableSimulation
#define  VF48_MBITS_RAW_DISABLE        (0x00000002) // SuppressRawdata
#define  VF48_MBITS_2                  (0x00000004) // unused?
#define  VF48_MBITS_INVERSE_SIGNAL     (0x00000008) // PolarityPlus
#define  VF48_MBITS_4                  (0x00000010) // unused?
#define  VF48_MBITS_5                  (0x00000020) // FakeOneData
#define  VF48_MBITS_6                  (0x00000040) // EnablePienuSum
#define  VF48_MBITS_7                  (0x00000080) // Enable_HitDetSum
#define  VF48_MBITS_SUM_ENABLE         (0x0000FF00) // SumEnable[7..0]
#define  VF48_MBITS_CH_SUPPRESS_ENABLE (0x00010000) // EnableChanSuppress
#define  VF48_MBITS_17                 (0x00020000) // HitDet_MinMax
#define  VF48_MBITS_DIVIDER            (0xFF000000) // Divider[7..0]

#define  VF48_RAW_DISABLE           VF48_MBITS_RAW_DISABLE
#define  VF48_CH_SUPPRESS_ENABLE    VF48_MBITS_CH_SUPPRESS_ENABLE
#define  VF48_INVERSE_SIGNAL        VF48_MBITS_INVERSE_SIGNAL

/*
Parameter frame
15 ...    ...
CCCCDDDD RVPP PPPP
C: Destination Card/Port N/A
R: Read bit (0:Write, 1:Read)
D: Destination channels (0..5)
   bit 11..8
          0: channel 1..8
          1: channel 9..16
          2: channel 17..24
          3: channel 25..32
          4: channel 33..40
          5: channel 41..48
          6: channel N/C
      7..15: channel N/C
V: Version 0 for now (0:D16, 1:D32(extended))
P: Parameter ID
  Default values for the different PIDs
ID#   Def Value
1     0x0000 Pedestal
2     0x000A Hit Det Threshold
3     0x0028 Clip Delay
4     0x0020 Pre-Trigger
5     0x0100 Segment size
6     0x0190 K-coeff
7     0x0200 L-coeff
8     0x1000 M-coeff
9     0x0005 Feature Delay A
10    0x0000 Mbit1
          0x1: Data simulation
          0x2: Supress Raw Data
          0x8: Inverse Signal
11    0x0001 Feature Delay B
12    0x0005 Latency
13    0x0100 Firmware ID
14    0x0190 Attenuator
15    0x0050 Trigger threshold
16    0x00FF Active Channel Mask
17    0x0000 Mbit2
          0x1: Enable Channel Suppress
       0xff00: sampling divisor         // Temporary
*/

/* Header definition */
#define  VF48_HEADER                (0x80000000)
#define  VF48_TIME_STAMP            (0xA0000000)
#define  VF48_CHANNEL               (0xC0000000)
#define  VF48_DATA                  (0x00000000)
#define  VF48_CFD_FEATURE           (0x40000000)
#define  VF48_Q_FEATURE             (0x50000000)
#define  VF48_TRAILER               (0xE0000000)

//#define  VF48_OUT_OF_SYNC           (0x88000000)
//#define  VF48_TIMEOUT               (0x10000000)

/* VF48 Reset functions */

int  vf48_ReconfigureFPGA(MVME_INTERFACE *myvme, DWORD base);
int  vf48_Reset(MVME_INTERFACE *myvme, DWORD base);
int  vf48_ResetCollector(MVME_INTERFACE *myvme, DWORD base);
int  vf48_ResetFrontends(MVME_INTERFACE *myvme, DWORD base, int groupMask);

/* VF48 Status functions */

int  vf48_isPresent(MVME_INTERFACE *mvme, DWORD base);
int  vf48_Status(MVME_INTERFACE *mvme, DWORD base);

/* VF48 Register and Parameter access */

int  vf48_RegisterRead(MVME_INTERFACE *myvme, DWORD base, int reg);
int  vf48_RegisterWrite(MVME_INTERFACE *myvme, DWORD base, int reg, int value);
int  vf48_ParameterRead(MVME_INTERFACE *myvme, DWORD base, int grp, int param);
int  vf48_ParameterWrite(MVME_INTERFACE *myvme, DWORD base, int grp, int param, int value);

int  vf48_CsrRead(MVME_INTERFACE *myvme, DWORD base);
int  vf48_CsrWrite(MVME_INTERFACE *myvme, DWORD base, int value);

/* VF48 Trigger setup */

int  vf48_ExtTrgSet(MVME_INTERFACE *myvme, DWORD base);
int  vf48_ExtTrgClr(MVME_INTERFACE *myvme, DWORD base);
int  vf48_Trigger(MVME_INTERFACE *mvme, DWORD base);

/* VF48 misc functions */

int  vf48_Setup(MVME_INTERFACE *mvme, DWORD base, int mode);

int  vf48_EventReadPio(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nwords);
int  vf48_EventReadSync(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nwords);

int  vf48_EventRead(MVME_INTERFACE *myvme, DWORD base, DWORD *event, int *elements);
int  vf48_EventRead64(MVME_INTERFACE *myvme, DWORD base, DWORD *event, int *elements);
int  vf48_GroupRead(MVME_INTERFACE *myvme, DWORD base, DWORD *event, int grp, int *elements);
int  vf48_DataRead(MVME_INTERFACE *myvme, DWORD base, DWORD *event, int *elements);
int  vf48_AcqStart(MVME_INTERFACE *myvme, DWORD base);
int  vf48_AcqStop(MVME_INTERFACE *myvme, DWORD base);
int  vf48_NFrameRead(MVME_INTERFACE *myvme, DWORD base);
int  vf48_GrpRead(MVME_INTERFACE *myvme, DWORD base);
int  vf48_GrpEnable(MVME_INTERFACE *myvme, DWORD base, int grpbit);
int  vf48_GrpOperationMode(MVME_INTERFACE *myvme, DWORD base, int grp, int opmode);

int  vf48_ParameterCheck(MVME_INTERFACE *myvme, DWORD base, int what);
int  vf48_SegmentSizeSet(MVME_INTERFACE *mvme, DWORD base, DWORD size);
int  vf48_SegmentSizeRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_TrgThresholdSet(MVME_INTERFACE *mvme, DWORD base, int grp, DWORD size);
int  vf48_TrgThresholdRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_HitThresholdSet(MVME_INTERFACE *mvme, DWORD base, int grp, DWORD size);
int  vf48_HitThresholdRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_ActiveChMaskSet(MVME_INTERFACE *mvme, DWORD base, int grp, DWORD size);
int  vf48_ActiveChMaskRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_RawDataSuppSet(MVME_INTERFACE *mvme, DWORD base, int grp, DWORD size);
int  vf48_RawDataSuppRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_ChSuppSet(MVME_INTERFACE *mvme, DWORD base, int grp, DWORD size);
int  vf48_ChSuppRead(MVME_INTERFACE *mvme, DWORD base, int grp);
int  vf48_DivisorWrite(MVME_INTERFACE *mvme, DWORD base, DWORD size);
int  vf48_DivisorRead(MVME_INTERFACE *mvme, DWORD base, int grp);

/* TIGC functions */

#define  TIGC_TRIGREG_SUMENABLE    (0x0100)
#define  TIGC_TRIGREG_THRESHOLD    (0x0108)
#define  TIGC_TRIGREG_DISABLEM     (0x0110)
#define  TIGC_TRIGREG_SUMGAINS     (0x0118)
#define  TIGC_GENRESET             (0x0080)

int  vf48_SetTigcSumEnable(MVME_INTERFACE *, DWORD, int);
int  vf48_SetTigcTrigThreshold(MVME_INTERFACE *, DWORD, int);
int  vf48_SetTigcDisableMask(MVME_INTERFACE *, DWORD, int);
int  vf48_SetTigcSumGains(MVME_INTERFACE *, DWORD, int);
int  vf48_TigcGenReset(MVME_INTERFACE *, DWORD);

#endif
