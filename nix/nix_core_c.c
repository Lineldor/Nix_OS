#include "nix_inner.h"

U32 guiSystemStatus;		//系统状态

NIX_TASKSCHEDTAB gstrReadyTab;	//任务就绪表
NIX_LIST gstrDelayTab;		//任务延迟表

U32 guiTick;			//操作系统tick计数
U8 gucTickSched;		//tick调度标志

STACKREG *gpstrCurTaskReg;	//当前任务的寄存器组指针
STACKREG *gpstrNextTaskReg;	//下一个任务的寄存器组指针

NIX_TCB *gpstrCurTcb;		//当前任务的TCB指针
NIX_TCB *gpstrRootTaskTcb;	//前根任务的TCB指针
NIX_TCB *gpstrIdleTaskTcb;	//Idle任务的TCB指针

U32 guiUser;			//操作系统所使用的用户状态

//优先级反向查找表
const U8 caucTaskPrioUnmapTab[256] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/**********************************************/
//函数功能:主函数，非OS态切换到OS态ROOT任务运行
//输入参数:none
//返回值  :不会运行到返回
/**********************************************/
S32 main(void)
{
	//初始化系统变量，建立操作系统启动所需要的环境
	NIX_SystemVarInit();
	//运行根任务
	NIX_TaskStart(gpstrRootTaskTcb);
	return 0;
}

/**********************************************/
//函数功能:初始化系统变量
//输入参数:none
//返回值  :none
/**********************************************/
void NIX_SystemVarInit(void)
{
	NIX_SetUser(USERROOT);	//设置为高级用户权限
	guiSystemStatus = SYSTEMNOTSCHEDULE;	//初始化为未进入操作系统状态

	//将当前任务、root任务和idle任务的TCB初始化为NULL
	gpstrCurTcb = (NIX_TCB *) NULL;
	gpstrRootTaskTcb = (NIX_TCB *) NULL;
	gpstrIdleTaskTcb = (NIX_TCB *) NULL;

	//tick计数初始化
	guiTick = 0;

	//初始化为非tick中断调度状态
	gucTickSched = TICKSCHEDCLR;

	//初始化锁中断计数为0
	guiIntLockCounter = 0;

#ifdef NIX_INCLUDETASKHOOK
	NIX_TaskHookInit();
#endif

	//初始化任务ready调度表
	NIX_TaskSchedTabInit(&gstrReadyTab);

	//初始化任务delay调度表
	NIX_ListInit(&gstrDelayTab);

	//创建前根任务
	gpstrRootTaskTcb =
	    NIX_TaskCreat(ROOTTASKNAME, NIX_BeforeRootTask, NULL, NULL,
			  ROOTTASKSTACK, USERHIGHESTPRIO, NULL);

	//创建Idle任务
	gpstrIdleTaskTcb =
	    NIX_TaskCreat(IDLETASKNAME, NIX_IdleTask, NULL, NULL,
			  IDLETASKSTACK, LOWESTPRIO, NULL);

}

/**********************************************/
//函数功能:前根任务，初始化硬件，设置用户权限，调用根任务
//输入参数:pvPara:入口参数
//返回值  :none
/**********************************************/
void NIX_BeforeRootTask(void *pvPara)
{
	NIX_SystemHardwareInit();
	guiSystemStatus = SYSTEMSCHEDULE;
	NIX_SetUser(USERGUEST);
	NIX_RootTask();
}

/*************************************************************************************/
/**************************************获取系统状态***********************************/
/*************************************************************************************/

/**********************************************/
//函数功能:获取当前的tick
//输入参数:none
//返回值  :系统当前的tick
/**********************************************/
U32 NIX_GetSystemTick(void)
{
	return guiTick;
}

/**********************************************/
//函数功能:获取当前任务的TCB指针
//输入参数:none
//返回值  :系统当前的TCB指针
/**********************************************/
NIX_TCB *NIX_GetCurrentTcb(void)
{
	return gpstrCurTcb;
}

/**********************************************/
//函数功能:获取根任务的TCB指针
//输入参数:none
//返回值  :根任务的TCB指针
/**********************************************/
NIX_TCB *NIX_GetRootTcb(void)
{
	return gpstrRootTaskTcb;
}

/**********************************************/
//函数功能:获取空闲任务的TCB指针
//输入参数:none
//返回值  :空闲任务的TCB指针
/**********************************************/
NIX_TCB *NIX_GetIdleTcb(void)
{
	return gpstrIdleTaskTcb;
}

/**********************************************/
//函数功能:设置用户
//输入参数:uiUser: 需要设置的用户
//返回值  :none
/**********************************************/
void NIX_SetUser(U32 uiUser)
{
	guiUser = uiUser;
}

/**********************************************/
//函数功能:获取用户
//输入参数:none
//返回值  :获取到的用户
/**********************************************/
U32 NIX_GetUser(void)
{
	return guiUser;
}

/*************************************************************************************/
/**************************************中断调度相关***********************************/
/*************************************************************************************/

/**********************************************/
//函数功能:tick中断调度任务函数
//输入参数:none
//返回值  :none
/**********************************************/
void NIX_TaskTick(void)
{
	guiTick++;
	gucTickSched = TICKSCHEDSET;
	NIX_IntPendSvSet();
}

/**********************************************/
//函数功能:为发生的任务切换做准备，更新汇编函数所使用的变量
//输入参数:pstrTcb: 即将运行的任务TCB指针
//返回值  :none
/**********************************************/
void NIX_TaskSwitch(NIX_TCB * pstrTcb)
{
	if (gpstrCurTcb != NULL) {
		gpstrCurTaskReg = &gpstrCurTcb->strStackReg;
	} else {
		gpstrCurTaskReg = NULL;
	}
	gpstrNextTaskReg = &pstrTcb->strStackReg;
	gpstrCurTcb = pstrTcb;
}

/**********************************************/
//函数功能:从非操作系统状态切换到操作系统状态
//输入参数:pstrTcb: 即将运行的任务TCB指针
//返回值  :none
/**********************************************/
void NIX_TaskStart(NIX_TCB * pstrTcb)
{
	gpstrNextTaskReg = &pstrTcb->strStackReg;
	gpstrCurTcb = pstrTcb;
	NIX_SwitchToTask();
}

/**********************************************/
//函数功能:任务调度，实现状态切换
//输入参数:none
//返回值  :none
/**********************************************/
void NIX_TaskSched(void)
{
	NIX_TCB *pstrTcb;

	if (gucTickSched == TICKSCHEDSET) {
		gucTickSched = TICKSCHEDCLR;
		NIX_TaskDelayTabSched();
	}
	pstrTcb = NIX_TaskReadyTabSched();

#ifdef NIX_INCLUDETASKHOOK
	if (gvfTaskSwitchHook != (VFHSWT) NULL) {
		gvfTaskSwitchHook(gpstrCurTcb, pstrTcb);
	}
#endif

	NIX_TaskSwitch(pstrTcb);
}

/*************************************************************************************/
/**************************************ready表相关************************************/
/*************************************************************************************/

/**********************************************/
//函数功能:初始化调度表
//输入参数:pstrSchedTab:调度表指针
//返回值  :none
/**********************************************/
void NIX_TaskSchedTabInit(NIX_TASKSCHEDTAB * pstrSchedTab)
{
	U32 i;
	for (i = 0; i < PRIORITYNUM; i++) {
		NIX_ListInit(&pstrSchedTab->astrList[i]);
	}
#if PRIORITYNUM >= PRIORITY128
	for (i = 0; i < PRIOFLAGGRP1; i++) {
		pstrSchedTab->strFlag.aucPrioFlagGRP1[i] = 0;
	}
	for (i = 0; i < PRIOFLAGGRP2; i++) {
		pstrSchedTab->strFlag.aucPrioFlagGRP2[i] = 0;
	}
	pstrSchedTab->strFlag.ucPrioFlagGrp3 = 0;
#elif PRIORITYNUM >= PRIORITY16
	for (i = 0; i < PRIOFLAGGRP1; i++) {
		pstrSchedTab->strFlag.aucPrioFlagGRP1[i] = 0;
	}
	pstrSchedTab->strFlag.ucPrioFlagGRP2 = 0;
#else
	pstrSchedTab->strFlag.ucPrioFlagGRP1 = 0;
#endif
}

/**********************************************/
//函数功能:将任务按照优先级添加到调度表
//输入参数:pstrList:调度表指针
//         pstrNode:待添加的节点指针
//         pstrPrioFlag:优先级标识表
//         ucTaskPrio:待添加的节点的优先级
//返回值  :none
/**********************************************/
void NIX_TaskAddToSchedTab(NIX_LIST * pstrList, NIX_LIST * pstrNode,
			   NIX_PRIOFLAG * pstrPrioFlag, U8 ucTaskPrio)
{
	NIX_ListNodeAdd(pstrList, pstrNode);
	NIX_TaskSetPrioFlag(pstrPrioFlag, ucTaskPrio);
}

/**********************************************/
//函数功能:从调度表中删除任务
//输入参数:pstrList:调度表指针
//         pstrPrioFlag:调度表对应的优先级标识指针
//         ucTaskPrio:要删除任务的优先级
//返回值  :被删除的任务的节点指针
/**********************************************/
NIX_LIST *NIX_TaskDelFromSchedTab(NIX_LIST * pstrList,
				  NIX_PRIOFLAG * pstrPrioFlag,
				  U8 ucTaskPrio)
{
	NIX_LIST *pstrDelNode;
	pstrDelNode = NIX_ListNodeDelete(pstrList);
	if (NIX_ListEmpInq(pstrList) == NULL) {
		NIX_TaskClrPrioFlag(pstrPrioFlag, ucTaskPrio);
	}
	return pstrDelNode;
}

/**********************************************/
//函数功能:对ready表进行调度
//输入参数:none
//返回值  :即将运行的任务Tcb指针
/**********************************************/
NIX_TCB *NIX_TaskReadyTabSched(void)
{
	NIX_TCB *pstrTcb;
	NIX_TCBQUE *pstrTaskQue;
	U8 ucTaskPrio;

	ucTaskPrio = NIX_TaskGetHighestPrio(&gstrReadyTab.strFlag);
	pstrTaskQue = (NIX_TCBQUE *)
	    NIX_ListEmpInq(&gstrReadyTab.astrList[ucTaskPrio]);
	pstrTcb = pstrTaskQue->pstrTcb;

	return pstrTcb;
}

/*************************************************************************************/
/**************************************delay表相关************************************/
/*************************************************************************************/
//delay表是按照等待时间排列的单一队列
//因此delay表的创建和delay表节点的删除即为list的初始化和节点删除
/**********************************************/
//函数功能:将任务按照优先级添加到Delay表
//输入参数:pstrNode:待添加的节点指针
//返回值  :none
/**********************************************/
void NIX_TaskAddToDelayTab(NIX_LIST * pstrNode)
{
	NIX_LIST *pstrTempNode;
	NIX_LIST *pstrNextNode;
	NIX_TCBQUE *pstrTcbQue;
	U32 uiStillTick;
	U32 uiTempStillTick;

	pstrTempNode = NIX_ListEmpInq(&gstrDelayTab);

	if (pstrTempNode != NULL) {
		pstrTcbQue = (NIX_TCBQUE *) pstrNode;
		uiStillTick = pstrTcbQue->pstrTcb->uiStillTick;

		while (1) {
			pstrTcbQue = (NIX_TCBQUE *) pstrTempNode;
			uiTempStillTick = pstrTcbQue->pstrTcb->uiStillTick;
			//接下来的逻辑暂时没有通过验证
			//验证时请充分考虑数据溢出的情况�
			//未来考虑参考freertos的实现
			if (uiStillTick < uiTempStillTick) {
				if (uiStillTick > guiTick) {
					NIX_ListNodeInsert(&gstrDelayTab,
							   pstrTempNode,
							   pstrNode);

					return;
				} else if (uiStillTick < guiTick) {
					if (guiTick > uiTempStillTick) {
						NIX_ListNodeInsert
						    (&gstrDelayTab,
						     pstrTempNode,
						     pstrNode);

						return;
					}
				}
			} else if (uiStillTick > uiTempStillTick) {
				if (uiStillTick > guiTick) {
					if (guiTick > uiTempStillTick) {
						NIX_ListNodeInsert
						    (&gstrDelayTab,
						     pstrTempNode,
						     pstrNode);

						return;
					}
				}
			}


			pstrNextNode =
			    NIX_ListNextNodeEmpInq(&gstrDelayTab,
						   pstrTempNode);

			if (pstrNextNode == NULL) {
				NIX_ListNodeAdd(&gstrDelayTab, pstrNode);
				return;
			} else {
				pstrTempNode = pstrNextNode;
			}

		}
	} else {
		NIX_ListNodeAdd(&gstrDelayTab, pstrNode);
		return;
	}
}

/**********************************************/
//函数功能:对delay表进行调度
//输入参数:none
//返回值  :none
/**********************************************/
void NIX_TaskDelayTabSched(void)
{
	NIX_TCB *pstrTcb;
	NIX_LIST *pstrList;
	NIX_LIST *pstrNode;
	NIX_LIST *pstrDelayNode;
	NIX_LIST *pstrNextNode;
	NIX_PRIOFLAG *pstrPrioFlag;
	NIX_TCBQUE *pstrTcbQue;
	U32 uiTick;
	U8 ucTaskPrio;

	pstrDelayNode = NIX_ListEmpInq(&gstrDelayTab);

	if (pstrDelayNode != NULL) {
		while (1) {
			pstrTcbQue = (NIX_TCBQUE *) pstrDelayNode;
			pstrTcb = pstrTcbQue->pstrTcb;
			uiTick = pstrTcb->uiStillTick;

			if (guiTick == uiTick) {
				pstrNextNode =
				    NIX_ListCurNodeDelete(&gstrDelayTab,
							  pstrDelayNode);

				pstrTcb->uiTaskFlag &=
				    (~((U32) DELAYQUEFLAG));
				pstrTcb->strTaskOpt.ucTaskSta &=
				    (~((U8) TASKDELAY));
				pstrTcb->strTaskOpt.uiDelayTick =
				    RTN_TKDLTO;

				pstrNode = &pstrTcb->strTcbQue.strQueHead;
				ucTaskPrio = pstrTcb->ucTaskPrio;
				pstrList =
				    &gstrReadyTab.astrList[ucTaskPrio];
				pstrPrioFlag = &gstrReadyTab.strFlag;

				NIX_TaskAddToSchedTab(pstrList, pstrNode,
						      pstrPrioFlag,
						      ucTaskPrio);

				pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;

				if (pstrNextNode == NULL) {
					break;
				} else {
					pstrDelayNode = pstrNextNode;
				}

			} else {
				break;
			}
		}
	}

}

/*************************************************************************************/
/**************************************sem表相关************************************/
/*************************************************************************************/
//初始化，添加，删除，调度
/**********************************************/
//函数功能:将任务添加到sem表
//输入参数:pstrTcb:待添加的节点指针
//         pstrSem:添加到的sem表指针
//返回值  :none
/**********************************************/
void NIX_TaskAddToSemTab(NIX_TCB * pstrTcb, NIX_SEM * pstrSem)
{
	NIX_LIST *pstrList;
	NIX_LIST *pstrNode;
	NIX_PRIOFLAG *pstrPrioFlag;
	U8 ucTaskPrio;

	if ((pstrSem->uiSemOpt & SEMSCHEDOPTMASK) == SEMPRIO) {
		ucTaskPrio = pstrTcb->ucTaskPrio;
		pstrList = &pstrSem->strSemtab.astrList[ucTaskPrio];
		pstrNode = &pstrTcb->strSemQue.strQueHead;
		pstrPrioFlag = &pstrSem->strSemtab.strFlag;

		NIX_TaskAddToSchedTab(pstrList, pstrNode, pstrPrioFlag,
				      ucTaskPrio);
	} else {
		pstrList = &pstrSem->strSemtab.astrList[LOWESTPRIO];
		pstrNode = &pstrTcb->strSemQue.strQueHead;

		NIX_ListNodeAdd(pstrList, pstrNode);
	}

	pstrTcb->pstrSem = pstrSem;

}

/**********************************************/
//函数功能:将任务从sem表中删除
//输入参数:pstrTcb:待删除的节点指针
//返回值  :删除任务的TCB指针
/**********************************************/
NIX_LIST *NIX_TaskDelFromSemTab(NIX_TCB * pstrTcb)
{
	NIX_SEM *pstrSem;
	NIX_LIST *pstrList;
	NIX_LIST *pstrPrioFlag;
	U8 ucTaskPrio;

	pstrSem = pstrTcb->pstrSem;

	if ((pstrSem->uiSemOpt & SEMSCHEDOPTMASK) == SEMPRIO) {
		ucTaskPrio = pstrTcb->ucTaskPrio;
		pstrList = &pstrSem->strSemtab.astrList[ucTaskPrio];
		pstrPrioFlag = &pstrSem->strSemtab.strFlag;

		NIX_TaskDelFromSchedTab(pstrList, pstrPrioFlag,
					ucTaskPrio);
	} else {
		pstrList = &pstrSem->strSemtab.astrList[LOWESTPRIO];

		NIX_ListNodeDelete(NIX_LIST * pstrList);
	}
}

/**********************************************/
//函数功能:调度sem表
//输入参数:pstrSem:待调度的sem表指针
//返回值  :查找到的任务TCB指针
/**********************************************/
NIX_TCB *NIX_SemGetAcitveTask(NIX_SEM * pstrSem)
{
	NIX_TCBQUE *pstrTaskQue;
	NIX_LIST *pstrNode;
	U8 ucTaskPrio;

	if ((pstrSem->uiSemOpt & SEMSCHEDOPTMASK) == SEMPRIO) {
		ucTaskPrio =
		    NIX_TaskGetHighestPrio(&pstrSem->strSemtab.strFlag);
	} else {
		ucTaskPrio = LOWESTPRIO;
	}

	pstrNode =
	    NIX_ListEmpInq(&pstrSem->strSemtab.astrList[ucTaskPrio]);

	if (pstrNode == NULL) {
		return (NIX_TCB *) NULL;
	} else {
		pstrTaskQue = (NIX_TCBQUE *) pstrNode;

		return pstrTaskQue->pstrTcb;
	}
}

/*************************************************************************************/
/**********************************优先级标识表相关***********************************/
/*************************************************************************************/

/**********************************************/
//函数功能:将任务的优先级添加到优先级标识表
//输入参数:pstrPrioFlag:优先级标识表
//         ucTaskPrio:待添加的节点的优先级
//返回值  :none
/**********************************************/
void NIX_TaskSetPrioFlag(NIX_PRIOFLAG * pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
	U8 ucPrioFlagGrp1;
	U8 ucPrioFlagGrp2;
	U8 ucPosInGrp1;
	U8 ucPosInGrp2;
	U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
	U8 ucPrioFlagGrp1;
	U8 ucPosInGrp1;
	U8 ucPosInGrp2;
#endif

#if PRIORITYNUM >= PRIORITY128
	//Byte
	ucPrioFlagGrp1 = ucTaskPrio / 8;
	ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;
	//bit
	ucPosInGrp1 = ucTaskPrio % 8;
	ucPosInGrp2 = ucPrioFlagGrp1 % 8;
	ucPosInGrp3 = ucPrioFlagGrp2;

	pstrPrioFlag->aucPrioFlagGRP1[ucPrioFlagGrp1] |=
	    (U8) (1 << ucPosInGrp1);
	pstrPrioFlag->aucPrioFlagGRP2[ucPrioFlagGrp2] |=
	    (U8) (1 << ucPosInGrp2);
	pstrPrioFlag->ucPrioFlagGrp3 |= (U8) (1 << ucPosInGrp3);
#elif PRIORITYNUM >= PRIORITY16
	ucPrioFlagGrp1 = ucTaskPrio / 8;

	ucPosInGrp1 = ucTaskPrio % 8;
	ucPosInGrp2 = ucPrioFlagGrp1;

	pstrPrioFlag->aucPrioFlagGRP1[ucPrioFlagGrp1] |=
	    (U8) (1 << ucPosInGrp1);
	pstrPrioFlag->ucPrioFlagGRP2 |= (U8) (1 << ucPosInGrp2);
#else
	pstrPrioFlag->ucPrioFlagGRP1 |= (U8) (1 << ucTaskPrio);
#endif
}

/**********************************************/
//函数功能:将该任务从任务调度表对应的优先级标识中清除
//输入参数:pstrPrioFlag:调度表对应的优先级标识指针
//         ucTaskPrio:任务的优先级
//返回值  :none
/**********************************************/
void NIX_TaskClrPrioFlag(NIX_PRIOFLAG * pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
	U8 ucPrioFlagGrp1;
	U8 ucPrioFlagGrp2;
	U8 ucPosInGrp1;
	U8 ucPosInGrp2;
	U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
	U8 ucPrioFlagGrp1;
	U8 ucPosInGrp1;
	U8 ucPosInGrp2;
#endif

#if PRIORITYNUM >= PRIORITY128
	//Byte
	ucPrioFlagGrp1 = ucTaskPrio / 8;
	ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;
	//bit
	ucPosInGrp1 = ucTaskPrio % 8;
	ucPosInGrp2 = ucPrioFlagGrp1 % 8;
	ucPosInGrp3 = ucPrioFlagGrp2;

	pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &=
	    ~((U8) (1 << ucPosInGrp1));
	if (pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] == 0) {
		pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] &=
		    ~((U8) (1 << ucPosInGrp2));
		if (pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] == 0) {
			pstrPrioFlag->ucPrioFlagGrp3 &=
			    ~((U8) (1 << ucPosInGrp3));
		}
	}
#elif PRIORITYNUM >= PRIORITY16

	ucPrioFlagGrp1 = ucTaskPrio / 8;

	ucPosInGrp1 = ucTaskPrio % 8;
	ucPosInGrp2 = ucPrioFlagGrp1;

	pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &=
	    ~((U8) (1 << ucPosInGrp1));
	if (pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] == 0) {
		pstrPrioFlag->ucPrioFlagGrp2 &= ~((U8) (1 << ucPosInGrp2));
	}
#else
	pstrPrioFlag->ucPrioFlagGRP1 &= ~((U8) (1 << ucTaskPrio));
#endif

}


/**********************************************/
//函数功能:获取优先级标识表中的最高优先级
//输入参数:pstrPrioFlag:优先级标识表
//返回值  :查找到的最高优先级
/**********************************************/
U8 NIX_TaskGetHighestPrio(NIX_PRIOFLAG * pstrPrioFlag)
{
#if PRIORITYNUM >= PRIORITY128
	U8 ucPrioFlagGrp1;
	U8 ucPrioFlagGrp2;
	U8 ucHighestFlagInGrp1;
#elif PRIORITYNUM >= PRIORITY16
	U8 ucPrioFlagGrp1;
	U8 ucHighestFlagInGrp1;
#endif

#if PRIORITYNUM >= PRIORITY128
	ucPrioFlagGrp2 =
	    caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp3];
	ucPrioFlagGrp1 =
	    caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGRP2
				 [ucPrioFlagGrp2]];
	ucHighestFlagInGrp1 =
	    caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGRP1
				 [ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1]];
	return (U8) ((ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1) * 8 +
		     ucHighestFlagInGrp1);
#elif PRIORITYNUM >= PRIORITY16
	ucPrioFlagGrp1 =
	    caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGRP2];
	ucHighestFlagInGrp1 =
	    caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGRP1
				 [ucPrioFlagGrp1]];
	return (U8) (ucPrioFlagGrp1 * 8 + ucHighestFlagInGrp1);
#else
	return caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGRP1];
#endif
}
