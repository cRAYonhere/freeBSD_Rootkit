#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#define SIZE 0x30

/* Replacement code. */
unsigned char nop_code[] ="\x90\x90"; /* nop */

int main(int argc, char *argv[]){
  int i, offset;
  char errbuf[_POSIX2_LINE_MAX];
  kvm_t *kd;
  struct nlist nl[] = { {NULL}, {NULL}, };
  unsigned char hello_code[SIZE];

  /* Initialize kernel virtual memory access. */
  kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
  if (kd == NULL){
    fprintf(stderr, "ERROR: %s\n", errbuf);
    exit(-1);
  }
  nl[0].n_name = "hello";

  /* Find the address of hello. */
  if (kvm_nlist(kd, nl) < 0) {
    fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
    exit(-1);
  }

  if (!nl[0].n_value) {
    fprintf(stderr, "ERROR: Symbol %s not found\n", nl[0].n_name);
    exit(-1);
  }

  /* Save a copy of hello. */
  if (kvm_read(kd, nl[0].n_value, hello_code, SIZE) < 0) {
    fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
    exit(-1);
  }

  /* Search through hello for the jns instruction. */
  for (i = 0; i < SIZE; i++){
    if (hello_code[i] == 0x79) {
      offset = i;
      break;
    }
  }
  /* Patch hello. */
  if (kvm_write(kd, nl[0].n_value + offset, nop_code, sizeof(nop_code) - 1) < 0){
    fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
    exit(-1);
  }

  /* Close kd. */
  if (kvm_close(kd) < 0) {
    fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
    exit(-1);
  }
  exit(0);
}
