/*�- Internal revision no. 4.00b -���� Last revision at 19:12 on 21-04-1998 -��

                        The 32 bit definition headerfile

                �������� ������� ���    ������� ���  ��� ��� ���
                ���  ��� ��� ��� ���    ��� ���  ������  ��� ���
                ��� ���� ������� ���    �������    ��     �����
                ���  ��� ��� ��� ���    ��� ���  ������    ���
                �������� ��� ��� ������ ��� ��� ���  ���   ���

                               .. MUSIC SYSTEM ..
                This document contains confidential information
                     Copyright (c) 1993-97 Carlo Vogelsang

  ���������������������������������������������������������������������������Ŀ
  �۲� COPYRIGHT NOTICE �����������������������������������������������������۳
  ���������������������������������������������������������������������������Ĵ
  � This source file, GLX-WAV.H is Copyright  (c) 1993-98 by Carlo Vogelsang. �
  � You may not copy, distribute,  duplicate or clone this file  in any form, �
  � modified or non-modified. It belongs to the author.  By copying this file �
  � you are violating laws and will be punished. I will knock your brains in  �
  � myself or you will be sued to death..                                     �
  �                                                                     Carlo �
  ��( How the fuck did you get this file anyway? )�����������������������������
*/
#ifndef _GLXWAV_H
#define _GLXWAV_H

#include "hdr\galaxy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern glxSample *	   __cdecl glxLoadWAV(glxSample *Sample,void *Effect,int Mode);
extern int			   __cdecl glxLoadWAV2(int Instrument,void *Effect,int Mode);
extern int			   __cdecl glxDecodeWAV(glxSample *Sample,void *WAVStream,int WAVStreamSize,short *LeftWaveStream,short *RightWaveStream,int WaveStreamSize,int *BytesRead,int *BytesWritten);

#ifdef __cplusplus
};
#endif

#endif
