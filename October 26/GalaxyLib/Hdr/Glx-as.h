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
  � This source file, GLX-AS.H is Copyright  (c)  1993-98 by Carlo Vogelsang. �
  � You may not copy, distribute,  duplicate or clone this file  in any form, �
  � modified or non-modified. It belongs to the author.  By copying this file �
  � you are violating laws and will be punished. I will knock your brains in  �
  � myself or you will be sued to death..                                     �
  �                                                                     Carlo �
  ��( How the fuck did you get this file anyway? )�����������������������������
*/
#ifndef _GLXAS_H
#define _GLXAS_H

#include "hdr\galaxy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern glxSample *	   __cdecl glxLoadAS(glxSample *Sample,void *Stream,int Mode);
extern int			   __cdecl glxSaveAS(glxSample *Sample,void *Stream,int Mode);

#ifdef __cplusplus
};
#endif

#endif
