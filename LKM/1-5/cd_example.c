#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>

/* Function prototypes. */
d_open_t	open;
d_close_t	close;
d_read_t	read;
d_write_t	write;

static struct cdevsw cd_example_cdevsw = {
	.d_version =	D_VERSION,
	.d_open	=	open,
	.d_close =	close,
	.d_read =	read,
	.d_write =	write,
	.d_name =	"cd_example"
};

static char buf[512+1];
static size_t len;

int
open(struct cdev *dev, int flag, int otyp, struct thread *td){
	/* Initialize character buffer. */
	memset(&buf, '\0', 513);
	len = 0;

	return(0);
}

int
close(struct cdev *dev, int flag, int otyp, struct thread *td){
	return(0);
}

//Moving data from user space to a buffer in kernal space
int
write(struct cdev *dev, struct uio *uio, int ioflag){
	int error = 0;

       /*
	* Take in aracter string, saving it in buf.
	* Note: The proper way to transfer data between buffers and I/O
	* vectors that cross the user/kernel space boundary is with
	* uiomove(), but this way is shorter. For more on device driver I/O
	* routines, see the uio(9) manual page.
	*/
	error = copyinstr(uio->uio_iov->iov_base, &buf, 512, &len);
	if (error != 0)
		uprintf("Write to \"cd_example\" failed.\n");
	uprintf("Length: %d\n", len);
	return(error);
}

int
read(struct cdev *dev, struct uio *uio, int ioflag){
	
	int error = 0;

	if (len <= 0){
		error = -1;
	}
	else{
		/* Return the saved character string to userland. */
		copystr(&buf, uio->uio_iov->iov_base, 513, &len);
		uprintf("Length: %d\n",len);
	}
	return(error);
}

/* Reference to the device in DEVFS */
static struct cdev *sdev;

/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg){

	int error = 0;

	switch (cmd) {
		case MOD_LOAD:
			sdev = make_dev(&cd_example_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "cd_example");
			uprintf("Chracter device loaded.\n");
			break;
		case MOD_UNLOAD:
			destroy_dev(sdev);
			uprintf("Character device unloaded.\n");
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}
	
	return(error);
}

DEV_MODULE(cd_example, load, NULL);
