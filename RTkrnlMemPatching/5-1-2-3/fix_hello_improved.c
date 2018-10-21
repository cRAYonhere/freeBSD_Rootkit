#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#define SIZE 0x64

/* Replacement code. */
unsigned char nop_code[] = "\x90\x90"; /* nop */

int main(int argc, char *argv[]){
	int i, jns_offset, call_offset;
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	struct nlist nl[] = { {NULL}, {NULL}, {NULL}, };
	unsigned char hello_code[SIZE], call_operand[4];

	/* Initialize kernel virtual memory access. */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (kd == NULL) {
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	nl[0].n_name = "hello";
	nl[1].n_name = "uprintf";

	/* Find the address of hello and uprintf. */
	if (kvm_nlist(kd, nl) < 0){
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Check if hello was found */
	if (!nl[0].n_value) {
		fprintf(stderr, "ERROR: Symbol %s not found\n", nl[0].n_name);
		exit(-1);
	}

	/* Check if uprintf was found */
	if(!nl[1].n_value) {
		fprintf(stderr, "ERROR: Symbol %s not found\n", nl[1].n_name);
		exit(-1);
	}

	/* Save a copy of hello. */
	if (kvm_read(kd, nl[0].n_value, hello_code, SIZE) < 0) {
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Search through hello for the jns and call instructions. */
	for (i =0 ; i < SIZE; i++){
		/* looking for jns instruction */
		if(hello_code[i] == 0xe9 && hello_code[i+1] == 0xd7)
			jns_offset = i;
		/* looking for call instruction */
		if(hello_code[i] == 0xe8)
			call_offset = i;
	}

	/* Calculate the call statement operand. */
	*(unsigned long *)&call_operand[0] = nl[1].n_value - (nl[0].n_value + call_offset + 5);

	/*
	// Patch hello.
	if (kvm_write(kd, nl[0].n_value + jns_offset, nop_code, sizeof(nop_code) -1) < 0) {
		fprintf(stderr, "ERROR in hello Patch: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	*/

	/* Patch printf. */
	if(kvm_write(kd, nl[0].n_value + call_offset + 1, call_operand, sizeof(call_operand)) < 0 ) {
		fprintf(stderr, "ERROR in printf Patch: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	/* Close kd. */
	if (kvm_close(kd) < 0) {
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	exit(0);
}
