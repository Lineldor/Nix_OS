#ifndef NIX_QUEUE_H
#define NIX_QUEUE_H

#define RTN_NULL                2	//����Ϊ��

typedef struct nix_que		//���нṹ
{
	NIX_LIST strList;	//��������
	U8 *pucQueMem;		//��������ʱ���ڴ��ַ
} NIX_QUE;

extern NIX_QUE *NIX_QueCreate(NIX_QUE * pstrQue);
extern U32 NIX_QuePut(NIX_QUE * pstrQue, NIX_LIST * pstrQueNode);
extern U32 NIX_QueGet(NIX_QUE * pstrQue, NIX_LIST * *ppstrQueNode);
extern U32 NIX_QueDelete(NIX_QUE * pstrQue);

#endif
