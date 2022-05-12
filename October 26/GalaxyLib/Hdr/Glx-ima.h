/*�- Internal revision no. 4.00b -���� Last revision at 19:12 on 11-06-1999 -��

                        The 32 bit definition headerfile

                �������� ������� ���    ������� ���  ��� ��� ���
                ���  ��� ��� ��� ���    ��� ���  ������  ��� ���
                ��� ���� ������� ���    �������    ��     �����
                ���  ��� ��� ��� ���    ��� ���  ������    ���
                �������� ��� ��� ������ ��� ��� ���  ���   ���

                               .. MUSIC SYSTEM ..
                This document contains confidential information
                     Copyright (c) 1993-99 Carlo Vogelsang

  ���������������������������������������������������������������������������Ŀ
  �۲� COPYRIGHT NOTICE �����������������������������������������������������۳
  ���������������������������������������������������������������������������Ĵ
  � This source file, GLX-IMA.H is Copyright  (c) 1993-99 by Carlo Vogelsang. �
  � You may not copy, distribute,  duplicate or clone this file  in any form, �
  � modified or non-modified. It belongs to the author.  By copying this file �
  � you are violating laws and will be punished. I will knock your brains in  �
  � myself or you will be sued to death..                                     �
  �                                                                     Carlo �
  ��( How the fuck did you get this file anyway? )�����������������������������
*/
#ifndef _GLXIMA_H
#define _GLXIMA_H

#include "hdr\galaxy.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack (push,1) 							/* Turn off alignment */

typedef struct									 
{
	struct
	{
		short Sample;							// Current sample
		signed char Index;						// Current step_size index
		unsigned char Reserved;					// Reserved
	}	Channel[2];
	//additional infomation
	int Channels;								// Channels
	int SamplesPerFrame;						// Samples per frame
	int FrameSize;								// Framesize in bytes
	int BitsPerSample;							// Bits per sample
	//decoder variables
	int Samples;								// Samples left
	int	Index;									// Current sample
	short pcmsamples[2][4096];					// 16 bit PCM samples
} IMAAudioStream; 								// IMA Audio header

#pragma pack (pop)								/* Default alignment */

extern int			   __cdecl glxDecodeIMA(glxSample *Sample,void *IMAStream,int IMAStreamSize,short *LeftWaveStream,short *RightWaveStream,int WaveStreamSize,int *BytesRead,int *BytesWritten);

#ifdef __cplusplus
};
#endif

#endif
