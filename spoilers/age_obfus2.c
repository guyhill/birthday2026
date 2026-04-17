/*
 * We can do some more obfuscation with the string literals. We
 * gather them into one static string and employ a simple encryption
 * algorithm so that the texts are no longer readable. Of course
 * we now need a simple decryption algorithm to decode the text.
 * 
 * By ending this static string with a period we can also replace 
 * the "." in the open call with a reference to the static string.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

char *msg = "N'fj'~bfut'hkc)\r.";

void write_str(char *s, int len) {
	if (len > 0) {
		char c = *s ^ 7;
		syscall(SYS_write, STDOUT_FILENO, &c, 1);
		write_str(s + 1, len - 1);
	}
}

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
		
		syscall(SYS_open, msg + 16, 0xc1, 0644);
		age = age * errno;
		
		write_str(msg, 5);
		write_int(age);
		write_str(msg + 4, 12);
	}
	return 0;
}


