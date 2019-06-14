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
 * @file   timer.c
 * @date   Wed Jun 23 09:41:26 2010
 * 
 * @brief  The timers which run the state machine.
 * 
 * Timers in the PTP daemon are run off of the signal system.  
 */

#include "../ptpd.h"


#define US_TIMER_INTERVAL (25000)
/*
 * original code calls sigalarm every fixed 1ms. This highly pollutes the debug_log, and causes more interrupted instructions
 * This was later modified to have a fixed granularity of 1s.
 *
 * Currently this has a configured granularity, and timerStart() guarantees that clocks expire ASAP when the granularity is too small.
 * Timers must now be explicitelly canceled with timerStop (instead of timerStart(0.0))
 */

static rt_timer_t timerHandles[TIMER_ARRAY_SIZE];

static rt_mailbox_t ptp_mbox;

static void timer_process(void* para)
{
    IntervalTimer * timer;
    timer = (IntervalTimer*)(para);
    timer->expire = TRUE;
    signalAction(SELECT_TIMER);
}

void signalAction(int value)
{
    if (ptp_mbox)
    {
        rt_mb_send(ptp_mbox, value);
    }
}

int waitSignal(void)
{
    int result;
    if (ptp_mbox) 
    {
        if(rt_mb_recv(ptp_mbox, (rt_uint32_t*)&result, RT_WAITING_FOREVER) == RT_EOK) 
        {
            DBGV("signal: %d \r\n", result);
            return result;
        }
        else
        {
            DBGV("wait signal is err!\r\n");
            return -1;
        }
    }

    return 0;
}

void initTimer(RunTimeOpts *rtOpts, PtpClock *ptpClock) 
{
    int i;
    char time_name[10];
    DBG("initTimer\n");
    (void) rtOpts;
    
    ptpClock->tickInterval.seconds = 0;
    ptpClock->tickInterval.nanoseconds = US_TIMER_INTERVAL * 1000;
  
    //null时重新创建，否则清空内容
    if (!ptp_mbox) 
    {
        ptp_mbox = rt_mb_create("PTPd_mb", 32, RT_IPC_FLAG_FIFO);
    }
    else
    {
        rt_mb_control(ptp_mbox, RT_IPC_CMD_RESET, NULL);
    }

    for (i = 0; i < TIMER_ARRAY_SIZE; i++) 
    {
         //null时重新创建，否则停止计时
        if (!timerHandles[i]) 
        {
            rt_snprintf(time_name, RT_NAME_MAX, "%s%d", "timer", i);
            timerHandles[i] = rt_timer_create(time_name, timer_process, &(ptpClock->itimer[i]),100, RT_TIMER_FLAG_PERIODIC);
        }
        else
        {
            rt_timer_stop(timerHandles[i]);
        }
        ptpClock->itimer[i].interval = 0;
    }
}

void timerStop(UInteger16 index, IntervalTimer * itimer)
{
    if (index >= TIMER_ARRAY_SIZE)
        return;

    if (timerHandles[index]) 
    {
        rt_timer_stop(timerHandles[index]);
    }
    
    itimer[index].interval = 0;
    DBG2("timerStop:      Stopping timer %d.   (New interval: %d)\n", index, itimer[index].interval);
}

void timerRestart(UInteger16 index, Integer32 interval, IntervalTimer * itimer)
{
    Integer32 ms = interval / (1000 / RT_TICK_PER_SECOND);

    if (interval == 0)//间隔为0 表示不起用
    {
        rt_kprintf("TIMERestart:%d is zero! no restart!\n", index);
        return;
    }
    if (index >= TIMER_ARRAY_SIZE)
        return;
    
    //最小定时间隔为1ms
    if (ms == 0)
    {
        ms = 1;
    }
    itimer[index].expire = FALSE;
    if (timerHandles[index])
    {
        rt_timer_stop(timerHandles[index]);
        rt_timer_control(timerHandles[index], RT_TIMER_CTRL_SET_TIME, &ms);
        rt_timer_start(timerHandles[index]);
    }
    else
    {
        rt_kprintf("TIMERestart:timer%d is NULL!", index);
        return;
    }

    itimer[index].interval = interval;   
    DBG("TIMERestart:     Set timer %d to %d ms.  New interval: %d ms;\n", index, interval, itimer[index].interval);    
}

void timerStart(UInteger16 index, Integer32 interval, IntervalTimer * itimer) 
{
    Integer32 ms = interval / (1000 / RT_TICK_PER_SECOND);

    if (index >= TIMER_ARRAY_SIZE)
        return;

    if (interval == 0)//间隔为0 表示不起用
    {
        rt_kprintf("TIMEStart:%d is zero! no start!\n", index);
        return;
    }
    itimer[index].expire = FALSE;
    //最小定时间隔为1ms
    if (ms == 0)
    {
        ms = 1;
    }
    if (timerHandles[index])
    {
        rt_timer_stop(timerHandles[index]);
        rt_timer_control(timerHandles[index], RT_TIMER_CTRL_SET_TIME, &ms);
        rt_timer_start(timerHandles[index]);
    }
    else
    {
        rt_kprintf("TIMEStart:timer%d is NULL!", index);
        return;
    }

    itimer[index].interval =  interval;
    DBG("TIMEStart:     Set timer %d to %d ms.  New interval: %d ms;\n", index, interval, itimer[index].interval);
}

/*
 * This function arms the timer with a uniform range, as requested by page 105 of the standard (for sending delayReqs.)
 * actual time will be U(0, interval * 2.0);
 *
 * PTPv1 algorithm was:
 *    ptpClock->R = getRand(&ptpClock->random_seed) % (PTP_DELAY_REQ_INTERVAL - 2) + 2;
 *    R is the number of Syncs to be received, before sending a new request
 * 
 */ 
void timerStartRandom(UInteger16 index, Integer32 interval, IntervalTimer * itimer)
{
	Integer32 new_value;

    //new_value = (int)(getRand() * interval * 2.0);
    new_value = interval;
	DBG2(" timerStart_random: requested %d ms, got %d ms\n", interval, new_value);
	
	timerStart(index, new_value, itimer);
}



Boolean timerExpired(UInteger16 index, IntervalTimer * itimer)
{
	if (index >= TIMER_ARRAY_SIZE)
		return FALSE;

	if (!itimer[index].expire)
		return FALSE;

	itimer[index].expire = FALSE;


	DBG2("timerExpired:   Timer %d expired, current interval: %d;\n", index, itimer[index].interval);

	return TRUE;
}
