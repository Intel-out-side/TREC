#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "ethercat.h"

static char IOmap[100]; // IOデータバッファ
static int expectedWKC; //期待されるワークカウンタ

//nif : ネットワークインターフェイスID
// return 0: failure, 1: success
int soem_open(char* nif) {
  int ret = ec_init(nif);
  return ret;
}

//通信を閉じる
void soem_close(void) {
  ec_close();
}

#define ALL_SLAVES_OP_STATE 0
#define NO_SLAVES_FOUND 1
#define NOT_ALL_OP_STATE 2


//return : 上の結果
int soem_config(void) {
  int oloop, iloop, chk;

  if (ec_config_init(FALSE) > 0) {
    printf("%d slaves found and configured.\n", ec_slavecount);

    ec_config_map(&IOmap);
    ec_configdc();

    printf("slaves mapped, state to SAFE_OP.\n");

    //すべてのスレーブがOP状態に達するのを待つ
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
    //ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE*4)

    oloop = ec_slave[0].Obytes; //ec_slave[0] is reserved for master
    if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
    if (oloop > 8) oloop = 8;

    iloop = ec_slave[0].Ibytes;
    if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
    if (iloop > 8) iloop = 8;

    printf("segments : %d : %d %d %d %d\n" ,
          ec_group[0].nsegments,
          ec_group[0].IOsegment[0],
          ec_group[0].IOsegment[1],
          ec_group[0].IOsegment[2],
          ec_group[0].IOsegment[3]);
    printf("Request operational state for all slaves\n");
    expectedWKC = (ec_group[0].outputsWKC * 2 + ec_group[0].inputsWKC);
    printf("Calculated workcounter %d\n", expectedWKC);

    //request operational state for all the slaves
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    ec_writestate(0);
    chk = 40;

    do {
      ec_send_processdata();
      ec_receive_processdata(EC_TIMEOUTRET);
      ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
    } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

    if (ec_slave[0].state == EC_STATE_OPERATIONAL) {
      return ALL_SLAVES_OP_STATE;
    }
    else {
      return NOT_ALL_OP_STATE;
    }
  }
  else {
    return NO_SLAVES_FOUND;
  }
}

// get the number of the slaves
int soem_getSlaveCount(void) {
  return ec_slavecount;
}

// update the state of the slaves
// return : the lowest state of the slave
int soem_updateState(void) {
  int ret = ec_readstate();
  return ret;
}

// get the state of the selected slave
// slave : incremental address of the slave
int soem_getState(int slave) {
  return ec_slave[slave].state;
}

// get the al statuscode of the slave
// slave : incremental address of the slave
int soem_getALStatusCode(int slave) {
  return ec_slave[slave].ALstatuscode;
}

// get the explanation of the al state code
// slave : the incremental address of the slave
// desc : description of the AL status code (31 chars at max)
void soem_getALStatusDesc(int slave, char* desc) {
  snprintf(desc, 31, "%s", ec_ALstatuscode2string( ec_slave[slave].ALstatuscode));
}

// request the updates of the slave states
void soem_requestState(int slave, int state) {
  ec_slave[slave].state = state;
  ec_writestate(slave);
}

// get the name of the slave
void soem_getName(int slave, char* name) {
  snprintf(name, 41, "%s", ec_slave[slave].name);
}

// slave's vendor number
// slave : incremental address of the slave
// id : {vendor number, product number, version number}
void soem_getId(int slave, unsigned long* id) {
  id[0] = ec_slave[slave].eep_man;
  id[1] = ec_slave[slave].eep_id;
  id[2] = ec_slave[slave].eep_rev;
}

// transfer through PDO
// return : 0->failed, 1->okay
int soem_transferPDO(void) {
  ec_send_processdata();
  int wkc = ec_receive_processdata(EC_TIMEOUTRET);

  if (wkc >= expectedWKC) {
    return 1;
  }
  else {
    return 0;
  }
}

// PDO input : slave -> master
// slave : incremental address of the slave
// offset : offset address of the slave
// reurn : input value
uint8_t soem_getInputPDO(int slave, int offset) {
  uint8_t ret = 0;
  if (slave <= ec_slavecount) {
    ret = ec_slave[slave].inputs[offset];
  }
  return ret;
}

// PDO output : Master -> Slave
// slave : incremental address of the slave
// offset : offset address of the slave
// value : output value
void soem_setOutPDO(int slave, int offset, uint8_t value) {
  if (slave <= ec_slavecount) {
    ec_slave[slave].outputs[offset] = value;
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    // network interface
    char *ifname = argv[1];

    // initialize SOEM
    int result = soem_open(ifname);
    if (result == 0) {
      printf("failed to open the network adapter\n");
      return 1;
    }
    result = soem_config();

    if (result == NO_SLAVES_FOUND) {
      printf("No slaves found!\n");
      return 2;
    }

    if (result == NOT_ALL_OP_STATE) {
      printf("There is at least one slave which is not operational");
      return 3;
    }

    uint8_t fx_ref = 100;
    uint8_t fy_ref = 101;
    uint8_t fz_ref = 102;
    uint8_t tx_ref = 103;
    uint8_t ty_ref = 104;
    uint8_t tz_ref = 105;


    uint8_t fx, fy, fz, tx, ty, tz;

    while (1) {

      //printf("master to slave output : %f %f %f %f %f %f \n", fx_ref, fy_ref, fz_ref, tx_ref, ty_ref, tz_ref);

      soem_setOutPDO(1, 0, fx_ref); //ここのインデックスはbyteじゃなくてbit?
      soem_setOutPDO(1, 1, fy_ref);
      soem_setOutPDO(1, 2, fz_ref);
      soem_setOutPDO(1, 3, tx_ref);
      soem_setOutPDO(1, 4, ty_ref);
      soem_setOutPDO(1, 5, tz_ref);

      soem_transferPDO();

      fx = soem_getInputPDO(1, 0);
      fy = soem_getInputPDO(1, 1);
      fz = soem_getInputPDO(1, 2);
      tx = soem_getInputPDO(1, 3);
      ty = soem_getInputPDO(1, 4);
      tz = soem_getInputPDO(1, 5);

      printf("slave to master input : %d %d %d %d %d %d\n", fx, fy, fz, tx, ty, tz);
    }

    soem_requestState(0, EC_STATE_INIT);
    soem_close();
  }
  else {
    ;
  }
  return 0;
}
