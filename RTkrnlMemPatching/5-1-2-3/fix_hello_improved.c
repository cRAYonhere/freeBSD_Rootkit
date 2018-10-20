#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <sys/types.h>

#define SIZE 0x30

/* Replacement code. */
unsigned char nop_code[] = "\x90\x90"; /* nop */

int main(int argc, char *argv[]){
	int i, jns_offset, call_offset;
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	struct nlist nl[] = { {NULL}, {NULL}, {NULL}, };
	unsigned char hello_code[SIZE], call_operand[4];

	/* Initialize kernel virtual memory access. */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDRW, errbuf);
	if (kd == NULL) {
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	nl[0].n_name = "hello";
	n1[1].n_name = "uprintf";
