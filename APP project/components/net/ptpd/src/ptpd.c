/*-
 * Copyright (c) 2011-2012 George V. Neville-Neil,
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen,
 *                         Inaqui Delgado,
 *                         Rick Ratzel,
 *                         National Instruments.
 * Copyright (c) 2009-2010 George V. Neville-Neil, 
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen
 *
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   ptpd.c
 * @date   Wed Jun 23 10:13:38 2010
 *
 * @brief  The main() function for the PTP daemon
 *
 * This file contains very little code, as should be obvious,
 * and only serves to tie together the rest of the daemon.
 * All of the default options are set here, but command line
 * arguments are processed in the ptpdStartup() routine called
 * below.
 */

#include "ptpd.h"
#include "param.h"

RunTimeOpts priv_rtOpts; /* statically allocated run-time
					 * configuration data */
PtpClock priv_ptpClock;

ForeignMasterRecord priv_foreign[DEFAULT_MAX_FOREIGN_RECORDS];

/*
 * Global variable with the main PTP port. This is used to show the current state in DBG()/message()
 * without having to pass the pointer everytime.
 *
 * if ptpd is extended to handle multiple ports (eg, to instantiate a Boundary Clock),
 * then DBG()/message() needs a per-port pointer argument
 */
PtpClock *G_ptpClock = NULL;
RunTimeOpts * G_rtOpts = NULL;

#define PTPD_THREAD_PRIO        ( 10 )
#define PTPD_THREAD_STACKSIZE   (1024)
/*PTPd服务线程*/
void ptpd_server_init(void) 
{
  sys_thread_new("ptpd", ptpd_thread_entry, NULL, PTPD_THREAD_STACKSIZE, PTPD_THREAD_PRIO);
}

void ptpdLoadDefaultOpts(RunTimeOpts * rtOpts) 
{
    /* initialize run-time options to default values */
    rtOpts->announceInterval = DEFAULT_ANNOUNCE_INTERVAL; 
    rtOpts->syncInterval = DEFAULT_SYNC_INTERVAL;
    rtOpts->clockQuality.clockAccuracy = DEFAULT_CLOCK_ACCURACY;
    rtOpts->clockQuality.clockClass = DEFAULT_CLOCK_CLASS;
    rtOpts->clockQuality.offsetScaledLogVariance = DEFAULT_CLOCK_VARIANCE;
    rtOpts->priority1 = (UInteger8)PTP_Priority1;//DEFAULT_PRIORITY1;
    rtOpts->priority2 = (UInteger8)PTP_Priority2;//DEFAULT_PRIORITY2;
    rtOpts->domainNumber = (UInteger8)PTP_Domain_Num;//DEFAULT_DOMAIN_NUMBER;
#ifdef PTPD_EXPERIMENTAL
    rtOpts->mcast_group_Number = 0;
    rtOpts->do_hybrid_mode = FALSE;
#endif

    rtOpts->currentUtcOffset = DEFAULT_UTC_OFFSET;
    rtOpts->currentUtcOffsetValid = DEFAULT_UTC_VALID;
    rtOpts->leap59 = FALSE;
    rtOpts->leap61 = FALSE;
    rtOpts->timeTraceable = FALSE;/* time derived from atomic clock? */
    rtOpts->frequencyTraceable = FALSE;/* frequency derived from frequency standard? */
    rtOpts->ptpTimescale = FALSE;
    rtOpts->timeSource = DEFAULT_TIME_SOURCE;
    rtOpts->ifaceName[0] = '\0';
#ifdef PTPD_UNICAST_MODE
    rtOpts->do_unicast_mode = 0;
#endif
    rtOpts->twoStepFlag = TRUE;
    rtOpts->noAdjust = NO_ADJUST; //TRUE; //NO_ADJUST; // false
    // rtOpts->unicastAddress
    rtOpts->ap = DEFAULT_AP;
    rtOpts->ai = DEFAULT_AI;
    rtOpts->s = DEFAULT_DELAY_S;
    rtOpts->inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
    rtOpts->outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
    rtOpts->max_foreign_records = DEFAULT_MAX_FOREIGN_RECORDS;
    // rtOpts->ethernet_mode = FALSE;
    // rtOpts->offset_first_updated = FALSE;
    // rtOpts->file[0] = 0;

    
    rtOpts->displayStats = TRUE;    

    /*
     * defaults for new options
     */
    rtOpts->slaveOnly = (Boolean)PTP_Slave_Only;//FALSE;
    rtOpts->ignore_delayreq_interval_master = FALSE; //TRUE;
    rtOpts->do_IGMP_refresh = TRUE;

    rtOpts->announceReceiptTimeout = DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT; 
#ifdef RUNTIME_DEBUG
    rtOpts->debug_level = LOG_WARNING;
#endif

    rtOpts->ttl = DEFAULT_TTL; 
    rtOpts->delayMechanism = DEFAULT_DELAY_MECHANISM;
    rtOpts->noResetClock = DEFAULT_NO_RESET_CLOCK;
    rtOpts->log_seconds_between_message = 0;

    rtOpts->initial_delayreq = DEFAULT_DELAYREQ_INTERVAL; 
    rtOpts->subsequent_delayreq = DEFAULT_DELAYREQ_INTERVAL; 

    rtOpts->drift_recovery_method = DRIFT_RESET;
}

void ptpdLoadOpts(RunTimeOpts * rtOpts) 
{
    ptpdLoadDefaultOpts(rtOpts);
}

PtpClock * ptpdStartup(PtpClock * ptp_Clock, RunTimeOpts * rt_Opts, ForeignMasterRecord *foreign) 
{
    (void)rt_Opts;

    ptp_Clock->foreign = foreign;
    ptp_Clock->netPath.eventConn = NULL;
    ptp_Clock->netPath.generalConn = NULL;
    ptp_Clock->netPath.sendBuf = NULL;

    /* Init user_description */
    memset(ptp_Clock->user_description, 0, sizeof(ptp_Clock->user_description));
    memcpy(ptp_Clock->user_description, &USER_DESCRIPTION, sizeof(USER_DESCRIPTION));

    return ptp_Clock;
}

void ptpdShutdown(RunTimeOpts * rtopts, PtpClock * ptpClock) 
{
    netShutdown(&(ptpClock->netPath), rtopts, ptpClock);
}

void ptpd_thread_entry(void * args) 
{
    PtpClock *ptpClock;
    RunTimeOpts * rtOpts = &priv_rtOpts;
    G_rtOpts = rtOpts;

    (void) args;
    
    ptpdLoadOpts(rtOpts);
    
    /* Initialize run time options with command line arguments */
    ptpClock = ptpdStartup(&priv_ptpClock, rtOpts, priv_foreign);
    if (!ptpClock)
    {
        ERROR("PTPd Startup fail!\n");
        return;
    }
    rt_kprintf("ptpd server initialized!\n");
    /* global variable for message(), please see comment on top of this file */
    G_ptpClock = ptpClock;

    /* do the protocol engine */
    protocol(rtOpts, ptpClock);
    /* forever loop.. */

    ptpdShutdown(rtOpts, ptpClock);

    NOTIFY(INFO_PREFIX "Self shutdown, probably due to an error\n");

    return;
}

/***************************************************************************************************
*\Function      displayPTP
*\Description   显示ptp命令状态
*\Parameter     
*\Return        void
*\Note          
*\Log           2015.08.18    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void displayPTP(void)
{
    char buffer[80];
    const char *s;
    unsigned char * uuid;
    char sign;

    rt_kprintf("\n");
    uuid = (unsigned char*)G_ptpClock->parentPortIdentity.clockIdentity;
    /*master clock UUID*/
    rt_kprintf("master clock_uuid---->%02X%02X:%02X%02X:%02X%02X:%02X%02X\n",
        uuid[0], uuid[1],
        uuid[2], uuid[3],
        uuid[4], uuid[5],
        uuid[6], uuid[7]);


    switch (G_ptpClock->portState)
    {
    case PTP_INITIALIZING:  s = "init";  break;
    case PTP_FAULTY:        s = "faulty";   break;
    case PTP_LISTENING:     s = "listening";  break;
    case PTP_PASSIVE:       s = "passive";  break;
    case PTP_UNCALIBRATED:  s = "uncalibrated";  break;
    case PTP_SLAVE:         s = "slave";   break;
    case PTP_PRE_MASTER:    s = "pre master";  break;
    case PTP_MASTER:        s = "master";   break;
    case PTP_DISABLED:      s = "disabled";  break;
    default:                s = "?";     break;
    }

    /* state of the PTP */
    rt_kprintf("ptp_state ----> %s          \n", s);

    {
        TimeInternal  time;
        getTime(&time);
        rt_kprintf("ptp_stamp:% 10d.%09d    \n", time.seconds, time.nanoseconds);
    }
    /* one way delay */
    switch (G_ptpClock->delayMechanism)
    {
    case E2E:
        sprintf(buffer, "path delay: %dns          \n", G_ptpClock->meanPathDelay.nanoseconds);
        break;
    case P2P:
        sprintf(buffer, "path delay: %dns          \n", G_ptpClock->peerMeanPathDelay.nanoseconds);
        break;
    default:
        sprintf(buffer, "path delay: unknown       \n");
        /* none */
        break;
    }
    rt_kprintf("%s", buffer);

    /* offset from master */
    if (G_ptpClock->offsetFromMaster.seconds)
    {
        sprintf(buffer, "offset: %ds           \n", G_ptpClock->offsetFromMaster.seconds);
    }
    else
    {
        sprintf(buffer, "offset: %dns           \n", G_ptpClock->offsetFromMaster.nanoseconds);
    }
    rt_kprintf("%s", buffer);


    /* observed drift from master */
    sign = ' ';
    if (G_ptpClock->observed_drift > 0) sign = '+';
    if (G_ptpClock->observed_drift < 0) sign = '-';
    sprintf(buffer, "drift: %c%d.%03dppm       \n", sign, abs(G_ptpClock->observed_drift / 1000), abs(G_ptpClock->observed_drift % 1000));
    rt_kprintf("%s\n\n", buffer);     
} 

#if (defined LWIP_PTP && defined RT_USING_FINSH) 
#include <finsh.h>
FINSH_FUNCTION_EXPORT(displayPTP, display PTP info.);
#endif
