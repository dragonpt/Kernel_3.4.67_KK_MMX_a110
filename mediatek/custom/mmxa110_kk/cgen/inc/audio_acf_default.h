/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

    /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
    #define BES_LOUDNESS_HSF_COEFF \
    0x7DA1709,0xF054253E,0x7D1C95A,0x7D56C298, \
    0x7D6C2CA,0xF05B867F,0x7CDBD5F,0x7D19C2D1, \
    0x7C751CD,0xF07DBC74,0x7BAFE48,0x7BFEC3DA, \
    0x7B4AD39,0xF0A6FF57,0x7A46980,0x7AA5C515, \
    0x7AE1CEE,0xF0B5836B,0x79C79B1,0x7A2BC583, \
    0x78FC91E,0xF0F882BF,0x777E4BF,0x77F0C777, \
    0x76B705A,0xF148A5E6,0x74C3E63,0x7537C9BF, \
    0x75EB726,0xF164A91C,0x73D0346,0x7440CA88, \
    0x7247308,0xF1E4AE1B,0x6F7957D,0x6FC0CE05   

    /* Compensation Filter BPF coeffs: default all pass filter      */ 
    #define BES_LOUDNESS_BPF_COEFF \
    0x3FD481A8,0x3EFF7E57,0xC12C0000, \ 
    0x3FD081DA,0x3EE97E25,0xC1460000, \ 
    0x3FBE82D7,0x3E817D28,0xC1C00000, \ 
    0x3FA98440,0x3E037BBF,0xC2520000, \ 
    0x3FA184CE,0x3DD77B31,0xC2860000, \ 
    0x3F7E87BD,0x3D0C7842,0xC3740000, \ 
\
    0x3FD481C0,0x3EFF7E3F,0xC12C0000, \ 
    0x3FD081F5,0x3EE97E0A,0xC1460000, \ 
    0x3FBE830B,0x3E817CF4,0xC1C00000, \ 
    0x3FA9849C,0x3E037B63,0xC2520000, \ 
    0x3FA1853A,0x3DD77AC5,0xC2860000, \ 
    0x3F7E8889,0x3D0C7776,0xC3740000, \ 
\
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
\
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0, \ 
    0x0,0x0,0x0       
    
    #define BES_LOUDNESS_DRC_FORGET_TABLE \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000   

    #define BES_LOUDNESS_WS_GAIN_MAX  0
           
    #define BES_LOUDNESS_WS_GAIN_MIN  0
           
    #define BES_LOUDNESS_FILTER_FIRST  0
           
    #define BES_LOUDNESS_GAIN_MAP_IN \
    -45, -41, -40, -18,  0
   
    #define BES_LOUDNESS_GAIN_MAP_OUT \            
    12, 12, 12, 12, 0               

#endif
