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
 * @file   protocol.c
 * @date   Wed Jun 23 09:40:39 2010
 * 
 * @brief  The code that handles the IEEE-1588 protocol and state machine
 * 
 * 
 */

#include "ptpd.h"

Boolean doInit(RunTimeOpts*,PtpClock*);
static void doState(RunTimeOpts*,PtpClock*);

void handle(RunTimeOpts*,PtpClock*);
static void handleAnnounce(MsgHeader*, rt_int32_t,Boolean, const RunTimeOpts*,PtpClock*);
static void handleSync(const MsgHeader*, rt_int32_t,TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handleFollowUp(const MsgHeader*, rt_int32_t,Boolean,RunTimeOpts*,PtpClock*);
static void handlePDelayReq(MsgHeader*, rt_int32_t,const TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handleDelayReq(const MsgHeader*, rt_int32_t, const TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handlePDelayResp(const MsgHeader*, TimeInternal* ,rt_int32_t,Boolean,RunTimeOpts*,PtpClock*);
static void handleDelayResp(const MsgHeader*, rt_int32_t, RunTimeOpts*,PtpClock*);
static void handlePDelayRespFollowUp(const MsgHeader*, rt_int32_t, RunTimeOpts*,PtpClock*);
static void handleManagement(MsgHeader*, Boolean,RunTimeOpts*,PtpClock*);
static void handleSignaling(PtpClock*);

static void issueAnnounce(RunTimeOpts*,PtpClock*);
static void issueSync(RunTimeOpts*,PtpClock*);
static void issueFollowup(const TimeInternal*,RunTimeOpts*,PtpClock*);
static void issuePDelayReq(RunTimeOpts*,PtpClock*);
static void issueDelayReq(RunTimeOpts*,PtpClock*);
static void issuePDelayResp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*);
static void issueDelayResp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*);
static void issuePDelayRespFollowUp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*);


void addForeign(Octet*,MsgHeader*,PtpClock*);

void clearCounters(PtpClock *);
void clearStats(PtpClock *);


void check_signals(RunTimeOpts * rtOpts, PtpClock * ptpClock) 
{
    (void) rtOpts;
    (void) ptpClock;
    // nothing to do
}
/* loop forever. doState() has a switch for the actions and events to be
   checked for 'port_state'. the actions and events may or may not change
   'port_state' by calling toState(), but once they are done we loop around
   again and perform the actions required for the new 'port_state'. */
void protocol(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBG("event POWERUP\n");

	toState(PTP_INITIALIZING, rtOpts, ptpClock);

	DBG("Debug Initializing...\n");

	for (;;)
	{
		/* 20110701: this main loop was rewritten to be more clear */

		if (ptpClock->portState == PTP_INITIALIZING) 
        {
			if (!doInit(rtOpts, ptpClock)) 
            {
				return;
			}
		} 
        else 
        {
			doState(rtOpts, ptpClock);
		}
        
		if (ptpClock->message_activity)
			DBGV("activity\n");

		/* Perform the heavy signal processing synchronously */
		check_signals(rtOpts, ptpClock);
	}
}


/* perform actions required when leaving 'port_state' and entering 'state' */
void toState(UInteger8 state, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	ptpClock->message_activity = TRUE;
	
	/* leaving state tasks */
	switch (ptpClock->portState)
	{
	case PTP_MASTER:
		timerStop(SYNC_INTERVAL_TIMER, ptpClock->itimer);  
		timerStop(ANNOUNCE_INTERVAL_TIMER, ptpClock->itimer);
		timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer); 
		break;
		
	case PTP_SLAVE:
		timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);
		
		if (ptpClock->delayMechanism == E2E)
			timerStop(DELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
		else if (ptpClock->delayMechanism == P2P)
			timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
		/* save observed drift value, don't inform user */
		saveDrift(ptpClock, rtOpts, TRUE);
		initClock(rtOpts, ptpClock); 
		break;
		
	case PTP_PASSIVE:
		timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
		timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);
		break;
		
	case PTP_LISTENING:
		timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);
		break;
		
	default:
		break;
	}
	
	/* entering state tasks */

	DBG("tostate %s\n",portState_getName(state));

	/*
	 * No need of PRE_MASTER state because of only ordinary clock
	 * implementation.
	 */
	
	switch (state)
	{
	case PTP_INITIALIZING:
		ptpClock->portState = PTP_INITIALIZING;
		break;
		
	case PTP_FAULTY:
		ptpClock->portState = PTP_FAULTY;
		break;
		
	case PTP_DISABLED:
		ptpClock->portState = PTP_DISABLED;
		break;
		
	case PTP_LISTENING:
		/* in Listening mode, make sure we don't send anything. Instead we just expect/wait for announces (started below) */
		timerStop(SYNC_INTERVAL_TIMER,      ptpClock->itimer);
		timerStop(ANNOUNCE_INTERVAL_TIMER,  ptpClock->itimer);
		timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
		timerStop(DELAYREQ_INTERVAL_TIMER,  ptpClock->itimer);
		
		/*
		 *  Count how many _unique_ timeouts happen to us.
		 *  If we were already in Listen mode, then do not count this as a seperate reset, but stil do a new IGMP refresh
		 */
		if (ptpClock->portState != PTP_LISTENING) 
        {
			ptpClock->reset_count++;
		}

		/* Revert to the original DelayReq interval, and ignore the one for the last master */
		ptpClock->logMinDelayReqInterval = rtOpts->initial_delayreq;

		/* force a IGMP refresh per reset */
		if (rtOpts->do_IGMP_refresh) 
        {
			netRefreshIGMP(&ptpClock->netPath, rtOpts, ptpClock);
		}
		

		timerRestart(ANNOUNCE_RECEIPT_TIMER,  (ptpClock->announceReceiptTimeout) *  (pow2ms(ptpClock->logAnnounceInterval)),  ptpClock->itimer);
		ptpClock->portState = PTP_LISTENING;
		displayStatus(ptpClock, "Now in state: ");
		break;

	case PTP_MASTER:
		timerStart(SYNC_INTERVAL_TIMER, pow2ms(ptpClock->logSyncInterval), ptpClock->itimer);

		timerStart(ANNOUNCE_INTERVAL_TIMER, pow2ms(ptpClock->logAnnounceInterval), ptpClock->itimer);

		if (ptpClock->delayMechanism == P2P) 
        {
			timerStart(PDELAYREQ_INTERVAL_TIMER, pow2ms(ptpClock->logMinPdelayReqInterval), ptpClock->itimer);
		}
		ptpClock->portState = PTP_MASTER;
		displayStatus(ptpClock, "Now in state: ");
        issueAnnounce(rtOpts, ptpClock);
		break;

	case PTP_PASSIVE:
		if (ptpClock->delayMechanism == P2P) 
        {
			timerStart(PDELAYREQ_INTERVAL_TIMER, pow2ms(ptpClock->logMinPdelayReqInterval), ptpClock->itimer);
		}
		timerRestart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->announceReceiptTimeout) * (pow2ms(ptpClock->logAnnounceInterval)), ptpClock->itimer);
		ptpClock->portState = PTP_PASSIVE;
		displayStatus(ptpClock, "Now in state: ");
		p1(ptpClock, rtOpts);
		break;

	case PTP_UNCALIBRATED:
		ptpClock->portState = PTP_UNCALIBRATED;
		break;

	case PTP_SLAVE:
		initClock(rtOpts, ptpClock);

		/*
		 * restore the observed drift value using the selected method,
		 * reset on failure or when -F 0 (default) is used, don't inform user
		 */
		restoreDrift(ptpClock, rtOpts, TRUE);


		ptpClock->waitingForFollow = FALSE;
		ptpClock->waitingForDelayResp = FALSE;

		// FIXME: clear these vars inside initclock
		clearTime(&ptpClock->delay_req_send_time);
		clearTime(&ptpClock->delay_req_receive_time);
		clearTime(&ptpClock->pdelay_req_send_time);
		clearTime(&ptpClock->pdelay_req_receive_time);
		clearTime(&ptpClock->pdelay_resp_send_time);
		clearTime(&ptpClock->pdelay_resp_receive_time);
		
		timerStart(OPERATOR_MESSAGES_TIMER, OPERATOR_MESSAGES_INTERVAL*1000, ptpClock->itimer);
		
		timerRestart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->announceReceiptTimeout) * (pow2ms(ptpClock->logAnnounceInterval)), ptpClock->itimer);
		
		/*
		 * Previously, this state transition would start the
		 * delayreq timer immediately.  However, if this was
		 * faster than the first received sync, then the servo
		 * would drop the delayResp Now, we only start the
		 * timer after we receive the first sync (in
		 * handle_sync())
		 */
		ptpClock->waiting_for_first_sync = TRUE;
		ptpClock->waiting_for_first_delayresp = TRUE;

		ptpClock->portState = PTP_SLAVE;
		displayStatus(ptpClock, "Now in state: ");



		/* 
		 * leap second pending in kernel but no leap second 
		 * info from GM - withdraw kernel leap second
		 * if the flags have disappeared but we're past 
		 * leap second event, do nothing - kernel flags 
		 * will be unset in handleAnnounce()
		 */
		if((!ptpClock->leap59 && !ptpClock->leap61) && !ptpClock->leapSecondInProgress && (checkTimexFlags(STA_INS) || checkTimexFlags(STA_DEL))) 
        {
			WARNING(INFO_PREFIX "Leap second pending in kernel but not on "	"GM: aborting kernel leap second\n");
			unsetTimexFlags(STA_INS | STA_DEL, TRUE);
		}

		break;
	default:
		DBG("to unrecognized state\n");
		break;
	}

	if (rtOpts->displayStats)
		displayStats(rtOpts, ptpClock);
}


Boolean doInit(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBG("manufacturerIdentity: %s\n", MANUFACTURER_ID);
	DBG("manufacturerOUI: %02X:%02X:%02X \n", MANUFACTURER_ID_OUI0, MANUFACTURER_ID_OUI1, MANUFACTURER_ID_OUI2);
	/* initialize networking */
	netShutdown(&ptpClock->netPath, rtOpts, ptpClock);
	if (!netInit(&ptpClock->netPath, rtOpts, ptpClock)) 
    {
		ERROR("failed to initialize network\n");
		toState(PTP_FAULTY, rtOpts, ptpClock);
		return FALSE;
	}
	
	/* initialize other stuff */
	initData(rtOpts, ptpClock);
	initTimer(rtOpts, ptpClock);
	initClock(rtOpts, ptpClock);

	/* restore observed drift and inform user */
	if(ptpClock->slaveOnly)
		restoreDrift(ptpClock, rtOpts, FALSE);

	m1(rtOpts, ptpClock);
	msgPackHeader(ptpClock->msgObuf, ptpClock);
	
	toState(PTP_LISTENING, rtOpts, ptpClock);
	
	return TRUE;
}

/* handle actions and events for 'port_state' */
static void doState(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	UInteger8 state;
	
	ptpClock->message_activity = FALSE;
	
	/* Process record_update (BMC algorithm) before everything else */
	switch (ptpClock->portState)
	{
	case PTP_LISTENING:
	case PTP_PASSIVE:
	case PTP_SLAVE:
	case PTP_MASTER:
		/*State decision Event*/

		/* If we received a valid Announce message, and can use it (record_update), then run the BMC algorithm */
		if(ptpClock->record_update)
		{
			DBG2("event STATE_DECISION_EVENT\n");
			ptpClock->record_update = FALSE;
			state = bmc(ptpClock->foreign, rtOpts, ptpClock);
			if(state != ptpClock->portState)
				toState(state, rtOpts, ptpClock);
		}
		break;
		
	default:
		break;
	}
	
	
	switch (ptpClock->portState)
	{
	case PTP_FAULTY:
		/* imaginary troubleshooting */
		DBG("event FAULT_CLEARED\n");
		toState(PTP_INITIALIZING, rtOpts, ptpClock);
		return;
		
	case PTP_LISTENING:
	case PTP_UNCALIBRATED:
	case PTP_SLAVE:
	// passive mode behaves like the SLAVE state, in order to wait for the announce timeout of the current active master
	case PTP_PASSIVE:
		handle(rtOpts, ptpClock);
		
		/*
		 * handle SLAVE timers:
		 *   - No Announce message was received
		 *   - Time to send new delayReq  (miss of delayResp is not monitored explicitelly)
		 */
		if (timerExpired(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer))
		{
			DBG("event ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES\n");

			if(!ptpClock->slaveOnly && ptpClock->clockQuality.clockClass != SLAVE_ONLY_CLOCK_CLASS) 
            {
				ptpClock->number_foreign_records = 0;
				ptpClock->foreign_record_i = 0;
				m1(rtOpts,ptpClock);
				toState(PTP_MASTER, rtOpts, ptpClock);

			} 
            else if(ptpClock->portState != PTP_LISTENING)
            {

				/*
				* Don't reset yet - just disqualify current GM.
				* If another live master exists, it will be selected,
				* otherwise, timer will cycle and we will reset.
				* Also don't clear the FMR just yet.
				*/
				if (ptpClock->grandmasterClockQuality.clockClass != 255 &&
				    ptpClock->grandmasterPriority1 != 255 &&
				    ptpClock->grandmasterPriority2 != 255) 
                {
					ptpClock->grandmasterClockQuality.clockClass = 255;
					ptpClock->grandmasterPriority1 = 255;
					ptpClock->grandmasterPriority2 = 255;
					ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterPriority1=255;
					ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterPriority2=255;
					ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterClockQuality.clockClass=255;
					INFO(INFO_PREFIX "GM announce timeout, disqualified current best GM\n");
					netRefreshIGMP(&ptpClock->netPath, rtOpts, ptpClock);
				} 
                else 
                {
					INFO(INFO_PREFIX "No active masters present\n");
					ptpClock->number_foreign_records = 0;
					ptpClock->foreign_record_i = 0;
					toState(PTP_LISTENING, rtOpts, ptpClock);
				}
			} 
            else
            {
				/*
				 *  Force a reset when getting a timeout in state listening, that will lead to an IGMP reset
				 *  previously this was not the case when we were already in LISTENING mode
				 */
				    toState(PTP_LISTENING, rtOpts, ptpClock);                         
            }
          }

		if (timerExpired(OPERATOR_MESSAGES_TIMER, ptpClock->itimer)) 
        {
			reset_operator_messages(rtOpts, ptpClock);
		}


		if (ptpClock->delayMechanism == E2E) 
        {
			if(timerExpired(DELAYREQ_INTERVAL_TIMER, ptpClock->itimer)) 
            {
				DBG2("event DELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
				issueDelayReq(rtOpts,ptpClock);
			}
		} 
        else if (ptpClock->delayMechanism == P2P) 
        {
			if (timerExpired(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer)) 
            {
				DBGV("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
				issuePDelayReq(rtOpts,ptpClock);
			}

			/* FIXME: Path delay should also rearm its timer with the value received from the Master */
		}

        if (ptpClock->leap59 || ptpClock->leap61) 
            DBGV("seconds to midnight: %.3f\n",secondsToMidnight());

        /* leap second period is over */
        if(timerExpired(LEAP_SECOND_PAUSE_TIMER,ptpClock->itimer) && ptpClock->leapSecondInProgress) 
        {
            /* 
            * do not unpause offset calculation just
            * yet, just indicate and it will be
            * unpaused in handleAnnounce()
            */
            ptpClock->leapSecondPending = FALSE;
            timerStop(LEAP_SECOND_PAUSE_TIMER,ptpClock->itimer);
        } 
        /* check if leap second is near and if we should pause updates */
        if( ptpClock->leapSecondPending && !ptpClock->leapSecondInProgress && (secondsToMidnight() <= getPauseAfterMidnight(ptpClock->logAnnounceInterval))) 
        {
            WARNING(INFO_PREFIX "Leap second event imminent - pausing " "clock and offset updates\n");
            ptpClock->leapSecondInProgress = TRUE;
            if(!checkTimexFlags(ptpClock->leap61 ? STA_INS : STA_DEL)) 
            {
                WARNING(INFO_PREFIX "Kernel leap second flags have " "been unset - attempting to set " "again");
                setTimexFlags(ptpClock->leap61 ? STA_INS : STA_DEL, FALSE);
            }
            /*
            * start pause timer from now until [pause] after
            * midnight, plus an extra second if inserting
            * a leap second
            */
            timerStart(LEAP_SECOND_PAUSE_TIMER, secondsToMidnight() + (int)ptpClock->leap61 + getPauseAfterMidnight(ptpClock->logAnnounceInterval), ptpClock->itimer);
        }

		break;

	case PTP_MASTER:
		/*
		 * handle SLAVE timers:
		 *   - Time to send new Sync
		 *   - Time to send new Announce
		 *   - Time to send new PathDelay
		 *      (DelayResp has no timer - as these are sent and retransmitted by the slaves)
		 */
	
		if (timerExpired(ANNOUNCE_INTERVAL_TIMER, ptpClock->itimer)) 
        {
			DBGV("event ANNOUNCE_INTERVAL_TIMEOUT_EXPIRES\n");
			issueAnnounce(rtOpts, ptpClock);
		}

		if (timerExpired(SYNC_INTERVAL_TIMER, ptpClock->itimer)) 
        {
			DBGV("event SYNC_INTERVAL_TIMEOUT_EXPIRES\n");
			issueSync(rtOpts, ptpClock);
		}
			
		if (ptpClock->delayMechanism == P2P) 
        {
			if (timerExpired(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer)) 
            {
				DBGV("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
				issuePDelayReq(rtOpts,ptpClock);
			}
		}
		
		// TODO: why is handle() below expiretimer, while in slave is the opposite
		handle(rtOpts, ptpClock);
		
		if (ptpClock->slaveOnly || ptpClock->clockQuality.clockClass == SLAVE_ONLY_CLOCK_CLASS)
			toState(PTP_LISTENING, rtOpts, ptpClock);
		
		break;

	case PTP_DISABLED:
		handle(rtOpts, ptpClock);
		break;
		
	default:
		DBG("(doState) do unrecognized state\n");
		break;
	}
}

static Boolean
isFromCurrentParent(const PtpClock *ptpClock, const MsgHeader* header)
{

	return (Boolean)(!memcmp(
		ptpClock->parentPortIdentity.clockIdentity,
		header->sourcePortIdentity.clockIdentity,
		CLOCK_IDENTITY_LENGTH)	&& 
		(ptpClock->parentPortIdentity.portNumber ==
		 header->sourcePortIdentity.portNumber));

}

/* check and handle received messages */
void
handle(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	int ret = SELECT_TIMER;
	rt_int32_t length = 0;
	Boolean isFromSelf;
	TimeInternal tint = { 0, 0 };

	if (!ptpClock->message_activity)
    {
		ret = netSelect(&ptpClock->tickInterval, &ptpClock->netPath);
		if (ret < 0) 
        {
			PERROR("failed to poll sockets");
			toState(PTP_FAULTY, rtOpts, ptpClock);
			return;
		}
        else if (!ret)
        {
			/* DBGV("handle: nothing\n"); */
			return;
		}
		/* else length > 0 */
	}       
	/* TODO: this should be based on the select actual FDs (if(FD_ISSET(...)) */
	if (ret == SELECT_EVENT) 
    {
		length = netRecvEvent(ptpClock->msgIbuf, &tint, &ptpClock->netPath);

		if (length > 0) 
        {
			/*
			 * make sure we use the TAI to UTC offset specified, if the master is sending the UTC_VALID bit
			 *
			 *
			 * On the slave, all timestamps that we handle here have been collected by our local clock (loopback+kernel-level timestamp)
			 * This includes delayReq just send, and delayResp, when it arrives.
			 *
			 * these are then adjusted to the same timebase of the Master (+35 leap seconds, as of July 2012)
			 *
			 */
			//DBGV("__UTC_offset: %d %d \n", ptpClock->currentUtcOffsetValid, ptpClock->currentUtcOffset);
			//if (ptpClock->currentUtcOffsetValid) {
			//	tint.seconds += ptpClock->currentUtcOffset;
                        //}
		} 
        else if (length < 0) 
        {
			PERROR("failed to receive on the event socket");
			toState(PTP_FAULTY, rtOpts, ptpClock);
			return;
		}
	}

	if (ret == SELECT_GENEAL) 
    {
		length = netRecvGeneral(ptpClock->msgIbuf, &ptpClock->netPath);
		if (length < 0) 
        {
			PERROR("failed to receive on the general socket");
			toState(PTP_FAULTY, rtOpts, ptpClock);
			return;
		}
	}

	if (!length) 
    {
		return;
	}
      
	ptpClock->message_activity = TRUE;

	if (length < HEADER_LENGTH) 
    {
		DBG("Error: message shorter than header length\n");
		return;
	}

	msgUnpackHeader(ptpClock->msgIbuf, &ptpClock->msgTmpHeader);

	if (ptpClock->msgTmpHeader.versionPTP != ptpClock->versionNumber) 
    {
		DBG2("ignore version %d message\n", ptpClock->msgTmpHeader.versionPTP);
		return;
	}

	if(ptpClock->msgTmpHeader.domainNumber != ptpClock->domainNumber) 
    {
		DBG2("ignore message from domainNumber %d\n", ptpClock->msgTmpHeader.domainNumber);
		return;
	}

	/*Spec 9.5.2.2*/
	isFromSelf = (Boolean)(ptpClock->portIdentity.portNumber == ptpClock->msgTmpHeader.sourcePortIdentity.portNumber
		      && !memcmp(ptpClock->msgTmpHeader.sourcePortIdentity.clockIdentity, ptpClock->portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH));

	/*
	 * subtract the inbound latency adjustment if it is not a loop
	 *  back and the time stamp seems reasonable 
	 */
	if (!isFromSelf && tint.seconds > 0)
		subTime(&tint, &tint, &rtOpts->inboundLatency);


#ifdef PTPD_DBG
    {
        /* easy display of received messages */
        char *st;

        switch(ptpClock->msgTmpHeader.messageType)
        {
        case ANNOUNCE:
            st = "Announce";
            break;
        case SYNC:
            st = "Sync";
            break;
        case FOLLOW_UP:
            st = "FollowUp";
            break;
        case DELAY_REQ:
            st = "DelayReq";
            break;
        case DELAY_RESP:
            st = "DelayResp";
            break;
        case MANAGEMENT:
            st = "Management";
            break;
        default:
            st = "Unk";
            break;
        }
        DBG("      ==> %s received\n", st);
    }
#endif

	/*
	 *  on the table below, note that only the event messsages are passed the local time,
	 *  (collected by us by loopback+kernel TS, and adjusted with UTC seconds
	 *
	 *  (SYNC / DELAY_REQ / PDELAY_REQ / PDELAY_RESP)
	 */
	switch(ptpClock->msgTmpHeader.messageType)
	{
	case ANNOUNCE:
		handleAnnounce(&ptpClock->msgTmpHeader,length, isFromSelf, rtOpts, ptpClock);
		break;
	case SYNC:
		handleSync(&ptpClock->msgTmpHeader, 
			   length, &tint, isFromSelf, rtOpts, ptpClock);
		break;
	case FOLLOW_UP:
		handleFollowUp(&ptpClock->msgTmpHeader,
			       length, isFromSelf, rtOpts, ptpClock);
		break;
	case DELAY_REQ:
		handleDelayReq(&ptpClock->msgTmpHeader,
			       length, &tint, isFromSelf, rtOpts, ptpClock);
		break;
	case PDELAY_REQ:
		handlePDelayReq(&ptpClock->msgTmpHeader,
				length, &tint, isFromSelf, rtOpts, ptpClock);
		break;  
	case DELAY_RESP:
		handleDelayResp(&ptpClock->msgTmpHeader,
				length, rtOpts, ptpClock);
		break;
	case PDELAY_RESP:
		handlePDelayResp(&ptpClock->msgTmpHeader,
				 &tint, length, isFromSelf, rtOpts, ptpClock);
		break;
	case PDELAY_RESP_FOLLOW_UP:
		handlePDelayRespFollowUp(&ptpClock->msgTmpHeader, 
					 length, rtOpts, ptpClock);
		break;
	case MANAGEMENT:
		handleManagement(&ptpClock->msgTmpHeader,
				 isFromSelf, rtOpts, ptpClock);
		break;
	case SIGNALING:
		handleSignaling(ptpClock);
		break;
	default:
		DBG("handle: unrecognized message\n");
		break;
	}

}

/*spec 9.5.3*/
static void 
handleAnnounce(MsgHeader *header, rt_int32_t length, 
	       Boolean isFromSelf, const RunTimeOpts *rtOpts, PtpClock *ptpClock)
{

	DBGV("HandleAnnounce : Announce message received : \n");

	if(length < ANNOUNCE_LENGTH) 
    {
		DBG("Error: Announce message too short\n");
		return;
	}

	switch (ptpClock->portState) {
	case PTP_INITIALIZING:
	case PTP_FAULTY:
	case PTP_DISABLED:
		DBG("Handleannounce : disregard\n");
		return;
		
	case PTP_UNCALIBRATED:	
	case PTP_SLAVE:
		if (isFromSelf) 
        {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}
		
		/*
		 * Valid announce message is received : BMC algorithm
		 * will be executed 
		 */
		ptpClock->record_update = TRUE;

		switch (isFromCurrentParent(ptpClock, header)) 
        {
		case TRUE:
	   		msgUnpackAnnounce(ptpClock->msgIbuf, &ptpClock->msgTmp.announce);

			/* update datasets (file bmc.c) */
	   		s1(header,&ptpClock->msgTmp.announce,ptpClock, rtOpts);

			/* update current master in the fmr as well */
			memcpy(&ptpClock->foreign[ptpClock->foreign_record_best].header,
			       header,sizeof(MsgHeader));
			memcpy(&ptpClock->foreign[ptpClock->foreign_record_best].announce,
			       &ptpClock->msgTmp.announce,sizeof(MsgAnnounce));

			if(ptpClock->leapSecondInProgress) 
            {
				/*
				 * if leap second period is over
				 * (pending == FALSE, inProgress ==
				 * TRUE), unpause offset calculation
				 * (received first announce after leap
				 * second)
				*/
				if (!ptpClock->leapSecondPending) {
					WARNING(INFO_PREFIX "Leap second event over - "
						"resuming clock and offset updates\n");
					ptpClock->leapSecondInProgress=FALSE;
					ptpClock->leap59 = FALSE;
					ptpClock->leap61 = FALSE;

					unsetTimexFlags(STA_INS | STA_DEL, TRUE);

				}
			}
			DBG2("___ Announce: received Announce from current Master, so reset the Announce timer\n");
	   		/*Reset Timer handling Announce receipt timeout*/
	   		timerRestart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->announceReceiptTimeout) * (pow2ms(ptpClock->logAnnounceInterval)), ptpClock->itimer);

#ifdef PTPD_EXPERIMENTAL
			// remember IP address of our master for -U option
			// todo: add this to bmc(), to cover the very first packet
			ptpClock->MasterAddr = ptpClock->netPath.lastRecvAddr;
#endif
			break;

		case FALSE:
			/*addForeign takes care of AnnounceUnpacking*/
			/* the actual decision to change masters is only done in  doState() / record_update == TRUE / bmc() */
			/*
			 * wowczarek: do not restart timer here:
			 * the slave will  sit idle if current parent
			 * is not announcing, but another GM is
			 */
			addForeign(ptpClock->msgIbuf,header,ptpClock);
			break;

		default:
			DBG("HandleAnnounce : (isFromCurrentParent)"
			     "strange value ! \n");
	   		return;

		} /* switch on (isFromCurrentParrent) */
		break;

	/*
	 * Passive case: previously, this was handled in the default, just like the master case.
	 * This the announce would call addForeign(), but NOT reset the timer, so after 12s it would expire and we would come alive periodically 
	 *
	 * This code is now merged with the slave case to reset the timer, and call addForeign() if it's a third master
	 *
	 */
	case PTP_PASSIVE:
		if (isFromSelf) {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}

		/*
		 * Valid announce message is received : BMC algorithm
		 * will be executed
		 */
		ptpClock->record_update = TRUE;

		if (isFromCurrentParent(ptpClock, header)) {
			msgUnpackAnnounce(ptpClock->msgIbuf,
					  &ptpClock->msgTmp.announce);

			/* TODO: not in spec
			 * datasets should not be updated by another master
			 * this is the reason why we are PASSIVE and not SLAVE
			 * this should be p1(ptpClock, rtOpts);
			 */
			/* update datasets (file bmc.c) */
			s1(header,&ptpClock->msgTmp.announce,ptpClock, rtOpts);

			DBG("___ Announce: received Announce from current Master, so reset the Announce timer\n\n");
			/*Reset Timer handling Announce receipt timeout*/
			timerRestart(ANNOUNCE_RECEIPT_TIMER,
				   (ptpClock->announceReceiptTimeout) *
				   (pow2ms(ptpClock->logAnnounceInterval)),
				   ptpClock->itimer);
		} else {
			/*addForeign takes care of AnnounceUnpacking*/
			/* the actual decision to change masters is only done in  doState() / record_update == TRUE / bmc() */
			/* the original code always called: addforeign(new master) + timerstart(announce) */

			DBG("___ Announce: received Announce from another master, will add to the list, as it might be better\n\n");
			DBGV("this is to be decided immediatly by bmc())\n\n");
			addForeign(ptpClock->msgIbuf,header,ptpClock);
		}
		break;

		
	case PTP_MASTER:
	case PTP_LISTENING:           /* listening mode still causes timeouts in order to send IGMP refreshes */
	default :
		if (isFromSelf) {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}
		DBGV("Announce message from another foreign master\n");
		addForeign(ptpClock->msgIbuf,header,ptpClock);
		ptpClock->record_update = TRUE;    /* run BMC() as soon as possible */
		break;

	} /* switch on (port_state) */
}

static void
processSelfSync(TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock) 
{
    /*Add latency*/
    addTime(tint, tint, &rtOpts->outboundLatency);
    issueFollowup(tint, rtOpts, ptpClock);    
}

static void 
handleSync(const MsgHeader *header, rt_int32_t length, 
	   TimeInternal *tint, Boolean isFromSelf, 
	   RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	TimeInternal OriginTimestamp;
	TimeInternal correctionField;

	DBGV("Sync message received : \n");

	if (length < SYNC_LENGTH) {
		DBG("Error: Sync message too short\n");
		return;
	}

	switch (ptpClock->portState) {
	case PTP_INITIALIZING:
	case PTP_FAULTY:
	case PTP_DISABLED:
		DBGV("HandleSync : disregard\n");
		return;
		
	case PTP_UNCALIBRATED:
	case PTP_SLAVE:
		if (isFromSelf) 
        {
			DBGV("HandleSync: Ignore message from self \n");
			return;
		}

        if (isFromCurrentParent(ptpClock, header)) 
        {
            if (isTimeInternalZero(tint)) 
            {
                WARNING("HandleSync: Ignored - invalid receive timestamp .\n");
                break;
            }
                        
			/* We only start our own delayReq timer after receiving the first sync */
			if (ptpClock->waiting_for_first_sync) 
            {
				ptpClock->waiting_for_first_sync = FALSE;
				NOTICE("Received first Sync from Master\n");
				NOTICE("   going to arm DelayReq timer for the first time, with initial rate: %d\n", ptpClock->logMinDelayReqInterval);

				if (ptpClock->delayMechanism == E2E)
					timerStart(DELAYREQ_INTERVAL_TIMER, pow2ms(ptpClock->logMinDelayReqInterval), ptpClock->itimer);
				else if (ptpClock->delayMechanism == P2P)
					timerStart(PDELAYREQ_INTERVAL_TIMER, pow2ms(ptpClock->logMinPdelayReqInterval), ptpClock->itimer);
			}

			ptpClock->sync_receive_time.seconds = tint->seconds;
			ptpClock->sync_receive_time.nanoseconds = tint->nanoseconds;


			if ((header->flagField0 & PTP_TWO_STEP) == PTP_TWO_STEP) 
            {
				DBG2("HandleSync(%d): waiting for follow-up \n", header->sequenceId);

				ptpClock->waitingForFollow = TRUE;
				ptpClock->recvSyncSequenceId = header->sequenceId;
				/*Save correctionField of Sync message*/
				integer64_to_internalTime(header->correctionField, &correctionField);
				ptpClock->lastSyncCorrectionField.seconds = correctionField.seconds;
				ptpClock->lastSyncCorrectionField.nanoseconds = correctionField.nanoseconds;
				break;
			} 
            else 
            {
				msgUnpackSync(ptpClock->msgIbuf, &ptpClock->msgTmp.sync);
				integer64_to_internalTime(ptpClock->msgTmpHeader.correctionField, &correctionField);
				//timeInternal_display(&correctionField);
				ptpClock->waitingForFollow = FALSE;
				toInternalTime(&OriginTimestamp, &ptpClock->msgTmp.sync.originTimestamp);
				updateOffset(&OriginTimestamp, &ptpClock->sync_receive_time, &ptpClock->ofm_filt,rtOpts, ptpClock,&correctionField);
				updateClock(rtOpts,ptpClock);
				break;
			}
		} 
        else 
        {
			DBG("HandleSync: Sync message received from "  "another Master not our own \n");
		}
		break;

	case PTP_MASTER:
	default :
		if (!isFromSelf) 
        {
			DBGV("HandleSync: Sync message received from "  "another Master  \n");
			/* we are the master, but another is sending */
			break;
		} 
        if (ptpClock->twoStepFlag)
        {
			DBGV("HandleSync: going to send followup message\n ");
			processSelfSync(tint, rtOpts, ptpClock);
			break;
		} 
        else 
        {
			DBGV("HandleSync: Sync message received from self\n ");
		}
	}
}


static void 
handleFollowUp(const MsgHeader *header, rt_int32_t length, 
	       Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	TimeInternal preciseOriginTimestamp;
	TimeInternal correctionField;

    DBGV("Handlefollowup : Follow up message received \n");
    
	if (length < FOLLOW_UP_LENGTH)
	{
		DBG("Error: Follow up message too short\n");
		return;
	}

	if (isFromSelf)
	{
		DBGV("Handlefollowup : Ignore message from self \n");
		return;
	}

	switch (ptpClock->portState)
	{
	case PTP_INITIALIZING:
	case PTP_FAULTY:
	case PTP_DISABLED:
	case PTP_LISTENING:
		DBGV("Handlefollowup : disregard\n");
		return;
		
	case PTP_UNCALIBRATED:	
	case PTP_SLAVE:
		if (isFromCurrentParent(ptpClock, header)) 
        {
			if (ptpClock->waitingForFollow)	
            {
				if (ptpClock->recvSyncSequenceId == header->sequenceId) 
                {
					msgUnpackFollowUp(ptpClock->msgIbuf, &ptpClock->msgTmp.follow);
					ptpClock->waitingForFollow = FALSE;
					toInternalTime(&preciseOriginTimestamp, &ptpClock->msgTmp.follow.preciseOriginTimestamp);
					integer64_to_internalTime(ptpClock->msgTmpHeader.correctionField, &correctionField);
					addTime(&correctionField,&correctionField, &ptpClock->lastSyncCorrectionField);

					/*
					send_time = preciseOriginTimestamp (received inside followup)
					recv_time = sync_receive_time (received as CMSG in handleEvent)
					*/
					updateOffset(&preciseOriginTimestamp, &ptpClock->sync_receive_time,&ptpClock->ofm_filt, rtOpts, ptpClock, &correctionField);
					updateClock(rtOpts,ptpClock);
					break;
				} 
                else
                {
                    INFO("Ignored followup, SequenceID:%d doesn't match with last Sync message:%d \n", header->sequenceId, ptpClock->recvSyncSequenceId);
                }
			} 
            else 
            {
				DBG2("Ignored followup, Slave was not waiting a follow up " "message \n");
			}
		} 
        else 
        {
			DBG2("Ignored, Follow up message is not from current parent \n");
        }
        break;
	case PTP_MASTER:
	case PTP_PASSIVE:
		if(isFromCurrentParent(ptpClock, header))
        {
			DBGV("Ignored, Follow up message received from current master \n");
        }
		else 
        {
            /* follow-ups and syncs are expected to be seen from parent, but not from others */
			DBGV("Follow up message received from foreign master!\n");
		}
		break;

	default:
    		DBG("do unrecognized state1\n");
    		break;
	} /* Switch on (port_state) */

}

static void
processSelfDelayReq(const TimeInternal *tint, RunTimeOpts *rtOpts, PtpClock *ptpClock) {
    /*
     * Get sending timestamp from IP stack
     * with SO_TIMESTAMP
     */

    /*
     *  Make sure we process the REQ _before_ the RESP. While we could do this by any order,
     *  (because it's implicitly indexed by (ptpClock->sentDelayReqSequenceId - 1), this is
     *  now made explicit
     */
    ptpClock->waitingForDelayResp = TRUE;

    ptpClock->delay_req_send_time.seconds =
            tint->seconds;
    ptpClock->delay_req_send_time.nanoseconds =
            tint->nanoseconds;

    /*Add latency*/
    addTime(&ptpClock->delay_req_send_time,
            &ptpClock->delay_req_send_time,
            &rtOpts->outboundLatency);   
}

static void
handleDelayReq(const MsgHeader *header, rt_int32_t length, 
	       const TimeInternal *tint, Boolean isFromSelf,
	       RunTimeOpts *rtOpts, PtpClock *ptpClock)
{

	if (ptpClock->delayMechanism == E2E) {
		DBGV("delayReq message received : \n");
		
		if (length < DELAY_REQ_LENGTH) {
			DBG("Error: DelayReq message too short\n");
			return;
		}

		switch (ptpClock->portState) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
		case PTP_PASSIVE:
			DBGV("HandledelayReq : disregard\n");
			return;

		case PTP_SLAVE:
			if (isFromSelf)	{
				DBG("==> Handle DelayReq (%d)\n",
					 header->sequenceId);

				if ( ((UInteger16)(header->sequenceId + 1)) !=
					ptpClock->sentDelayReqSequenceId) {
					DBG("HandledelayReq : sequence mismatch - "
					    "last DelayReq sent: %d, received: %d\n",
					    ptpClock->sentDelayReqSequenceId,
					    header->sequenceId
					    );
					break;
				}

				processSelfDelayReq(tint, rtOpts, ptpClock);

				break;
			} else {
				DBG2("HandledelayReq : disregard delayreq from other client\n");
			}
			break;

		case PTP_MASTER:
			msgUnpackHeader(ptpClock->msgIbuf, &ptpClock->delayReqHeader);
			
#ifdef PTPD_EXPERIMENTAL
			// remember IP address of this client for -U option
			ptpClock->LastSlaveAddr = ptpClock->netPath.lastRecvAddr;
#endif
					
			issueDelayResp(tint,&ptpClock->delayReqHeader, rtOpts,ptpClock);
            
			break;

		default:
			DBG("do unrecognized state2\n");
			break;
		}
	} else {/* (Peer to Peer mode) */
		DBG("Delay messages are ignored in Peer to Peer mode\n");
	}
}

static void
handleDelayResp(const MsgHeader *header, rt_int32_t length,
		RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == E2E) 
    {
		TimeInternal requestReceiptTimestamp;
		TimeInternal correctionField;

		DBGV("delayResp message received : \n");

		if(length < DELAY_RESP_LENGTH) 
        {
			DBG("Error: DelayResp message too short\n");
			return;
		}

		switch(ptpClock->portState) 
        {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
			DBGV("HandledelayResp : disregard\n");
			return;

		case PTP_SLAVE:
			msgUnpackDelayResp(ptpClock->msgIbuf, &ptpClock->msgTmp.resp);

			if ((memcmp(ptpClock->portIdentity.clockIdentity, ptpClock->msgTmp.resp.requestingPortIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH) == 0) 
			    && (ptpClock->portIdentity.portNumber == ptpClock->msgTmp.resp.requestingPortIdentity.portNumber)
			    && isFromCurrentParent(ptpClock, header)) 
            {
				DBG("==> Handle DelayResp (%d)\n", header->sequenceId);

				if (!ptpClock->waitingForDelayResp) 
                {
					DBGV("Ignored DelayResp - wasn't waiting for one\n");
					break;
				}

				if (ptpClock->sentDelayReqSequenceId !=	((UInteger16)(header->sequenceId + 1))) 
                {
					DBG("HandledelayResp : sequence mismatch - "  "last DelayReq sent: %d, delayResp received: %d\n", ptpClock->sentDelayReqSequenceId, header->sequenceId);
					break;
				}

				ptpClock->waitingForDelayResp = FALSE;

				toInternalTime(&requestReceiptTimestamp,  &ptpClock->msgTmp.resp.receiveTimestamp);
				ptpClock->delay_req_receive_time.seconds = requestReceiptTimestamp.seconds;
				ptpClock->delay_req_receive_time.nanoseconds = requestReceiptTimestamp.nanoseconds;

				integer64_to_internalTime(header->correctionField,	&correctionField);
				/*
					send_time = delay_req_send_time (received as CMSG in handleEvent)
					recv_time = requestReceiptTimestamp (received inside delayResp)
				*/

				updateDelay(&ptpClock->owd_filt,  rtOpts,ptpClock, &correctionField);

				if (ptpClock->waiting_for_first_delayresp)
                {
					ptpClock->waiting_for_first_delayresp = FALSE;
					NOTICE("  received first DelayResp from Master\n");
				}

				if (rtOpts->ignore_delayreq_interval_master == 0) 
                {
					DBGV("current delay_req: %d  new delay req: %d \n",	ptpClock->logMinDelayReqInterval, header->logMessageInterval);

					/* Accept new DelayReq value from the Master */
					if (ptpClock->logMinDelayReqInterval != header->logMessageInterval) 
                    {
						NOTICE("  received new DelayReq frequency %d from Master (was: %d)\n", header->logMessageInterval, ptpClock->logMinDelayReqInterval );
					}

					// collect new value indicated from the Master
					ptpClock->logMinDelayReqInterval = header->logMessageInterval;
					
					/* FIXME: the actual rearming of this timer with the new value only happens later in doState()/issueDelayReq() */
				} 
                else 
                {
					if (ptpClock->logMinDelayReqInterval != rtOpts->subsequent_delayreq) 
                    {
						NOTICE("  received new DelayReq frequency %d from command line (was: %d)\n", rtOpts->subsequent_delayreq, ptpClock->logMinDelayReqInterval);
					}
					ptpClock->logMinDelayReqInterval = rtOpts->subsequent_delayreq;
				}
			} 
            else 
            {
				DBG("HandledelayResp : delayResp doesn't match with the delayReq. \n");
				break;
			}
		}
	} 
    else 
    { /* (Peer to Peer mode) */
		DBG("Delay messages are disregarded in Peer to Peer mode \n");
	}

}

static void
processSelfPDelayReq(const TimeInternal * tint, const RunTimeOpts * rtOpts, PtpClock * ptpClock) {
    /*
     * Get sending timestamp from IP stack
     * with SO_TIMESTAMP
     */
    ptpClock->pdelay_req_send_time.seconds = tint->seconds;
    ptpClock->pdelay_req_send_time.nanoseconds = tint->nanoseconds;

    /*Add latency*/
    addTime(&ptpClock->pdelay_req_send_time, &ptpClock->pdelay_req_send_time,
            &rtOpts->outboundLatency);
}

static void
handlePDelayReq(MsgHeader *header, rt_int32_t length, 
		const TimeInternal *tint, Boolean isFromSelf, 
		RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == P2P) {
		DBGV("PdelayReq message received : \n");

		if(length < PDELAY_REQ_LENGTH) {
			DBG("Error: PDelayReq message too short\n");
			return;
		}

		switch (ptpClock->portState ) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
			DBGV("HandlePdelayReq : disregard\n");
			return;

		case PTP_SLAVE:
		case PTP_MASTER:
		case PTP_PASSIVE:

			if (isFromSelf) {
				processSelfPDelayReq(tint, rtOpts, ptpClock);
				break;
			} else {
				msgUnpackHeader(ptpClock->msgIbuf,
						&ptpClock->PdelayReqHeader);
				issuePDelayResp(tint, header, rtOpts, 
						ptpClock);	
				break;
			}
		default:
			DBG("do unrecognized state3\n");
			break;
		}
	} else {/* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End "
		      "mode \n");
		}
}

static void
processSelfPDelayResp(TimeInternal * tint, RunTimeOpts *rtOpts, PtpClock * ptpClock) {
	addTime(tint,tint,&rtOpts->outboundLatency);
	issuePDelayRespFollowUp(tint, &ptpClock->PdelayReqHeader, rtOpts,ptpClock);
}

static void
handlePDelayResp(const MsgHeader *header, TimeInternal *tint,
		 rt_int32_t length, Boolean isFromSelf, 
		 RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == P2P) {
		/* Boolean isFromCurrentParent = FALSE; NOTE: This is never used in this function */
		TimeInternal requestReceiptTimestamp;
		TimeInternal correctionField;
	
		DBGV("PdelayResp message received : \n");

		if (length < PDELAY_RESP_LENGTH) {
			DBG("Error: PDelayResp message too short\n");
			return;
		}

		switch (ptpClock->portState ) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
			DBGV("HandlePdelayResp : disregard\n");
			return;

		case PTP_SLAVE:
		case PTP_MASTER:
			if (ptpClock->twoStepFlag && isFromSelf) {
				processSelfPDelayResp(tint, rtOpts, ptpClock);
				break;
			}
			msgUnpackPDelayResp(ptpClock->msgIbuf,
					    &ptpClock->msgTmp.presp);
		
#if 0  /* NOTE: This is never used in this function. Should it? */
/*
 * wowczarek: shouldn't, P2P vs. E2E. Parent can be somewhere else,
 * peer can sit  on an adjacent link - see next evaluation:
 * requestingPortIdentity vs. parentPortIdentity
 */
			isFromCurrentParent = !memcmp(ptpClock->parentPortIdentity.clockIdentity,
						      header->sourcePortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH) && 
				(ptpClock->parentPortIdentity.portNumber == 
				 header->sourcePortIdentity.portNumber);
#endif	
			if (ptpClock->sentPDelayReqSequenceId != 
			       header->sequenceId) {
				    DBGV("PDelayResp: sequence mismatch - sent: %d, received: %d\n",
					    ptpClock->sentPDelayReqSequenceId,
					    header->sequenceId);
				    break;
			}
			if ((!memcmp(ptpClock->portIdentity.clockIdentity,ptpClock->msgTmp.presp.requestingPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH))
				 && ( ptpClock->portIdentity.portNumber == ptpClock->msgTmp.presp.requestingPortIdentity.portNumber))	{
                                /* Two Step Clock */
				if ((header->flagField0 & PTP_TWO_STEP) == PTP_TWO_STEP) {
					/*Store t4 (Fig 35)*/
					ptpClock->pdelay_resp_receive_time.seconds = tint->seconds;
					ptpClock->pdelay_resp_receive_time.nanoseconds = tint->nanoseconds;
					/*store t2 (Fig 35)*/
					toInternalTime(&requestReceiptTimestamp,
						       &ptpClock->msgTmp.presp.requestReceiptTimestamp);
					ptpClock->pdelay_req_receive_time.seconds = requestReceiptTimestamp.seconds;
					ptpClock->pdelay_req_receive_time.nanoseconds = requestReceiptTimestamp.nanoseconds;
					
					integer64_to_internalTime(header->correctionField,&correctionField);
					ptpClock->lastPdelayRespCorrectionField.seconds = correctionField.seconds;
					ptpClock->lastPdelayRespCorrectionField.nanoseconds = correctionField.nanoseconds;
				} else {
				/* One step Clock */
					/*Store t4 (Fig 35)*/
					ptpClock->pdelay_resp_receive_time.seconds = tint->seconds;
					ptpClock->pdelay_resp_receive_time.nanoseconds = tint->nanoseconds;
					
					integer64_to_internalTime(header->correctionField,&correctionField);
					updatePeerDelay (&ptpClock->owd_filt,rtOpts,ptpClock,&correctionField,FALSE);
				}
				ptpClock->recvPDelayRespSequenceId = header->sequenceId;
				break;
			} else {
				DBGV("HandlePdelayResp : Pdelayresp doesn't "
				     "match with the PdelayReq. \n");
				break;
			}
			//break; /* XXX added by gnn for safety */
		default:
			DBG("do unrecognized state4\n");
			break;
		}
	} else { /* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End "
		      "mode \n");
	}
}

static void 
handlePDelayRespFollowUp(const MsgHeader *header, rt_int32_t length, 
			 RunTimeOpts *rtOpts, 
			 PtpClock *ptpClock)
{

	if (ptpClock->delayMechanism == P2P) {
		TimeInternal responseOriginTimestamp;
		TimeInternal correctionField;
	
		DBGV("PdelayRespfollowup message received : \n");
	
		if(length < PDELAY_RESP_FOLLOW_UP_LENGTH) {
			DBG("Error: PDelayRespfollowup message too short\n");
			return;
		}	
	
		switch(ptpClock->portState) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
			DBGV("HandlePdelayResp : disregard\n");
			return;
		
		case PTP_SLAVE:
		case PTP_MASTER:
			if (( ((UInteger16)(header->sequenceId + 1)) ==
			    ptpClock->sentPDelayReqSequenceId) && (header->sequenceId == ptpClock->recvPDelayRespSequenceId)) {
				msgUnpackPDelayRespFollowUp(
					ptpClock->msgIbuf,
					&ptpClock->msgTmp.prespfollow);
				toInternalTime(
					&responseOriginTimestamp,
					&ptpClock->msgTmp.prespfollow.responseOriginTimestamp);
				ptpClock->pdelay_resp_send_time.seconds = 
					responseOriginTimestamp.seconds;
				ptpClock->pdelay_resp_send_time.nanoseconds = 
					responseOriginTimestamp.nanoseconds;
				integer64_to_internalTime(
					ptpClock->msgTmpHeader.correctionField,
					&correctionField);
				addTime(&correctionField,&correctionField,
					&ptpClock->lastPdelayRespCorrectionField);
				updatePeerDelay (&ptpClock->owd_filt,
						 rtOpts, ptpClock,
						 &correctionField,TRUE);
				break;
			} else {
				DBGV("PdelayRespFollowup: sequence mismatch - Received: %d "
				"PdelayReq sent: %d, PdelayResp received: %d\n",
				header->sequenceId, ptpClock->sentPDelayReqSequenceId,
				ptpClock->recvPDelayRespSequenceId);
				break;
			}
		default:
			DBGV("Disregard PdelayRespFollowUp message  \n");
		}
	} else { /* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End "
		      "mode \n");
	}
}

static void 
handleManagement(MsgHeader *header,
		 Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBGV("Management message received : \n");
}

static void 
handleSignaling(PtpClock *ptpClock)
{
   (void)ptpClock;
}


/*Pack and send on general multicast ip adress an Announce message*/
static void issueAnnounce(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	msgPackAnnounce(ptpClock->msgObuf,ptpClock);

	if (!netSendGeneral(ptpClock->msgObuf,ANNOUNCE_LENGTH,
			    &ptpClock->netPath, 0)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("Announce message can't be sent -> FAULTY state \n");
	} else {
		DBGV("Announce MSG sent ! \n");
		ptpClock->sentAnnounceSequenceId++;
	}
}



/*Pack and send on event multicast ip adress a Sync message*/
static void
issueSync(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
	getTime(&internalTime);
	fromInternalTime(&internalTime,&originTimestamp);

	msgPackSync(ptpClock->msgObuf,&originTimestamp,ptpClock);

	if (!netSendEvent(ptpClock->msgObuf,SYNC_LENGTH, &internalTime, &ptpClock->netPath, 0)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("Sync message can't be sent -> FAULTY state \n");
	} else {
		DBGV("Sync MSG sent ! \n");
		ptpClock->sentSyncSequenceId++;
        if (ptpClock->twoStepFlag && !isTimeInternalZero(&internalTime)) {
 	       processSelfSync(&internalTime, rtOpts, ptpClock);
        }
	}
}


/*Pack and send on general multicast ip adress a FollowUp message*/
static void
issueFollowup(const TimeInternal *tint,RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp preciseOriginTimestamp;
	fromInternalTime(tint,&preciseOriginTimestamp);
	
	msgPackFollowUp(ptpClock->msgObuf,&preciseOriginTimestamp,ptpClock);
	
	if (!netSendGeneral(ptpClock->msgObuf,FOLLOW_UP_LENGTH,
			    &ptpClock->netPath, 0)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("FollowUp message can't be sent -> FAULTY state \n");
	} else {
		DBGV("FollowUp MSG sent ! \n");
	}
}


/*Pack and send on event multicast ip adress a DelayReq message*/
static void
issueDelayReq(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
    Integer32 dst = 0;
    
	DBG("==> Issue DelayReq (%d)\n", ptpClock->sentDelayReqSequenceId );

	/* call GTOD. This time is later replaced on handle_delayreq, to get the actual send timestamp from the OS */
	getTime(&internalTime);
	fromInternalTime(&internalTime,&originTimestamp);

	// uses current sentDelayReqSequenceId
	msgPackDelayReq(ptpClock->msgObuf, &originTimestamp, rtOpts, ptpClock);

	
#ifdef PTPD_EXPERIMENTAL
	if (rtOpts->do_hybrid_mode) {
		dst = ptpClock->MasterAddr;
	}
#endif

	if (!netSendEvent(ptpClock->msgObuf,DELAY_REQ_LENGTH, &internalTime,
			  &ptpClock->netPath, dst)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("delayReq message can't be sent -> FAULTY state \n");
	} else {
		DBGV("DelayReq MSG sent ! \n");
		ptpClock->sentDelayReqSequenceId++;

		/* From now on, we will only accept delayreq and delayresp of (sentDelayReqSequenceId - 1) */

		/* Explicitelly re-arm timer for sending the next delayReq */
		timerStartRandom(DELAYREQ_INTERVAL_TIMER,
		   pow2ms(ptpClock->logMinDelayReqInterval),
		   ptpClock->itimer);

		if (!isTimeInternalZero(&internalTime)) {
			processSelfDelayReq(&internalTime, rtOpts, ptpClock);
		}
	}
}
/*Pack and send on event multicast ip adress a DelayResp message*/
static void
issueDelayResp(const TimeInternal *tint,MsgHeader *header,RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
    Integer32 dst = 0;

    Timestamp requestReceiptTimestamp;
    fromInternalTime(tint,&requestReceiptTimestamp);
    msgPackDelayResp(ptpClock->msgObuf,header,&requestReceiptTimestamp,
        rtOpts, ptpClock);

#ifdef PTPD_EXPERIMENTAL
    if (rtOpts->do_hybrid_mode) {
        dst = ptpClock->LastSlaveAddr;
    }
#endif

    if (!netSendGeneral(ptpClock->msgObuf,PDELAY_RESP_LENGTH,
        &ptpClock->netPath, dst)) {
            toState(PTP_FAULTY,rtOpts,ptpClock);
            DBGV("delayResp message can't be sent -> FAULTY state \n");
    } else {
        DBGV("DelayResp MSG sent ! \n");
    }
}

/*Pack and send on event multicast ip adress a PDelayReq message*/
static void
issuePDelayReq(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
	getTime(&internalTime);
	fromInternalTime(&internalTime,&originTimestamp);
	
	msgPackPDelayReq(ptpClock->msgObuf,&originTimestamp,ptpClock);
	if (!netSendPeerEvent(ptpClock->msgObuf, PDELAY_REQ_LENGTH, &internalTime,
			      &ptpClock->netPath)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("PdelayReq message can't be sent -> FAULTY state \n");
	} else {
		DBGV("PDelayReq MSG sent ! \n");
		if (ptpClock->twoStepFlag && !isTimeInternalZero(&internalTime)) {
			processSelfPDelayReq(&internalTime, rtOpts, ptpClock);
		}
	}
}

/*Pack and send on event multicast ip adress a PDelayResp message*/
static void
issuePDelayResp(const TimeInternal *tint,MsgHeader *header,RunTimeOpts *rtOpts,
		PtpClock *ptpClock)
{
	Timestamp requestReceiptTimestamp;
	TimeInternal internalTime;
	fromInternalTime(tint,&requestReceiptTimestamp);
	msgPackPDelayResp(ptpClock->msgObuf,header,
			  &requestReceiptTimestamp,ptpClock);

	if (!netSendPeerEvent(ptpClock->msgObuf,PDELAY_RESP_LENGTH, &internalTime,
			      &ptpClock->netPath)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("PdelayResp message can't be sent -> FAULTY state \n");
	} else {
		DBGV("PDelayResp MSG sent ! \n");
		if (!isTimeInternalZero(&internalTime)) {
			processSelfPDelayResp(&internalTime, rtOpts, ptpClock);
		}
	}
}

static void
issuePDelayRespFollowUp(const TimeInternal *tint, MsgHeader *header,
			     RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	Timestamp responseOriginTimestamp;
	fromInternalTime(tint,&responseOriginTimestamp);

	msgPackPDelayRespFollowUp(ptpClock->msgObuf,header,
				  &responseOriginTimestamp,ptpClock);
	if (!netSendPeerGeneral(ptpClock->msgObuf,
				PDELAY_RESP_FOLLOW_UP_LENGTH,
				&ptpClock->netPath)) {
		toState(PTP_FAULTY,rtOpts,ptpClock);
		DBGV("PdelayRespFollowUp message can't be sent -> FAULTY state \n");
	} else {
		DBGV("PDelayRespFollowUp MSG sent ! \n");
	}
}


void
addForeign(Octet *buf,MsgHeader *header,PtpClock *ptpClock)
{
	int i,j;
	Boolean found = FALSE;

	j = ptpClock->foreign_record_best;
	
	/*Check if Foreign master is already known*/
	for (i=0;i<ptpClock->number_foreign_records;i++) {
		if (!memcmp(header->sourcePortIdentity.clockIdentity,
			    ptpClock->foreign[j].foreignMasterPortIdentity.clockIdentity,
			    CLOCK_IDENTITY_LENGTH) && 
		    (header->sourcePortIdentity.portNumber == 
		     ptpClock->foreign[j].foreignMasterPortIdentity.portNumber))
		{
			/*Foreign Master is already in Foreignmaster data set*/
			ptpClock->foreign[j].foreignMasterAnnounceMessages++; 
			found = TRUE;
			DBGV("addForeign : AnnounceMessage incremented \n");
			msgUnpackHeader(buf,&ptpClock->foreign[j].header);
			msgUnpackAnnounce(buf,&ptpClock->foreign[j].announce);
			break;
		}
	
		j = (j+1)%ptpClock->number_foreign_records;
	}

	/*New Foreign Master*/
	if (!found) {
		if (ptpClock->number_foreign_records < 
		    ptpClock->max_foreign_records) {
			ptpClock->number_foreign_records++;
		}
		j = ptpClock->foreign_record_i;
		
		/*Copy new foreign master data set from Announce message*/
		copyClockIdentity(ptpClock->foreign[j].foreignMasterPortIdentity.clockIdentity,
		       header->sourcePortIdentity.clockIdentity);
		ptpClock->foreign[j].foreignMasterPortIdentity.portNumber = 
			header->sourcePortIdentity.portNumber;
		ptpClock->foreign[j].foreignMasterAnnounceMessages = 0;
		
		/*
		 * header and announce field of each Foreign Master are
		 * usefull to run Best Master Clock Algorithm
		 */
		msgUnpackHeader(buf,&ptpClock->foreign[j].header);
		msgUnpackAnnounce(buf,&ptpClock->foreign[j].announce);
		DBGV("New foreign Master added \n");
		
		ptpClock->foreign_record_i = 
			(ptpClock->foreign_record_i+1) % 
			ptpClock->max_foreign_records;	
	}
}

void
clearStats(PtpClock * ptpClock)
{
	/* TODO: print port info */
	DBG("Port statistics cleared\n");
	memset(&ptpClock->stats, 0, sizeof(ptpClock->stats));

}

Boolean respectUtcOffset(RunTimeOpts * rtOpts, PtpClock * ptpClock) 
{
	(void)rtOpts;
    if (ptpClock->currentUtcOffsetValid) 
    {
		return TRUE;
	}
	return FALSE;
}
