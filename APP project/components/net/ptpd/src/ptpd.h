/**
 * @file   ptpd.h
 * @mainpage Ptpd v2 Documentation
 * @authors Martin Burnicki, Alexandre van Kempen, Steven Kreuzer, 
 *          George Neville-Neil
 * @version 2.0
 * @date   Fri Aug 27 10:22:19 2010
 * 
 * @section implementation Implementation
 * PTTdV2 is not a full implementation of 1588 - 2008 standard.
 * It is implemented only with use of Transparent Clock and Peer delay
 * mechanism, according to 802.1AS requierements.
 * 
 * This header file includes all others headers.
 * It defines functions which are not dependant of the operating system.
 */

#ifndef PTPD_H_
#define PTPD_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "dep/constants_dep.h"
#include "dep/datatypes_dep.h"
#include "datatypes.h"
#include "dep/ptpd_dep.h"
#include "rtthread.h"

extern PtpClock *G_ptpClock;
extern RunTimeOpts * G_rtOpts;

/* NOTE: this macro can be refactored into a function */
#define XMALLOC(ptr,size) \
    do \
    { \
        (ptr)=rt_malloc(size); \
        if(!(ptr)) { \
            PERROR("failed to allocate memory"); \
            ptpdShutdown(G_rtOpts, G_ptpClock); \
            return; \
            }\
    }while(0);
	

#define IS_SET(data, bitpos) \
	((data & ( 0x1 << bitpos )) == (0x1 << bitpos))

#define SET_FIELD(data, bitpos) \
	(data << bitpos)

#define INT_MAX                      (2147483647)

#define min(a,b)     (((a)<(b))?(a):(b))
#define max(a,b)     (((a)>(b))?(a):(b))
#define Abs(a)       ((a) > 0 ? (a) : -(a))

void ptpd_server_init(void);
void ptpd_thread_entry(void * args);

void ptpdLoadOpts(RunTimeOpts * rtOpts);
void ptpdLoadDefaultOpts(RunTimeOpts * rtOpts);


/** \name arith.c
 * -Timing management and arithmetic*/
 /**\{*/
/* arith.c */

/**
 * \brief Convert Integer64 into TimeInternal structure
 */
void integer64_to_internalTime(Integer64,TimeInternal*);
/**
 * \brief Convert TimeInternal structure to Integer64
 */
void internalTime_to_integer64(TimeInternal, Integer64*);
/**
 * \brief Convert TimeInternal into Timestamp structure (defined by the spec)
 */
void fromInternalTime(const TimeInternal*,Timestamp*);

/**
 * \brief Convert Timestamp to TimeInternal structure (defined by the spec)
 */
void toInternalTime(TimeInternal*, const Timestamp*);

void ts_to_InternalTime(const struct ptptime_t *, TimeInternal *);
void tv_to_InternalTime(const struct timeval  *, TimeInternal *);




/**
 * \brief Use to normalize a TimeInternal structure
 *
 * The nanosecondsField member must always be less than 10⁹
 * This function is used after adding or substracting TimeInternal
 */
void normalizeTime(TimeInternal*);

/**
 * \brief Add two InternalTime structure and normalize
 */
void addTime(TimeInternal*,const TimeInternal*,const TimeInternal*);

/**
 * \brief Substract two InternalTime structure and normalize
 */
void subTime(TimeInternal*,const TimeInternal*,const TimeInternal*);
/** \}*/

/**
 * \brief Divied an InternalTime by 2
 */
void div2Time(TimeInternal *);

/** \name bmc.c
 * -Best Master Clock Algorithm functions*/
 /**\{*/
/* bmc.c */
/**
 * \brief Compare data set of foreign masters and local data set
 * \return The recommended state for the port
 */
UInteger8 bmc(ForeignMasterRecord*, const RunTimeOpts*,PtpClock*);

/**
 * \brief When recommended state is Master, copy local data into parent and grandmaster dataset
 */
void m1(const RunTimeOpts *, PtpClock*);

/**
 * \brief When recommended state is Slave, copy dataset of master into parent and grandmaster dataset
 */
void s1(MsgHeader*,MsgAnnounce*,PtpClock*, const RunTimeOpts *);


void p1(PtpClock *ptpClock, const RunTimeOpts *rtOpts);

/**
 * \brief Initialize datas
 */
void initData(RunTimeOpts*,PtpClock*);
/** \}*/


/** \name protocol.c
 * -Execute the protocol engine*/
 /**\{*/
/**
 * \brief Protocol engine
 */
/* protocol.c */
void protocol(RunTimeOpts*,PtpClock*);


const char *portState_getName(Enumeration8 portState);



void clearTime(TimeInternal *time);

char *dump_TimeInternal(const TimeInternal * p);
char *dump_TimeInternal2(const char *st1, const TimeInternal * p1, const char *st2, const TimeInternal * p2);



int snprint_TimeInternal(char *s, int max_len, const TimeInternal * p);


void nano_to_Time(TimeInternal *time, int nano);
int gtTime(const TimeInternal *x, const TimeInternal *b);
void absTime(TimeInternal *time);
int is_Time_close(const TimeInternal *x, const TimeInternal *b, int nanos);
int isTimeInternalNegative(const TimeInternal * p);
int isTimeInternalZero(const TimeInternal * p);



int check_timestamp_is_fresh2(const TimeInternal * timeA, const TimeInternal * timeB);
int check_timestamp_is_fresh(const TimeInternal * timeA);


void toState(UInteger8,RunTimeOpts*,PtpClock*);

/* helper functions for leap second handling */
float secondsToMidnight(void);
float getPauseAfterMidnight(Integer8 announceInterval);

Boolean respectUtcOffset(RunTimeOpts * rtOpts, PtpClock * ptpClock);

#endif /*PTPD_H_*/
