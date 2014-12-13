/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
 
 
 
 

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/*******************************************************************************
*   ISP_NVRAM_REGISTER_STRUCT
********************************************************************************/
Idx:{//ISP_NVRAM_REG_INDEX_STRUCT
    Shading     :0,
    OB          :0,
    DM          :0,
    DP          :0, 
    NR1         :0,
    NR2         :0,
    EE          :0,
    Saturation  :0,
    Contrast    :4,
    Hue         :1,
    CCM         :0,
    Gamma       :0
},
Shading:{
    {set:{//00 Preview
        //0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x903F902F,0x40001950,0x00430036,0x20202020,
        //0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x903F902F,0xC201B200,0x00430036,0x20202020,
         0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x903F902F,0xC2019900,0x00430036,0x20202020,
    }},
    {set:{//01 Capture
        //0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x90809060,0x40001950,0x00820063,0x20202020,
        //0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0xF04FF03B,0xC2018000,0x00510042,0x20202020,
        0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0xF051F03C,0x00000000,0x00330033,0x20202020,
    }},
    {set:{//02
        0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
    }}
},
OB:{
    {set:{//00
        //0x8D8D8D8D
        0x91919191
    }}
}, 
DM:{
    {set:{//00 Preview ISO100/ISO200/ISO400
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//01 Preview ISO800
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//02 Preview ISO1600
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//03 Capture ISO100/ISO200
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//04 Capture ISO400
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//05 Capture ISO800
        0x00000100,	0x0A800810,	0x0020083F
    }}, 
    {set:{//06 Capture ISO1600
        0x00000100,	0x0A800810,	0x0020083F
    }}
}, 
DP:{//Auto Defect
    {set:{//00
        0x000006E6, 0x50285050,	0x006003A0,	0x00300050,	0x000009B7
    }},
    {set:{//01
        0x000006E6, 0x50285050,	0x006003A0,	0x00300050,	0x000009B7
    }},
    {set:{//02
        0x000006E6, 0x50285050,	0x006003A0,	0x00300050,	0x000009B7
    }}, 
    {set:{//03 Disable (Do not modify me)
        0x00000000, 0x50285050, 0x006003A0, 0x00300050, 0x000008B7
    }}
},
NR1:{
    {set:{//00 Preview ISO100
        0x000002E1, 0x000011A0, 0x092328C8, 0x0000089C, 0x02030507, 0x080A0B0D, 0x01030506, 
        0x08090A0C, 0x01030506, 0x08090A0C, 0x01030506, 0x08090A0C, 0x0206090D, 0x10101010
    }},
    {set:{//01 Preview ISO200
        0x00000221, 0x000001A0, 0x092328C8, 0x0000089C, 0x02050708, 0x0A0B0C0E, 0x02050708, 
        0x0A0B0C0E, 0x02050708, 0x0A0B0C0E, 0x02050708, 0x0A0B0D0E, 0x0206090D, 0x10101010
    }},
    {set:{//02 Preview ISO400
        0x000002E1, 0x000011A0, 0x092E28C8, 0x0000089C, 0x02080A0C, 0x0E101213, 0x03080B0D, 
        0x0E101213, 0x03080B0D, 0x0E101213, 0x03080A0C, 0x0E0F1113, 0x0206090D, 0x10101010
    }},
    {set:{//03 Preview ISO800
        0x000002E1, 0x000011A0, 0x092A28C8, 0x0000089C, 0x03090C0E, 0x11141619, 0x04090C0D, 
        0x0F101213, 0x04090C0D, 0x0F101213, 0x03080B0E, 0x10121416, 0x0206090D, 0x10101010
    }},
    {set:{//04 Preview ISO1600
        0x000002E1, 0x000011A0, 0x092328C8, 0x0000089C, 0x04090C0E, 0x10121315, 0x04090C0E, 
        0x10121315, 0x04090C0E, 0x10121315, 0x04080B0E, 0x10111314, 0x0206090D, 0x10101010
    }},
    {set:{//05 Capture ISO100 (o)       
        0x000002E1, 0x000011A0, 0x092328C8, 0x0000089C, 0x02030507, 0x080A0B0D, 0x01030506, 
        0x08090A0C, 0x01030506, 0x08090A0C, 0x01030506, 0x08090A0C, 0x0206090D, 0x10101010
    }},
    {set:{//06 Capture ISO200 (o)
        0x00000221, 0x000001A0, 0x092328C8, 0x0000089C, 0x02050708, 0x0A0B0C0E, 0x02050708, 
        0x0A0B0C0E, 0x02050708, 0x0A0B0C0E, 0x02050708, 0x0A0B0D0E, 0x0206090D, 0x10101010
    }}, 
    {set:{//07 Capture ISO400 (o)                      
        0x000002E1, 0x000011A0, 0x092E28C8, 0x0000089C, 0x02080A0C, 0x0E101213, 0x03080B0D, 
        0x0E101213, 0x03080B0D, 0x0E101213, 0x03080A0C, 0x0E0F1113, 0x0206090D, 0x10101010
    }},
    {set:{//08 Capture ISO800 (o)
        0x000002E1, 0x000011A0, 0x092A28C8, 0x0000089C, 0x03090C0E, 0x11141619, 0x04090C0D, 
        0x0F101213, 0x04090C0D, 0x0F101213, 0x03080B0E, 0x10121416, 0x0206090D, 0x10101010
    }},
    {set:{//09 Capture ISO1600 (o)
        0x000002E1, 0x000011A0, 0x092A28C8, 0x0000089C, 0x03090C0E, 0x11141619, 0x04090C0D, 
        0x0F101213, 0x04090C0D, 0x0F101213, 0x03080B0E, 0x10121416, 0x0206090D, 0x10101010
    }},
    {set:{//10 Disable (Do not modify me)
        0x000000C0, 0x000011A0, 0x094428A0, 0x000007AF, 0x03050709, 0x0B0D0F11, 0x03050709, 
        0x0B0D0F11, 0x03050709, 0x0B0D0F11, 0x03050709, 0x0B0D0F11, 0x0406090D, 0x10101010
    }}
},
NR2:{
    {set:{//00 Preview ISO100
        0x00200000,	0x00191414, 0x00D27768, 0x04080C12, 0x060B1119, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x0305080C
    }},                                                                                                                                   
    {set:{//01 Preview ISO200                                                                                                             
        0x00200000,	0x00191414, 0x00D27768, 0x060C121A, 0x08101823, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x04080C11
    }},                                                                                                                                   
    {set:{//02 Preview ISO400                                                                                                             
        0x00200000,	0x00191414, 0x00D27768, 0x0A131C29, 0x1324344C, 0x01307A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x070D131B
    }},                                                                                                                                   
    {set:{//03 Preview ISO800                                                                                                             
        0x00200000, 0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }},                                                                                                                                   
    {set:{//04 Preview ISO1600                                                                                                            
        0x00200000, 0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }},
    {set:{//05 Capture Mode0 ISO100
        0x00200000, 0x00191414, 0x00D27768, 0x04080C12, 0x060B1119, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x0305080C
    }},                                                                                                                                   
    {set:{//06 Capture Mode0 ISO200
        0x00200000,	0x00191414, 0x00D27768, 0x060C121A, 0x08101823, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x04080C11
    }},                                                                                                                                   
    {set:{//07 Capture Mode0 ISO400
        0x00200000,	0x00191414, 0x00D27768, 0x0A131C29, 0x1324344C, 0x01307A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x070D131B
    }},                                                                                                                                   
    {set:{//08 Capture Mode0 ISO800
        0x00200000,	0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }},                                                                                                                                   
    {set:{//09 Capture Mode0 ISO1600                                                                                                      
        0x00200000, 0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }}, 
    {set:{//10 Capture Mode1 ISO100 (o)
        0x00210000, 0x00191414, 0x00D27768, 0x04080C12, 0x060B1119, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x0305080C
    }},
    {set:{//11 Capture Mode1 ISO200 (o)
        0x00210000, 0x00191414, 0x00D27768, 0x060C121A, 0x08101823, 0x021072CA, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x04080C11
    }},
    {set:{//12 Capture Mode1 ISO400 (o)
        0x00210000, 0x00191414, 0x00D27768, 0x0A131C29, 0x1324344C, 0x01307A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x070D131B
    }},
    {set:{//13 Capture Mode1 ISO800 (o)
        0x00210000, 0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }},
    {set:{//14 Capture Mode1 ISO1600
        0x00210000, 0x00191414, 0x00D27768, 0x0C16202F, 0x16293B56, 0x01707A8E, 0x08080808, 0x10101010, 0x01AF5B48, 0x0000056B, 0x080F151F
    }},
    {set:{//15 Disable (Do not modify me)
        0x00000000, 0x0001FF01, 0x00523264, 0x04090B0F, 0x050A0B10, 0x02107294, 0x08101820, 0x10080604, 0x01AF5B43, 0x0000056B, 0x0306070A
    }}
},
EE:{//EE
    {set:{//00 Preview ISO100 (middle sharpness)
        0x00000024, 0x0028282c, 0x00380122, 0x026D0B54, 0x0327047C, 0x037700A4, 0xFFFFFD0D, 
    }},
    {set:{//01 Preview ISO200 (middle sharpness)
        0x00000024, 0x00282820, 0x00270031, 0x01ED0963, 0x0280038B, 0x02C000B3, 0xFFFFFD0D, 
    }},
    {set:{//02 Preview ISO400 (middle sharpness)
        0x00000024, 0x0028282c, 0x0012004B, 0x0140067D, 0x01A002A5, 0x01C900CD, 0xFFFFFD0D, 
    }},
    {set:{//03 Preview ISO800 (middle sharpness)
        0x00000024, 0x0028282c, 0x00110050, 0x012D0582, 0x018702AA, 0x01AE00D2, 0xFFFFFD0D, 
    }},
    {set:{//04 Preview ISO1600 (middle sharpness)
        0x00000024, 0x0028282c, 0x00110056, 0x01190588, 0x016D02B0, 0x019100D8, 0xFFFFFC0D, 
    }},
    {set:{//05 Capture ISO100     (o)                  // middle sharpness
        0x00000028, 0x0037372c, 0x00380122, 0x026D0B54, 0x0327047C, 0x037702A4, 0xFFFFFD0D
    }},
    {set:{//06 Capture ISO200   (o)                    // middle sharpness
        0x00000028, 0x007F3720, 0x00270031, 0x01ED0963, 0x0280038B, 0x02C001B3, 0xFFFFFD0D
    }},
    {set:{//07 Capture ISO400        (o)               // middle sharpness
        0x00000028, 0x0037372c, 0x0012004B, 0x0140067D, 0x01A002A5, 0x01C901CD, 0xFFFFFD0D
    }},
    {set:{//08 Capture ISO800    (o)                   // middle sharpness
        0x00000028, 0x0037372c, 0x00110050, 0x012D0582, 0x018702AA, 0x01AE00D2, 0xFFFFFD0D
    }},
    {set:{//09 Capture ISO1600                      // middle sharpness
        0x00000028, 0x0037372c, 0x00110056, 0x01190588, 0x016D02B0, 0x019100D8, 0xFFFFFC0D
    }},
    {set:{//10 no one uses this, this is Min EE     // low sharpness
        0x00000024, 0x0037372c, 0x00050023, 0x012C113C, 0x020001D0, 0x020000E7, 0xFFFF0005
    }},
    {set:{//11 no one uses this, this is Max EE     // high sharpness
        0x0000002c, 0x00373720, 0x000A0023, 0x03203937, 0x03FF01D0, 0x03FF00E7, 0xFFFF0005
    }}
}, 
Saturation:{
    {set:{//00 (middle saturation)
        0x00010708, 0x1020E0F0, 0x2E616461, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//01 (middle saturation)
        0x00010708, 0x1020E0F0, 0x2E616461, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//02 (middle saturation)
        0x00010708, 0x1020E0F0, 0x2E5D605D, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//03 (middle saturation)
        0x00010708, 0x1020E0F0, 0x2E535353, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//04 (middle saturation)
        0x00010708, 0x1020E0F0, 0x2E535353, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//05 no one uses this, this is Min Sat. (low saturation)
        0x00000708, 0x1020E0F0, 0x2E404040, 0x40000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }},
    {set:{//06 no one uses this, this is Max Sat. (high saturation)
        0x00000708, 0x1020E0F0, 0x2E606460, 0x30000000, 0xFF00FF00, 0x001A2D40, 0x000A0A0A, 0x00643C1E, 
    }}
}, 
Contrast:{
    //..........................................................................
    //  low brightness
    {set:{//00  //  low contrast
        0x00000000, 0x00F00000, 0xFF00003B
    }},
    {set:{//01  //  middle contrast
        0x00000000, 0x00F00000, 0xFF000040
    }},
    {set:{//02  //  high contrast
        0x00000000, 0x00F00000, 0xFF000045
    }}, 
    //..........................................................................
    //  middle brightness
    {set:{//03  //  low contrast
        0x00000000, 0x00000000, 0xFF00003B
    }},
    {set:{//04  //  middle contrast
        0x00000000, 0x00000000, 0xFF000040
    }},
    {set:{//05  //  high contrast
        0x00000000, 0x00000000, 0xFF000045
    }}, 
    //..........................................................................
    //  high brightness
    {set:{//06  //  low contrast
        0x00000000, 0x000A0000, 0xFF00003B
    }},
    {set:{//07  //  middle contrast
        0x00000000, 0x000A0000, 0xFF000040
    }},
    {set:{//08  //  high contrast
        0x00000000, 0x000A0000, 0xFF000045
    }}, 
}, 
Hue:{
    {set:{//00  //  low hue
        0x00000000, 0x808062AE
    }},
    {set:{//01  //  middle hue
        0x00000000, 0x00007F01
    }},
    {set:{//02  //  high hue
        0x00000000, 0x80806252
    }}
}, 
CCM:{
    {set:{//00
        0x01E404DA, 0x040A040F, 0x010A0005, 0x003504B1, 0x017C0000, 
    }},
    {set:{//01
        0x01E404DA, 0x040A040F, 0x010A0005, 0x003504B1, 0x017C0000, 
    }},
    {set:{//02
        0x01E404DA, 0x040A040F, 0x010A0005, 0x003504B1, 0x017C0000, 
    }}
},
Gamma:{
    {set:{//00
        0x3F1A0D03, 0x8a7C6C5a, 0xB4B2A196, 0xE4DcD4Cb, 0xFBF8F2EE
    }}
}

