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
  � This source file, LOADERS.H is Copyright  (c) 1993-98 by Carlo Vogelsang. �
  � You may not copy, distribute,  duplicate or clone this file  in any form, �
  � modified or non-modified. It belongs to the author.  By copying this file �
  � you are violating laws and will be punished. I will knock your brains in  �
  � myself or you will be sued to death..                                     �
  �                                                                     Carlo �
  ��( How the fuck did you get this file anyway? )�����������������������������
*/
#ifndef _LOADERS_H
#define _LOADERS_H

#include <stdio.h>
#include "hdr\galaxy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void *		  (__cdecl *getmem)(size_t size);
extern void *		  (__cdecl *resizemem)(void *mem,size_t size);
extern void			  (__cdecl *freemem)(void *mem);
extern int			  (__cdecl *seek)(void *source,long int offset,int mode);
extern size_t		  (__cdecl *read)(void *dest,size_t size,size_t count,void *source);
extern size_t		  (__cdecl *write)(void *source,size_t size,size_t count,void *dest);
extern long			  int (__cdecl *tell)(void *source);


#ifdef __cplusplus
};
#endif

#endif
