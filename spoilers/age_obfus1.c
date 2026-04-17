/*
 * We want to reduce the number of includes to the absolute
 * minimum; we also want to do without the C standard libraries.
 * 
 * The plain version uses 7 includes, which we list here with
 * what we need it for:
 * 
 * stdlib.h   - provides EXIT_SUCCESS and NULL (by including stddef.h)
 * stdio.h    - provides printf
 * signal.h   - provides kill
 * errno.h    - provides errno
 * unistd.h   - provides fork and write
 * sys/wait.h - provides wait
 * fcntl.h    - provides open, O_CREAT, O_WRONLY and O_EXCL 
 * 
 * We can eliminate most of the includes for Linux system calls by
 * calling syscall directly. Doing so eliminates signal.h and 
 * sys/wait.h from the list of required includes. We would still need
 * fcntl.h because of the O_* identifiers. We need the additional 
 * header file sys/syscall.h for the symbolic constants that represent
 * the various system calls. We also need to replace wait with wait4,
 * as wait itself is not a Linux system call.
 * 
 * We can eliminate stdlib.h and fcntl.h by replacing the various 
 * identifiers included from those header files with their values:
 * EXIT_SUCCESS -> 0 (not technically portable, but the current code
 *                    requires Linux anyway, where this is true)
 * NULL     -> 0     (not technically portable, but required by Posix)
 * O_CREAT  -> 0x40
 * O_WRONLY -> 0x01
 * O_EXCL   -> 0x80
 * 
 * To eliminate stdio.h we need to do two things:
 * 1. Write to the console without it. This can be done using the 
 * write system call and the STDOUT_FILENO file descriptor (both in
 * unistd.h)
 * 2. Write our own code to output an integer to as a string. The only
 * way to do this in standard C is by using the various format
 * specifiers in printf, which are not available if we do not include
 * stdio.h. We therefore need to write our own code to output an integer 
 * to the console.
 *  
 * Doing all this leaves just three includes:
 */
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

void write_int(int num) {
	int prefix = num / 10;
	char digit = '0' + num % 10;
	
	if (prefix != 0) {
		write_int(prefix);
	}
	syscall(SYS_write, STDOUT_FILENO, &digit, 1); 
}

int main(int argc, char **argv) {
	int age;
	
	if (syscall(SYS_fork)) {
		syscall(SYS_kill, syscall(SYS_wait4, -1, 0, 0, 0), 0);
		age = errno;
		
		syscall(SYS_open, ".", 0xc1, 0644);
		age = age * errno;
		
		syscall(SYS_write, STDOUT_FILENO, "I am ", 5);
		write_int(age);
		syscall(SYS_write, STDOUT_FILENO, " years old.\n", 12);
	}
	return 0;
}

