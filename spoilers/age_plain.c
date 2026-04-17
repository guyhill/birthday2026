#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	/* 
	 * This program calculates our age by multiplying two 
	 * error codes. Since we are currently 51 = 3 * 17 years old,
	 * we need to multiply the errors ESRCH (No such process), 
	 * which has numerical value 3 and EEXIST (File exists), which
	 * has value 17. We force those errors by suitable combinations
	 * of Linux system calls.
	 */
	int age;
	
	if (fork()) {
		/* 
		 * This sequence of system calls forces error 
		 * ESRCH by deliberately trying to send a signal 
		 * to a process that no longer exists.  
		 * 
		 * We create a child process using fork(); this child process
		 * does nothing and terminates immediately. 
		 * 
		 * In the parent process we wait for the child process
		 * to terminate before sending a (dummy) signal to the child
		 * process. Which fails with error ESRCH because the child
		 * process no longer exists. 
		 */
		kill(wait(NULL), 0);
		age = errno;
		
		/*
		 * This open call forces error EEXIST by deliberately trying
		 * to create the file with path "." (i.e., the current 
		 * directory) with the O_EXCL flag, which "ensures that this
		 * call creates the file" (from the man page for open) and 
		 * "fails with the error EEXIST" if the path already exists.
		 */
		open(".", O_CREAT | O_WRONLY | O_EXCL, 0644);
		age = age * errno;
		
		printf("I am %d years old.\n", age);
	}
	return EXIT_SUCCESS;
}
