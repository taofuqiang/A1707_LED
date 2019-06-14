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
 * @file   sys.c
 * @date   Tue Jul 20 16:19:46 2010
 *
 * @brief  Code to call kernel time routines and also display server statistics.
 *
 *
 */

#include <inttypes.h>

#include "../ptpd.h"
#include "stm32_eth.h"

/*
 displays 2 timestamps and their strings in sequence, and the difference between then
 DO NOT call this twice in the same printf!
*/
#if 1
char *dump_TimeInternal2(const char *st1, const TimeInternal * p1, const char *st2, const TimeInternal * p2)
{
    /* display difference */
	TimeInternal r;
	static char buf[BUF_SIZE];
	int n = 0;

	/* display Timestamps */
	if (st1) {
		n += snprintf(buf + n, BUF_SIZE - n, "%s ", st1);
	}
	n += snprint_TimeInternal(buf + n, BUF_SIZE - n, p1);
	n += snprintf(buf + n, BUF_SIZE - n, "    ");

	if (st2) {
		n += snprintf(buf + n, BUF_SIZE - n, "%s ", st2);
	}
	n += snprint_TimeInternal(buf + n, BUF_SIZE - n, p2);
	n += snprintf(buf + n, BUF_SIZE - n, " ");
	
	subTime(&r, p1, p2);
	n += snprintf(buf + n, BUF_SIZE - n, "   (diff: ");
	n += snprint_TimeInternal(buf + n, BUF_SIZE - n, &r);
	n += snprintf(buf + n, BUF_SIZE - n, ") ");

	return buf;
}
#endif


int 
snprint_TimeInternal(char *s, int max_len, const TimeInternal * p)
{
	int len = 0;

	/* always print either a space, or the leading "-". This makes the stat files columns-aligned */
	len += snprintf(&s[len], max_len - len, "%c",
		isTimeInternalNegative(p)? '-':' ');

	len += snprintf(&s[len], max_len - len, "%d.%09d",
	    abs(p->seconds), abs(p->nanoseconds));

	return len;
}

char *
translatePortState(PtpClock *ptpClock)
{
	char *s;
	switch(ptpClock->portState) {
	    case PTP_INITIALIZING:  s = "init";  break;
	    case PTP_FAULTY:        s = "flt";   break;
	    case PTP_LISTENING:
		    /* seperate init-reset from real resets */
		    if(ptpClock->reset_count == 1){
		    	s = "lstn_init";
		    } else {
		    	s = "lstn_reset";
		    }
		    break;
	    case PTP_PASSIVE:       s = "pass";  break;
	    case PTP_UNCALIBRATED:  s = "uncl";  break;
	    case PTP_SLAVE:         s = "slv";   break;
	    case PTP_PRE_MASTER:    s = "pmst";  break;
	    case PTP_MASTER:        s = "mst";   break;
	    case PTP_DISABLED:      s = "dsbl";  break;
	    default:                s = "?";     break;
	}
	return s;
}


int 
snprint_ClockIdentity(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i;

	for (i = 0; ;) {
		len += snprintf(&s[len], max_len - len, "%02x", (unsigned char) id[i]);

		if (++i >= CLOCK_IDENTITY_LENGTH)
			break;
	}

	return len;
}


/* show the mac address in an easy way */
int
snprint_ClockIdentity_mac(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i;

	for (i = 0; ;) {
		/* skip bytes 3 and 4 */
		if(!((i==3) || (i==4))){
			len += snprintf(&s[len], max_len - len, "%02x", (unsigned char) id[i]);

			if (++i >= CLOCK_IDENTITY_LENGTH)
				break;

			/* print a separator after each byte except the last one */
			len += snprintf(&s[len], max_len - len, "%s", ":");
		} else {

			i++;
		}
	}

	return len;
}


/*
 * wrapper that caches the latest value of ether_ntohost
 * this function will NOT check the last accces time of /etc/ethers,
 * so it only have different output on a failover or at restart
 *
 */
int ether_ntohost_cache(char *hostname, struct ether_addr *addr)
{
	static int valid = 0;
	static struct ether_addr prev_addr;
	static char buf[BUF_SIZE];

#if defined(linux) || defined(__NetBSD__)
	if (memcmp(addr->ether_addr_octet, &prev_addr, 
		  sizeof(struct ether_addr )) != 0) {
		valid = 0;
	}
#else // e.g. defined(__FreeBSD__)
	if (memcmp(addr->octet, &prev_addr, 
		  sizeof(struct ether_addr )) != 0) {
		valid = 0;
	}
#endif

	if (!valid) {
//		if(ether_ntohost(buf, addr)){
			sprintf(buf, "%s", "unknown");
//		}

		/* clean possible commas from the string */
		while (strchr(buf, ',') != NULL) {
			*(strchr(buf, ',')) = '_';
		}

		prev_addr = *addr;
	}

	valid = 1;
	strcpy(hostname, buf);
	return 0;
}


/* Show the hostname configured in /etc/ethers */
int
snprint_ClockIdentity_ntohost(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i,j;
	char  buf[100];
	struct ether_addr e;

	/* extract mac address */
	for (i = 0, j = 0; i< CLOCK_IDENTITY_LENGTH ; i++ ){
		/* skip bytes 3 and 4 */
		if(!((i==3) || (i==4))){
#if defined(linux) || defined(__NetBSD__)
			e.ether_addr_octet[j] = (uint8_t) id[i];
#else // e.g. defined(__FreeBSD__)
			e.octet[j] = (uint8_t) id[i];
#endif
			j++;
		}
	}

	/* convert and print hostname */
	ether_ntohost_cache(buf, &e);
	len += snprintf(&s[len], max_len - len, "(%s)", buf);

	return len;
}


int 
snprint_PortIdentity(char *s, int max_len, const PortIdentity *id)
{
	int len = 0;

#ifdef PRINT_MAC_ADDRESSES
	len += snprint_ClockIdentity_mac(&s[len], max_len - len, id->clockIdentity);
#else	
	len += snprint_ClockIdentity(&s[len], max_len - len, id->clockIdentity);
#endif

	len += snprint_ClockIdentity_ntohost(&s[len], max_len - len, id->clockIdentity);

	len += snprintf(&s[len], max_len - len, "/%02x", (unsigned) id->portNumber);
	return len;
}

/**\convert port state to string*/
const char
*portState_getName(Enumeration8 portState)
{
    static const char *ptpStates[PTP_DESC_TOTAL];
    /* converting to int to avoid compiler warnings when comparing enum*/
    static const int max = PTP_SLAVE;
    int intstate = portState;

    ptpStates[PTP_INITIALIZING] = "PTP_INITIALIZING";
    ptpStates[PTP_FAULTY] = "PTP_FAULTY";
    ptpStates[PTP_DISABLED] = "PTP_DISABLED";
    ptpStates[PTP_LISTENING] = "PTP_LISTENING";
    ptpStates[PTP_PRE_MASTER] = "PTP_PRE_MASTER";
    ptpStates[PTP_MASTER] = "PTP_MASTER";
    ptpStates[PTP_PASSIVE] = "PTP_PASSIVE";
    ptpStates[PTP_UNCALIBRATED] = "PTP_UNCALIBRATED";
    ptpStates[PTP_SLAVE] = "PTP_SLAVE";

    if( intstate < 0 || intstate > max ) {
        return("PTP_UNKNOWN");
    }

    return(ptpStates[portState]);

}

void displayStats(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
    (void)rtOpts;
    (void)ptpClock;
    DBGV("Stats:%s\n", portState_getName(ptpClock->portState));
}

void 
displayStatus(PtpClock *ptpClock, const char *prefixMessage) 
{

	static char sbuf[SCREEN_BUFSZ];
	int len = 0;

	memset(sbuf, ' ', sizeof(sbuf));
	len += snprintf(sbuf + len, sizeof(sbuf) - len, INFO_PREFIX "%s", prefixMessage);
	len += snprintf(sbuf + len, sizeof(sbuf) - len, "%s", 
			portState_getName(ptpClock->portState));

	if (ptpClock->portState == PTP_SLAVE ||
	    ptpClock->portState == PTP_MASTER ||
	    ptpClock->portState == PTP_PASSIVE) {
		len += snprintf(sbuf + len, sizeof(sbuf) - len, ", Best master: ");
		len += snprint_PortIdentity(sbuf + len, sizeof(sbuf) - len,
			&ptpClock->foreign[ptpClock->foreign_record_best].header.sourcePortIdentity);
        }

        len += snprintf(sbuf + len, sizeof(sbuf) - len, "\n");
        INFO("%s",sbuf);
}

void 
displayPortIdentity(PortIdentity *port, const char *prefixMessage) 
{
	static char sbuf[SCREEN_BUFSZ];
	int len = 0;

	memset(sbuf, ' ', sizeof(sbuf));
	len += snprintf(sbuf + len, sizeof(sbuf) - len, INFO_PREFIX "%s ", prefixMessage);
	len += snprint_PortIdentity(sbuf + len, sizeof(sbuf) - len, port);
        len += snprintf(sbuf + len, sizeof(sbuf) - len, "\n");
        INFO("%s",sbuf);
}


void getTime(TimeInternal * time)
{
#ifdef LWIP_PTP
    ETH_PTPTime_GetTime((struct ptptime_t *)time);
#endif
}

void setTime(TimeInternal * time)
{
#ifdef LWIP_PTP
    ETH_PTPTime_SetTime((struct ptptime_t *)time);
#endif
    DBG("setTime: resetting system clock to %d.%09ds\n", time->seconds, time->nanoseconds);
}


/* returns a double beween 0.0 and 1.0 */
double 
getRand(void)
{
	return ((rand() * 1.0) / RAND_MAX);
}





/*
 * TODO: this function should have been coded in a way to manipulate both the frequency and the tick,
 * to avoid having to call setTime() when the clock is very far away.
 * This would result in situations we would force the kernel clock to run the clock twice as slow,
 * in order to avoid stepping time backwards
 */
Boolean
adjFreq(Integer32 adj)
{
    DBGV("adjFreq %d\n", adj);

    if (adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if (adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;
#ifdef LWIP_PTP
    /* Fine update method */
    ETH_PTPTime_AdjFreq(adj);
#endif
    return TRUE;
}


Integer32
getAdjFreq(void)
{
	return 0;
}

void restoreDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet)
{
	Boolean reset_offset = FALSE;

	DBGV("restoreDrift called\n");

	if (ptpClock->drift_saved && rtOpts->drift_recovery_method > 0 ) 
    {
		ptpClock->observed_drift = ptpClock->last_saved_drift;
		if (!rtOpts->noAdjust) 
        {
			adjFreq_wrapper(rtOpts, ptpClock, -ptpClock->last_saved_drift);
		}
		DBG("loaded cached drift");
		return;
	}

	switch (rtOpts->drift_recovery_method) 
    {

		case DRIFT_KERNEL:
		    break;
		default:
			reset_offset = TRUE;
            break;

	}

	if (reset_offset) 
    {
		if (!rtOpts->noAdjust)

		  adjFreq_wrapper(rtOpts, ptpClock, 0);

		ptpClock->observed_drift = 0;
		return;
	}
}

void saveDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet)
{
        (void)quiet;
        DBGV("saveDrift called\n");

        if (rtOpts->drift_recovery_method > 0) 
        {
                ptpClock->last_saved_drift = ptpClock->observed_drift;
                ptpClock->drift_saved = TRUE;
        }  
}

void
setTimexFlags(int flags, Boolean quiet)
{
        (void)flags;
        (void)quiet;
}

void
unsetTimexFlags(int flags, Boolean quiet) 
{
        (void)flags;
        (void)quiet;
}

int getTimexFlags(void)
{
        return 0;
}

Boolean
checkTimexFlags(int flags) {
        (void)flags;
        return FALSE;
}

Boolean adjTime(TimeInternal * time) 
{

#ifdef LWIP_PTP
    struct ptptime_t timeoffset;

    DBGV("adjTime: %ds %dns\n", time->seconds, time->nanoseconds);

    timeoffset.tv_sec = -time->seconds;
    timeoffset.tv_nsec = -time->nanoseconds;

    /* Coarse update method */
    ETH_PTPTime_UpdateOffset(&timeoffset);
#endif
    DBGV("updateTime: updated\n");
    return TRUE;
}


