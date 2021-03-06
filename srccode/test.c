#include "test.h"

NIX_QUE gstrSerialMsgQue;	/* 串口打印消息队列指针 */

NIX_TCB *gpstrSerialTaskTcb;	/* 串口打印任务TCB指针 */

NIX_TCB *gpstrTask4Tcb;		/* task4任务TCB指针 */


/**********************************************/
//函数功能:测试任务1
//输入参数:pvPara:任务入口指针
//返回值  :none
/**********************************************/
void TEST_TestTask1(void *pvPara)
{
	while (1) {
		/* 任务打印 */
		DEV_PutStrToMem((U8 *) "\r\nTask1 is running! Tick is: %d",
				NIX_GetSystemTick());

		/* 任务运行1秒 */
		TEST_TaskRun(1000);

		/* 任务延迟1秒 */
		(void) NIX_TaskDelay(100);
	}
}


/**********************************************/
//函数功能:测试任务2
//输入参数:pvPara:任务入口指针
//返回值  :none
/**********************************************/
void TEST_TestTask2(void *pvPara)
{
	while (1) {
		/* 任务打印 */
		DEV_PutStrToMem((U8 *) "\r\nTask2 is running! Tick is: %d",
				NIX_GetSystemTick());

		/* 任务运行2秒 */
		TEST_TaskRun(2000);

		/* 任务延迟1.5秒 */
		(void) NIX_TaskDelay(150);
	}
}


/**********************************************/
//函数功能:测试任务3
//输入参数:pvPara:任务入口指针
//返回值  :none
/**********************************************/
void TEST_TestTask3(void *pvPara)
{
	U8 i;
	/* 任务打印 */
	DEV_PutStrToMem((U8 *) "\r\nTask3 is running! Tick is: %d",
			NIX_GetSystemTick());

	/* 唤醒task4 */
	(void) NIX_TaskWake(gpstrTask4Tcb);

	for (i = 0; i < 5; i++) {
		/* 任务打印 */
		DEV_PutStrToMem((U8 *) "\r\nTask3 is running! Tick is: %d",
				NIX_GetSystemTick());

		/* 任务运行5秒 */
		TEST_TaskRun(3000);

		/* 任务延迟5秒 */
		(void) NIX_TaskDelay(700);

	}

}

/**********************************************/
//函数功能:测试任务4
//输入参数:pvPara:任务入口指针
//返回值  :none
/**********************************************/
void TEST_TestTask4(void *pvPara)
{
	while (1) {
		/* 任务打印 */
		DEV_PutStrToMem((U8 *) "\r\nTask4 is running! Tick is: %d",
				NIX_GetSystemTick());

		/* 任务运行1秒 */
		TEST_TaskRun(1000);

		/* 任务延迟10秒 */
		(void) NIX_TaskDelay(1000);
	}

}

/**********************************************/
//函数功能:串口打印任务，从队列中获取需要打印的消息缓冲，将缓冲中的数据打印到串口
//输入参数:pvPara:任务入口指针
//返回值  :none
/**********************************************/
void TEST_SerialPrintTask(void *pvPara)
{
	NIX_LIST *pstrMsgQueNode;
	MSGBUF *pstrMsgBuf;

	/* 从队列循环获取消息 */
	while (1) {
		/* 从队列中获取到一条需要打印的消息, 向串口打印消息数据 */
		if (RTN_SUCD ==
		    NIX_QueGet(&gstrSerialMsgQue, &pstrMsgQueNode)) {
			pstrMsgBuf = (MSGBUF *) pstrMsgQueNode;

			/* 将缓冲中的数据打印到串口 */
			DEV_PrintMsg(pstrMsgBuf->aucBuf,
				     pstrMsgBuf->ucLength);

			/* 缓冲消息中的数据发送完毕, 释放缓冲 */
			DEV_BufferFree(&gstrBufPool, pstrMsgQueNode);
		} else {	/* 没有获取到消息, 延迟一段时间后再查询队列 */

			(void) NIX_TaskDelay(100);
		}
	}

}

/**********************************************/
//函数功能:模拟任务运行函数
//输入参数:uiMs:要延迟的时间，单位ms
//返回值  :none
/**********************************************/
void TEST_TaskRun(U32 uiMs)
{
	DEV_DelayMs(uiMs);
}

/**********************************************/
//函数功能:将任务创建过程打印到内存中
//输入参数:pstrTcb:新创建的任务的TCB指针
//返回值  :none
/**********************************************/
void TEST_TaskCreatePrint(NIX_TCB * pstrTcb)
{
	if (pstrTcb != (NIX_TCB *) NULL) {
		DEV_PutStrToMem((U8 *)
				"\r\nTask %s is created! Tick is: %d",
				pstrTcb->pucTaskName, NIX_GetSystemTick());
	} else {
		DEV_PutStrToMem((U8 *)
				"\r\nFail to create task! Tick is: %d",
				NIX_GetSystemTick());
	}
}

/**********************************************/
//函数功能:将任务切换过程打印到内存中
//输入参数:pstrOldTcb:切换前的任务TCB指针
//         pstrNewTcb:切换后的任务TCB指针
//返回值  :none
/**********************************************/
void TEST_TaskSwitchPrint(NIX_TCB * pstrOldTcb, NIX_TCB * pstrNewTcb)
{
	//不打印串口打印任务的切换过程,将此任务定义为空闲任务
	if (pstrOldTcb == gpstrSerialTaskTcb) {
		pstrOldTcb = NIX_GetIdleTcb();
	}

	if (pstrNewTcb == gpstrSerialTaskTcb) {
		pstrNewTcb = NIX_GetIdleTcb();
	}
	//同一个任务切换不打印信息
	if (pstrNewTcb == pstrOldTcb) {
		return;
	}

	if (pstrOldTcb != (NIX_TCB *) NULL) {
		DEV_PutStrToMem((U8 *)
				"\r\nTask %s ----> Task %s ! Tick is: %d",
				pstrOldTcb->pucTaskName,
				pstrNewTcb->pucTaskName,
				NIX_GetSystemTick());
	} else {
		DEV_PutStrToMem((U8 *)
				"\r\nTask NULL ----> Task %s ! Tick is: %d",
				pstrNewTcb->pucTaskName,
				NIX_GetSystemTick());
	}

}

/**********************************************/
//函数功能:将任务删除过程打印到内存中
//输入参数:pstrTcb:被删除的任务的TCB指针
//返回值  :none
/**********************************************/
void TEST_TaskDeletePrint(NIX_TCB * pstrTcb)
{
	DEV_PutStrToMem((U8 *) "\r\nTask %s is deleted! Tick is: %d",
			pstrTcb->pucTaskName, NIX_GetSystemTick());
}
