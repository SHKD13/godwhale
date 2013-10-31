/* $Id: pcommon3.h,v 1.4 2012/03/26 06:21:19 eikii Exp $ */

#ifndef PCOMMON_H
#define PCOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "if_bonanza.h"

/* ���̃t�@�C���ɂ�bonanza������Q�Ƃ��Ȃ��ϐ��A�֐��Ȃǂ�
 * ��`���܂��B���̂��� extern "C" �͎g�p���܂���B
 */

#define BNS_COMPAT    0
#define MASTERRANK    0

#define MAX_SRCH_DEP_FIGHT     30
#define MAX_SRCH_DEP_PROB       9
extern int MAX_SRCH_DEP;

#define NO_PENDING_REQUEST (-1)

#define DBG_INTERVAL_CHK  0

#undef max
#undef min

#define areaclr(x)  memset(&(x), 0, sizeof(x))
#define forr(i,m,n) for (int i=(m); i<=(n); i++)
#define forv(i,m,n) for (int i=(m); i>=(n); i--)
#define max(m,n)    ((m)>(n) ? (m) : (n))
#define min(m,n)    ((m)<(n) ? (m) : (n))
#define NULLMV      mvC((int)(0))

#define itdexd2srd(itd, exd) (2*(itd) - (exd))

#define TBC  assert(0)
#define NTBR assert(0)

#define DBG_SL_TRACE 1
#define DBG_MS_TRACE 1
#define DBG_SL       0
#define SLTOut(...) \
    do {if (DBG_SL_TRACE) out_file(slavelogfp, __VA_ARGS__); } while(0)
#define MSTOut(...) \
    do {if (DBG_MS_TRACE) out_file(masterlogfp, __VA_ARGS__); } while(0)
#define SLDOut(...) \
    do {if (DBG_SL      ) out_file(slavelogfp, __VA_ARGS__); } while(0)
#define MSDOut(...) \
    do {if (DBG_MASTER  ) out_file(masterlogfp, __VA_ARGS__); } while(0)
void out_file( FILE *pf, const char *format, ... );


// �]���`��
typedef enum
{
    VALTYPE_ALPHA,
    VALTYPE_BETA,
    VALTYPE_GAMMA
} ValueType;

enum
{
    ULE_NA = 0,
    ULE_UPPER = 2,
    ULE_LOWER = 1,
    ULE_EXACT = 3,
};


#if defined(_WIN32)
struct timespec
{
    int tv_sec;
    int tv_nsec;
};
#endif

class mvC {
public:
    int v;
    mvC(int x = 0) : v(x) {}
    bool operator ==(mvC x) { return (v==x.v); }
    bool operator !=(mvC x) { return (v!=x.v); }
};


#define THINK_TIME   sec_limit
#define BYOYOMI_TIME sec_limit_up

//extern int THINK_TIME, BYOYOMI_TIME;
extern int DBG_MASTER, VMMODE;
extern int x_dmy_for_calcinc, INCS_PER_USEC;
// �}�X�^�^�X���[�u�Ԃ̎���[10us]
extern int time_offset;
// �v���Z�X���Ƃ̃X���b�h��
extern int tlp_max_arg;

#define NOSIDE (-1)
// �����̎�Ԃ������܂�
extern int myTurn;

class planeC;
extern planeC plane;


 // defined in putils.cpp
extern void ei_clock_gettime(struct timespec* tsp);
extern int worldTime();
extern int64_t worldTimeLl();
extern void initTime();
extern void microsleep(int);
extern void microsleepMaster(int);
extern int readable_c(int mv);
extern int readable(mvC);

 // for invokempi.cpp
extern void sendQuit(int pr);

 // for 
extern void sendPacket(int dst, int* buf, int count); //FIXME unsigned
extern int recvPacket(int rank, int* v); // ditto
extern int probePacketSlave();
extern int probeProcessor();

 // for perfsl.h
extern int initPerf(tree_t * restrict ptree);
extern int displayPerf(tree_t * restrict ptree);

 // for slave
extern int inRoot, rootExceeded, inFirst, firstReplied;
extern uint64_t preNodeCount;

extern void replyFirst();

 // sbody.h
extern int problemMode();

#endif /* PCOMMON_H */
