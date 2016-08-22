#include <stdlib.h>
#include "nix_inner.h"

/**********************************************/
//��������:����һ������
//�������:pstrQue:��Ҫ�����Ķ��е�ָ�룬��ΪNULL���ɸú������������ڴ�
//����ֵ  :NULL:��������ʧ��
//         ����:�������гɹ�������ֵΪ�����Ķ���ָ��
/**********************************************/
NIX_QUE *NIX_QueCreate(NIX_QUE * pstrQue)
{
	U8 *pucQueMemAddr;

	if (pstrQue == NULL) {
		(void) NIX_IntLock();
		pucQueMemAddr = malloc(sizeof(NIX_QUE));

		if (pucQueMemAddr == NULL) {
			(void) NIX_IntUnLock();
			return (NIX_QUE *) NULL;
		}

		(void) NIX_IntLock();
		pstrQue = (NIX_QUE *) pucQueMemAddr;
	} else {
		pucQueMemAddr = (U8 *) NULL;
	}

	NIX_ListInit(&pstrQue->strList);

	pstrQue->pucQueMem = pucQueMemAddr;

	return pstrQue;
}

/**********************************************/
//��������:��һ���ڵ�������
//�������:pstrQue:����ָ��
//         pstrQueNode:Ҫ����Ķ��нڵ�ָ��
//����ֵ  :RTN_SUCD:�������гɹ�
//         RTN_FAIL:��������ʧ��
/**********************************************/
U32 NIX_QuePut(NIX_QUE * pstrQue, NIX_LIST * pstrQueNode)
{
	if ((pstrQue == NULL) || (pstrQueNode == NULL)) {
		return RTN_FAIL;
	}

	(void) NIX_IntLock();

	NIX_ListNodeAdd(&pstrQue->strList, pstrQueNode);

	(void) NIX_IntUnLock();
	return RTN_SUCD;
}

/**********************************************/
//��������:�Ӷ�����ȡ��һ���ڵ�
//�������:pstrQue:����ָ��
//         ppstrQueNode:��Ŷ��нڵ�ָ���ָ��
//����ֵ  :RTN_SUCD:�������гɹ�
//         RTN_FAIL:��������ʧ��
//         RTN_NULL:����Ϊ��
/**********************************************/
U32 NIX_QueGet(NIX_QUE * pstrQue, NIX_LIST ** ppstrQueNode)
{
	NIX_LIST *pstrQueNode;

	/* ��ڲ������ */
	if ((NULL == pstrQue) || (NULL == ppstrQueNode)) {
		return RTN_FAIL;
	}

	(void) NIX_IntLock();

	/* �Ӷ���ȡ���ڵ� */
	pstrQueNode = NIX_ListNodeDelete(&pstrQue->strList);

	(void) NIX_IntUnLock();

	/* ���в�Ϊ��, ����ȡ���ڵ� */
	if (NULL != pstrQueNode) {
		*ppstrQueNode = pstrQueNode;

		return RTN_SUCD;
	} else {		/* ����Ϊ��, �޷�ȡ���ڵ� */

		return RTN_NULL;
	}

}

/**********************************************/
//��������:ɾ��һ������
//�������:pstrQue:��ɾ���Ķ���ָ��
//����ֵ  :RTN_SUCD:ɾ�����гɹ�
//         RTN_FAIL:ɾ������ʧ��
/**********************************************/
U32 NIX_QueDelete(NIX_QUE * pstrQue)
{
	if (pstrQue == NULL) {
		return RTN_FAIL;
	}

	if (pstrQue->pucQueMem != NULL) {
		(void) NIX_IntLock();

		free(pstrQue->pucQueMem);

		(void) NIX_IntUnLock();
	}

	return RTN_SUCD;

}
