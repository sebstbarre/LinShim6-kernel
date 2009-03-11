/*
 *	Linux REAP implementation
 *
 *	Author:
 *	Sébastien Barré		<sebastien.barre@uclouvain.be>
 *
 *	date : December 2007
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *  
 *      TODO : - verify that we are not using bad addresses after a change in
 *               link layer connectivity (maybe see DNA). (draft section 3.2).
 *             - Trigger an exploration when there are indications of failure
 *               from different things than the send timeout (indication from 
 *               upper layers (lower layers:done). (section 3.3)
 *             - Tune exploration order by using rfc3484 and addr selection 
 *               draft (to study) (section 4.3)
 *             - Use the MAX_PROBE_TIMEOUT as an indication that the context is
 *               no longer useable (section 4.3)
 */

#ifndef _NET_REAP_H
#define _NET_REAP_H
#include <linux/timer.h>
#include <linux/seq_file.h>
#include <linux/in6.h>
#include <net/shim6_types.h>
#include <linux/shim6.h>


#define REAP_DEBUG /*To comment if you don't want debug messages*/

struct reap_ctx {
	int                 state;
	__u64               ct_local; /*Used for sending netlink messages*/
/*Timer used for both keepalive and send timers
 * the handling function is changed accordingly*/
	struct timer_list   timer; 
	int                 stop_timer;
	spinlock_t          stop_timer_lock; /* For protection of the 
					      * stop_retr_timer flag
					      * (when doing the del_timer_sync)
					      */
	
/*Timeout values in seconds*/
	int                 tka;
	int                 tsend;

/*Used for keeping track of keepalive timeout.
 * When first starting the keepalive timer, we
 * set this field to the current value of jiffies.
 * Then, upon keepalive timer expiry, we check if the 
 * next expiry will fall after ka_timestamp+ka_timeout*HZ,
 * in which case the timer is not rearmed.
 */
	unsigned long       ka_timestamp;

/*For the shim6 garbage collector, to indicate that the reap context
  has been initialized, and so must be freed in case of garbage collection.*/
	int                 started;
	spinlock_t          lock; /*Useful despite of the lock from the xfrm
				    state, since two different xfrm states 
				    may use a shim6_ctx in the same time
				    (inbound and outbound)*/
	unsigned long       last_recvd_data; /*Timestamp for last received 
					       data*/
        char                art_switch:1; /*ART switch, this will trigger
					    a message to the daemon, when
					    the next data packet 
					    will enter*/
	struct kref         kref;
};

void init_reap_ctx(struct reap_ctx* rctx, struct shim6_data* sd);
void del_reap_ctx(struct reap_ctx *rctx);

/*To notify reap about occurrence of an incoming/outgoing packet*/
void reap_notify_in(struct reap_ctx* x);
void reap_notify_out(struct reap_ctx* rctx);

/*Manually start a new exploration process. */
void reap_init_explore(struct reap_ctx* rctx);

/*To display information about reap context within shim6 files*/
void reap_seq_show(struct seq_file *s, struct reap_ctx *rctx);

/*REAP module termination
 * This can be called ONLY by shim6_init*/
void __exit reap_exit(void);

#endif /*_NET_REAP_H*/
