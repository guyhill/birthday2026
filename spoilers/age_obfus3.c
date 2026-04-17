/*
 * We can obfuscate even further by choosing single letter 
 * identifier names and replacing all remaining symbolic constants
 * with their values:
 * 
 * SYS_write ->  1
 * SYS_fork  -> 57
 * SYS_kill  -> 62
 * SYS_wait4 -> 61
 * SYS_open  ->  2
 * 
 * STDOUT_FILENO -> 1
 * 
 * Incidentally, this removes the need for including sys/syscall.h
 * 
 * We also don't need the age variable, as the main declaration already
 * gives us an integer variable to (ab)use. We can also use the fact
 * that argc == 1 when the code is called without parameters.
 * 
 * We also performed a couple of spot obfuscations, such as replace
 * '0' by 48 (as ASCII 48 is character 0) and -1 by ~0. We replaced
 * the file permissions 0644 by 0 as they don't matter anyway.
 */
#include <errno.h>
#include <unistd.h>

char *o = "N'fj'~bfut'hkc)\r.";

void p(char *v, int u) {
	if (u) {
		char w = *v ^ 7;
		syscall(1, 1, &w, 1);
		p(v + 1, u - 1);
	}
}

void q(int b) {
	int u = b / 10;
	if (u) q(u);
	char v = 48 + b % 10;
	syscall(1, 1, &v, 1); 
}

int main(int b, char **d) {
	if (syscall(57)) {
		syscall(62, syscall(61, ~0, 0, 0, 0), 0);
		b = b * errno;
		
		syscall(2, o + 16, 0xc1, 0);
		b = b * errno;
		
		p(o, 5);
		q(b);
		p(o + 4, 12);
	}
	return 0;
}



