
#ifndef MDS_SEM_H
#define MDS_SEM_H


/* ��ȡ�ź����ķ���ֵ */
#define RTN_SMTKTO          2           /* �ȴ��ź�����ʱ��ľ�, ��ʱ���� */
#define RTN_SMTKRT          3           /* û�л�ȡ���ź���, ֱ�ӷ��� */
#define RTN_SMTKDL          4           /* �ź�����ɾ�� */


/* �ź�������ƫ���� */
#define SEMSCHOF            0           /* �ź��������е��ȷ�ʽ��ƫ���� */

/* �ź������ȷ�ʽ */
#define SEMFIFO             (1 << SEMSCHOF) /* �ź��������Ƚ��ȳ����� */
#define SEMPRIO             (2 << SEMSCHOF) /* �ź����������ȼ����� */

/* �ź�����ʼֵ */
#define SEMEMPTY            0           /* �ź���Ϊ��״̬ */
#define SEMFULL             0xFFFFFFFF  /* �ź���Ϊ��״̬ */


/* �ӳٵȴ���ʱ�� */
#define SEMNOWAIT           0           /* ���ȴ� */
#define SEMWAITFEV          0xFFFFFFFF  /* ���õȴ� */

#endif

