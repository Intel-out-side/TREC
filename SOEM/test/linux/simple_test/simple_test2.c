#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

#define EC_TIMEOUTMON 500

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

void simpletest(char *ifname) {
  //int i, j, oloop, iloop, chk;
  int oloop, iloop;
  needlf = FALSE;
  inOP = FALSE;

  if (ec_init(ifname)) {
    printf("ec_init on %s succeeded.\n",ifname);
    /* find and auto-config slaves */

    if (ec_config_init(FALSE) > 0) {
      printf("%d slaves found and configured. \n", ec_slavecount);

      ec_config_map(&IOmap);
      //Map all PDOs from slaves to IOmap with Outputs/Inputs in sequential order (legacy SOEM way).

      ec_configdc();
      //no available description

      printf("slaves are mapped. state to SAFE_OP");

      ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
      //全スレーブをSAFE_OPモードにする？

      //output loop ?
      oloop = ec_slave[0].Obytes;
      if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
      if (oloop > 8) oloop = 8;

      //input loop ?
      iloop = ec_slave[0].Ibytes;
      if ((iloop == 0) && (ec_slave[0].Ibytes > 0)) iloop = 1;
      if (iloop > 8) iloop = 8;
      //printf(ec_slave[0].Obytes);
      //ec_slave :: Main slave data array. Each slave found on the network gets its own record. ec_slave[0] is reserved for the master.
      //Structure gets filled in by the configuration function ec_config().

      printf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);
      //ec_group :: ec slave structure　とだけ書いてある

      printf("Request operational states for all the slaves\n");
      expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
      ec_slave[0].state = EC_STATE_OPERATIONAL;

      //send a process by the function below
      ec_send_processdata();
      ec_receive_processdata(EC_TIMEOUTRET);
      ec_writestate(0);
      //chk = 200;

    }
  }
}

//ここはよくわからんのでコピペです
OSAL_THREAD_FUNC ecatcheck( void *ptr )
{
    int slave;
    (void)ptr;                  /* Not used */

    while(1)
    {
        if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
               needlf = FALSE;
               printf("\n");
            }
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();
            for (slave = 1; slave <= ec_slavecount; slave++)
            {
               if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
               {
                  ec_group[currentgroup].docheckstate = TRUE;
                  if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                  {
                     printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                     ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                     ec_writestate(slave);
                  }
                  else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
                  {
                     printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                     ec_slave[slave].state = EC_STATE_OPERATIONAL;
                     ec_writestate(slave);
                  }
                  else if(ec_slave[slave].state > EC_STATE_NONE)
                  {
                     if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                     {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d reconfigured\n",slave);
                     }
                  }
                  else if(!ec_slave[slave].islost)
                  {
                     /* re-check state */
                     ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                     if (ec_slave[slave].state == EC_STATE_NONE)
                     {
                        ec_slave[slave].islost = TRUE;
                        printf("ERROR : slave %d lost\n",slave);
                     }
                  }
               }
               if (ec_slave[slave].islost)
               {
                  if(ec_slave[slave].state == EC_STATE_NONE)
                  {
                     if (ec_recover_slave(slave, EC_TIMEOUTMON))
                     {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d recovered\n",slave);
                     }
                  }
                  else
                  {
                     ec_slave[slave].islost = FALSE;
                     printf("MESSAGE : slave %d found\n",slave);
                  }
               }
            }
            if(!ec_group[currentgroup].docheckstate)
               printf("OK : all slaves resumed OPERATIONAL.\n");
        }
        osal_usleep(10000);
    }
}

int main(int argc, char *argv[]) {
  if (argc == 0) return 1;

  osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);

  simpletest(argv[1]);
  //argv[1] is ethercat socket (slave?) name
  return 0;
}
