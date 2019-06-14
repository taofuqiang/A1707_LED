/**
 * @file   ptpd_dep.h
 * 
 * @brief  External definitions for inclusion elsewhere.
 * 
 * 
 */

#ifndef PTPD_DEP_H_
#define PTPD_DEP_H_

#include "rtthread.h"
#include "param.h"

#ifdef __DEBUG__
#define PTPD_DBGV
#endif

#ifdef RUNTIME_DEBUG
#undef PTPD_DBGV
#define PTPD_DBGV
#endif

#ifdef DBG_SIGUSR2_CHANGE_DOMAIN
#ifdef DBG_SIGUSR2_CHANGE_DEBUG

#error "Cannot compile with both DBG_SIGUSR2_CHANGE_DOMAIN and DBG_SIGUSR2_CHANGE_DEBUG"

#endif
#endif




 /** \name System messages*/
 /**\{*/


// Syslog ordering. We define extra debug levels above LOG_DEBUG for internal use - but message() doesn't pass these to SysLog

// extended from <sys/syslog.h>
#define LOG_DEBUG1   7
#define LOG_DEBUG2   8
#define LOG_DEBUGV   9

#ifdef PTPD_DBGV

    #define MESSAGE_DEBUG(x, ...)   do{\
                                        if (x <= PTP_DebugLevel) \
                                        { \
                                            rt_kprintf("<Level%d> ", x); \
                                            rt_kprintf(##__VA_ARGS__); \
                                        } \
                                      }while(0)

    #define EMERGENCY(...)    MESSAGE_DEBUG(LOG_EMERG ,##__VA_ARGS__)
    #define ALERT(...)        MESSAGE_DEBUG(LOG_ALERT ,##__VA_ARGS__)
    #define CRITICAL(...)     MESSAGE_DEBUG(LOG_EMERG ,##__VA_ARGS__)
    #define ERROR(...)        MESSAGE_DEBUG(LOG_ERR ,##__VA_ARGS__)
    #define PERROR(...)       MESSAGE_DEBUG(LOG_ERR ,##__VA_ARGS__)
    #define WARNING(...)      MESSAGE_DEBUG(LOG_WARNING ,##__VA_ARGS__)
    #define NOTIFY(...)       MESSAGE_DEBUG(LOG_NOTICE ,##__VA_ARGS__)
    #define NOTICE(...)       MESSAGE_DEBUG(LOG_NOTICE ,##__VA_ARGS__)
    #define INFO(...)         MESSAGE_DEBUG(LOG_INFO ,##__VA_ARGS__)
#else
    #define EMERGENCY(...)    
    #define ALERT(...)        
    #define CRITICAL(...)     
    #define ERROR(...)        
    #define PERROR(...)       
    #define WARNING(...)     
    #define NOTIFY(...)       
    #define NOTICE(...)      
    #define INFO(...)         
#endif

#include <assert.h>


/*
  list of per-module defines:

./dep/sys.c:#define PRINT_MAC_ADDRESSES
./dep/timer.c:#define US_TIMER_INTERVAL 125000
*/
#define USE_BINDTODEVICE



// enable this line to show debug numbers in nanoseconds instead of microseconds 
// #define DEBUG_IN_NS

#define DBG_UNIT_US (1000)
#define DBG_UNIT_NS (1)

#ifdef DEBUG_IN_NS
#define DBG_UNIT DBG_UNIT_NS
#else
#define DBG_UNIT DBG_UNIT_US
#endif




/** \}*/

/** \name Debug messages*/
 /**\{*/

#ifdef PTPD_DBGV
#undef PTPD_DBG
#undef PTPD_DBG2
#define PTPD_DBG
#define PTPD_DBG2

#define DBGV(...) MESSAGE_DEBUG(LOG_DEBUGV ,##__VA_ARGS__)
#else
#define DBGV(...)    
#endif

/*
 * new debug level DBG2:
 * this is above DBG(), but below DBGV() (to avoid changing hundreds of lines)
 */


#ifdef PTPD_DBG2
#undef PTPD_DBG
#define PTPD_DBG
#define DBG2(...) MESSAGE_DEBUG(LOG_DEBUG2 ,##__VA_ARGS__)
#else
#define DBG2(...) 
#endif

#ifdef PTPD_DBG
#define DBG(...) MESSAGE_DEBUG(LOG_DEBUG1 ,##__VA_ARGS__)
#else
#define DBG(...) 
#endif

/** \}*/

/** \name Endian corrections*/
 /**\{*/

#if defined(PTPD_MSBF)
#define shift8(x,y)   ( (x) << ((3-y)<<3) )
#define shift16(x,y)  ( (x) << ((1-y)<<4) )
#elif defined(PTPD_LSBF)
#define shift8(x,y)   ( (x) << ((y)<<3) )
#define shift16(x,y)  ( (x) << ((y)<<4) )
#endif

#define flip16(x) htons(x)
#define flip32(x) htonl(x)

/* i don't know any target platforms that do not have htons and htonl,
   but here are generic funtions just in case */
/*
#if defined(PTPD_MSBF)
#define flip16(x) (x)
#define flip32(x) (x)
#elif defined(PTPD_LSBF)
static inline Integer16 flip16(Integer16 x)
{
   return (((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00);
}

static inline Integer32 flip32(x)
{
  return (((x) >> 24) & 0x000000ff) | (((x) >> 8 ) & 0x0000ff00) |
         (((x) << 8 ) & 0x00ff0000) | (((x) << 24) & 0xff000000);
}
#endif
*/

/** \}*/


/** \name Bit array manipulations*/
 /**\{*/

#define getFlag(x,y)  !!( *(UInteger8*)((x)+((y)<8?1:0)) &   (1<<((y)<8?(y):(y)-8)) )
#define setFlag(x,y)    ( *(UInteger8*)((x)+((y)<8?1:0)) |=   1<<((y)<8?(y):(y)-8)  )
#define clearFlag(x,y)  ( *(UInteger8*)((x)+((y)<8?1:0)) &= ~(1<<((y)<8?(y):(y)-8)) )
/** \}*/

/** \name msg.c
 *-Pack and unpack PTP messages */
 /**\{*/

void msgUnpackHeader(Octet * buf,MsgHeader*);
void msgUnpackAnnounce (Octet * buf,MsgAnnounce*);
void msgUnpackSync(Octet * buf,MsgSync*);
void msgUnpackFollowUp(Octet * buf,MsgFollowUp*);
void msgUnpackDelayReq(Octet * buf, MsgDelayReq * delayreq);
void msgUnpackDelayResp(Octet * buf,MsgDelayResp *);
void msgUnpackPDelayReq(Octet * buf,MsgPDelayReq*);
void msgUnpackPDelayResp(Octet * buf,MsgPDelayResp*);
void msgUnpackPDelayRespFollowUp(Octet * buf,MsgPDelayRespFollowUp*);
void msgUnpackManagement(Octet * buf,MsgManagement*, MsgHeader*, PtpClock *ptpClock);
void msgPackHeader(Octet * buf,PtpClock*);
void msgPackAnnounce(Octet * buf,PtpClock*);
void msgPackSync(Octet * buf,Timestamp*,PtpClock*);
void msgPackFollowUp(Octet * buf,Timestamp*,PtpClock*);
void msgPackDelayReq(Octet * buf, Timestamp *, RunTimeOpts *, PtpClock *);
void msgPackDelayResp(Octet * buf, MsgHeader *, Timestamp *, RunTimeOpts *, PtpClock *);
void msgPackPDelayReq(Octet * buf,Timestamp*,PtpClock*);
void msgPackPDelayResp(Octet * buf,MsgHeader*,Timestamp*,PtpClock*);
void msgPackPDelayRespFollowUp(Octet * buf,MsgHeader*,Timestamp*,PtpClock*);


void copyClockIdentity( ClockIdentity dest, ClockIdentity src);
void copyPortIdentity( PortIdentity * dest, PortIdentity * src);




/** \}*/

/** \name net.c (Unix API dependent)
 * -Init network stuff, send and receive datas*/
 /**\{*/

Boolean netInit(NetPath*,RunTimeOpts*,PtpClock*);
Boolean netShutdown(NetPath*,RunTimeOpts*,PtpClock*);
int netSelect(TimeInternal*,NetPath*);
rt_int32_t netRecvEvent(Octet*,TimeInternal*,NetPath*);
rt_int32_t netRecvGeneral(Octet*,NetPath*);
rt_int32_t netSendEvent(Octet*,UInteger16,TimeInternal*,NetPath*, Integer32 );
rt_int32_t netSendGeneral(Octet*,UInteger16,NetPath*, Integer32 );
rt_int32_t netSendPeerGeneral(Octet*,UInteger16,NetPath*);
rt_int32_t netSendPeerEvent(Octet*,UInteger16,TimeInternal*,NetPath*);

Boolean netRefreshIGMP(NetPath *, RunTimeOpts *, PtpClock *);


/** \}*/

#if defined PTPD_SNMP
/** \name snmp.c (SNMP subsystem)
 * -Handle SNMP subsystem*/
 /**\{*/

void snmpInit(PtpClock *);

/** \}*/
#endif

/** \name servo.c
 * -Clock servo*/
 /**\{*/

void initClock(RunTimeOpts*,PtpClock*);
void updatePeerDelay (one_way_delay_filter*, RunTimeOpts*,PtpClock*,TimeInternal*,Boolean);
void updateDelay (one_way_delay_filter*, RunTimeOpts*, PtpClock*,TimeInternal*);
void updateOffset(TimeInternal*,TimeInternal*,
  offset_from_master_filter*,RunTimeOpts*,PtpClock*,TimeInternal*);
void updateClock(RunTimeOpts*,PtpClock*);

void servo_perform_clock_step(RunTimeOpts * rtOpts, PtpClock * ptpClock);

/** \}*/

/** \name startup.c (Unix API dependent)
 * -Handle with runtime options*/
 /**\{*/
int logToFile(RunTimeOpts * rtOpts);
int recordToFile(RunTimeOpts * rtOpts);
PtpClock * ptpdStartup(PtpClock * ptp_Clock, RunTimeOpts * rt_Opts, ForeignMasterRecord *foreign);
void ptpdShutdown(RunTimeOpts * rtopts, PtpClock * ptpClock);

void check_signals(RunTimeOpts * rtOpts, PtpClock * ptpClock);


/** \}*/

/** \name sys.c (Unix API dependent)
 * -Manage timing system API*/
 /**\{*/

/* new debug methods to debug time variables */
char *time2st(const TimeInternal * p);
void DBG_time(const char *name, const TimeInternal  p);


void displayStats(RunTimeOpts *rtOpts, PtpClock *ptpClock);
void displayStatus(PtpClock *ptpClock, const char *prefixMessage);
void displayPortIdentity(PortIdentity *port, const char *prefixMessage);
void increaseMaxDelayThreshold(RunTimeOpts * rtOpts);
void decreaseMaxDelayThreshold(RunTimeOpts * rtOpts);
Boolean nanoSleep(TimeInternal*);
void getTime(TimeInternal*);
void setTime(TimeInternal*);
double getRand(void);


Boolean adjTime(TimeInternal * time);
void PTP_handlePhyEvent(Octet *msgIbuf, rt_int32_t, RunTimeOpts *rtOpts, PtpClock *ptpClock);




Boolean adjFreq(Integer32);
void adjFreq_wrapper(RunTimeOpts * rtOpts, PtpClock * ptpClock, Integer32 adj);
Integer32 getAdjFreq(void);

/* Observed drift save / recovery functions */
void restoreDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet);
void saveDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet);

/* Helper function to manage ntpadjtime / adjtimex flags */
void setTimexFlags(int flags, Boolean quiet);
void unsetTimexFlags(int flags, Boolean quiet);
int getTimexFlags(void);
Boolean checkTimexFlags(int flags);

#if defined(MOD_TAI) &&  NTP_API == 4
void setKernelUtcOffset(int utc_offset);
#endif /* MOD_TAI */


/** \}*/

/** \name timer.c (Unix API dependent)
 * -Handle with timers*/
 /**\{*/
void initTimer(RunTimeOpts *rtOpts, PtpClock *ptpClock);
void timerUpdate(IntervalTimer*);
void timerStop(UInteger16,IntervalTimer*);
//void timerStart(UInteger16,UInteger16,IntervalTimer*);

/* R135 patch: we went back to floating point periods (for less than 1s )*/
void timerStart(UInteger16 index, Integer32 interval, IntervalTimer * itimer);
void timerRestart(UInteger16 index, Integer32 interval, IntervalTimer * itimer);

/* Version with randomized backoff */
void timerStartRandom(UInteger16 index, Integer32 interval, IntervalTimer * itimer);

Boolean timerExpired(UInteger16,IntervalTimer*);
void signalAction(int value);
int waitSignal(void);
/** \}*/


const char * translateLogPriority(int priority);

/*Test functions*/
void
reset_operator_messages(RunTimeOpts * rtOpts, PtpClock * ptpClock);

void callback_sync(void);
void callback_event(TimeInternal * time);
void callback_delay(void);
void callback_listening(void);


/* pow2ms(a) = round(pow(2,a)*1000) */

#define pow2ms(a) (((a)>0) ? (1000 << (a)) : (1000 >>(-(a))))

#endif /*PTPD_DEP_H_*/
