#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
//#include <sys/stat.h>
#include <sys/fcntl.h>

/* openat system call hook. */
static int
openat_hook(struct thread *td, void *syscall_args)
{
	struct openat_args /* {
		int fd;
		char *path;
		int flags;
	} */ *uap;
	uap = (struct openat_args *)syscall_args;

	char path[255];
	size_t done;
	int error;

	error = copyinstr(uap->path, path, 255, &done);
	if (error == -1)
		return(error);
	/* Print a debug message. */

	uprintf("The files \"%s\"in directory will be shown\n",uap->path);
	return(sys_openat(td, syscall_args));
}

/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg){

	int error = 0;

	switch (cmd) {
	
		case MOD_LOAD:
			/* Replace mkdir with mkdir_hook. */
			sysent[SYS_openat].sy_call = (sy_call_t *)openat_hook;
			break;
		case MOD_UNLOAD:
			/* Change everything back to normal. */
			sysent[SYS_openat].sy_call = (sy_call_t *)sys_openat;
			break;
		default:
			error = EOPNOTSUPP;
			break;
		}
		return(error);
}

static moduledata_t openat_hook_mod = {
	"openat_hook",	/* module name*/
	load,		/* event handler */
	NULL		/* extra data */
};

DECLARE_MODULE(openat_hook, openat_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
