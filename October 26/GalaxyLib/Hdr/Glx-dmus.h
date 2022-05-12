/*�- Internal revision no. 4.00b -���� Last revision at 19:12 on 20-06-1998 -��

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
  � This source file, GLX-DMUS.H is Copyright (c) 1993-98 by Carlo Vogelsang. �
  � You may not copy, distribute,  duplicate or clone this file  in any form, �
  � modified or non-modified. It belongs to the author.  By copying this file �
  � you are violating laws and will be punished. I will knock your brains in  �
  � myself or you will be sued to death..                                     �
  �                                                                     Carlo �
  ��( How the fuck did you get this file anyway? )�����������������������������
*/
#ifndef _GLXDMUS_H
#define _GLXDMUS_H

#include "hdr\galaxy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int    __cdecl glxDeinitDirectMusic(void);
extern int	  __cdecl glxInitDirectMusic(void);
extern int    __cdecl glxLoadDirectMusic(char *Filename,int Flags);
extern int	  __cdecl glxUnloadDirectMusic(void);
extern int    __cdecl glxStartDirectMusic(void);
extern int	  __cdecl glxStopDirectMusic(void);
extern int	  __cdecl glxControlDirectMusic(int Command,int Parameter);

#ifdef __cplusplus
};
#endif

#endif
