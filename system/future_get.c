/* future_get.c - get the value of the future */

#include <future.h>

/*------------------------------------------------------------------------
 *  future get  -  Get the value of a future set by an operation and change 
 *		   the state of future from FUTURE_VALID to FUTURE_EMPTY
 *------------------------------------------------------------------------
 */
syscall future_get(
	  future_t* f,
	  char* value
	)
{
	intmask mask;
	mask = disable();
	
	/*------------------------------------------------------------------------
	 *  FUTURE_SHARED
	 *------------------------------------------------------------------------
	 */
	if(f->mode == FUTURE_SHARED) {
		if(f->state == FUTURE_READY){
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_WAITING){
			/* enqueue the current process in get_queue  */
			insert_in_getq(f, currpid);
			suspend(currpid);

			/* resume operations */
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_EMPTY){
			/* enqueue the current process in get_queue  */
			f->pid = currpid;
			f->state = FUTURE_WAITING;
			insert_in_getq(f, currpid);
			suspend(currpid);
			
			/* resume operations */
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		
		/* If Queue is empty, send message */		
		//if(isGetQueueEmpty(f)) {
		//	send(main_pid, "Done!");
		//}
	} //end-if

	/*------------------------------------------------------------------------
	 *  FUTURE_QUEUE
	 *------------------------------------------------------------------------
	 */
	else if(f->mode == FUTURE_QUEUE) {
		if(isSetQueueEmpty(f)) {
			insert_in_getq(f, currpid);
			suspend(currpid);
			
			// after resumption
			*value = f->data;
		}
		else {
			resume(remove_from_setq(f));
			
			*value = f->data;
		}
	} //end-if
	
	else {
	
		/*------------------------------------------------------------------------
		 *  OTHERS
		 *------------------------------------------------------------------------
		 */
		if(f->state == FUTURE_READY){
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_WAITING){
			//restore(mask);
			return SYSERR;
		}
		else if(f->state == FUTURE_EMPTY){ 
			/*block waiting for it to set and store pid */
			f->pid = currpid;
			f->state = FUTURE_WAITING;
		
			/* busy wait on the future */
			while(1) {
				if(f->state == FUTURE_READY) {
					/* read future value & break from loop */
					*value = f->data;
					f->state = FUTURE_EMPTY;
					break;
				}
				else {
					/* reschedule */
					resched();
				}
			}
		}
	} //end-else

	restore(mask);
	return OK;
}