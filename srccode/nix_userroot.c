#include "nix_userroot.h"

/**********************************************/
//��������:�������û�����Ӵ�ִ��
//�������:none
//����ֵ  :none
/**********************************************/
void NIX_RootTask(void)
{
    NIX_TASKOPT strOption;

    DEV_SoftwareInit();

    DEV_HardwareInit();

    strOption.ucTaskSta = TASKREADY;
    (void)NIX_TaskCreat("Task1",TEST_TestTask1, NULL, NULL, TASKSTACK, 5,
                        &strOption);

    (void)NIX_TaskCreat("Task2",TEST_TestTask2, NULL, NULL, TASKSTACK, 4, NULL);

    strOption.ucTaskSta = TASKDELAY;
    strOption.uiDelayTick = 2000;
    (void)NIX_TaskCreat("Task3",TEST_TestTask3, NULL, NULL, TASKSTACK, 3,
                        &strOption);

    strOption.ucTaskSta = TASKDELAY;
    strOption.uiDelayTick = DELAYWAITFEV;
    gpstrTask4Tcb = NIX_TaskCreat("Task4",TEST_TestTask4, NULL, NULL, TASKSTACK,
                                    2, &strOption);

    gpstrSerialTaskTcb = NIX_TaskCreat("SrlPrt",TEST_SerialPrintTask, NULL, NULL, TASKSTACK, 6,
                            NULL);

    DEV_PutStrToMem((U8*)"\r\nNIX is running! Tick is: %d", NIX_GetSystemTick());

    (void)NIX_TaskDelay(1000);

}


/**********************************************/
//��������:��������
//�������:pvPara:������ڲ���ָ��
//����ֵ  :none
/**********************************************/
void NIX_IdleTask(void* pvPara)
{
    while(1)
        {
            ;
    }
}

