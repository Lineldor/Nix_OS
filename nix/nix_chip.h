#ifndef NIX_CHIP_H
#define NIX_CHIP_H

//���жϷ���ţ�0x00000000~0x1FFFFFFFΪ����ϵͳ����
#define SWI_TASKSCHED               0x10001001      //�������

extern void NIX_IntPendSvSet(void);
extern U32  NIX_RunInInt(void);
extern U32  NIX_IntLock(void);
extern U32  NIX_IntUnLock(void);

#endif
