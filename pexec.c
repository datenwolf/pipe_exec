#define _GNU_SOURCE
#include <unistd.h>
//#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/memfd.h>
#include <linux/fcntl.h>
#include <errno.h>
#include <stddef.h>

int fcntl(int fd, int cmd, ...);

static
int memfd_create(char const *name, unsigned flags)
{
	return syscall(SYS_memfd_create, name, flags);
}

static
int copyfile(int fd_in, int fd_out)
{
	int rc;
	size_t total_bytes = 0;
	char buf[4096];
	for(;;) {
		size_t bytes;
		ptrdiff_t wr_off = 0;
redo_read:
		rc= read(fd_in, buf, sizeof(buf));
		if( 0 > rc ){
			int const read_errno = errno;
			switch( read_errno ){
			case EAGAIN: usleep(100);
			case EINTR:  rc = 0;
				goto redo_read;

			default: return -1;
			}
		} else
		if( !rc ){ break; }
		bytes = rc;

redo_write:
		while( bytes ){
			rc= write(fd_out, buf + wr_off, bytes);
			if( 0 > rc ){
				int const read_errno = errno;
				switch( read_errno ){
				case EAGAIN: usleep(100);
				case EINTR:  rc = 0;
					goto redo_write;

				default:
					return -1;
				}
			}
			wr_off += rc;
			total_bytes += rc;;
			bytes -= rc;
		}
	}
	return total_bytes;
}

int main(int argc, char *argv[])
{
	int rc;
	int fd_exe;

	(void)argc;

	fd_exe = memfd_create("exe", MFD_CLOEXEC | MFD_ALLOW_SEALING);
	if( -1 == fd_exe ){ return 2; }

	do {
		rc= copyfile(0, fd_exe);
		if( 0 > rc ){ return 3; }
	} while( 0 < rc );

	rc= fcntl(fd_exe,
		F_ADD_SEALS,
		F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_WRITE | F_SEAL_SEAL );
	if( 0 > rc ){ return 4; }

	fexecve(fd_exe, argv, environ);
	return 5;
}
