# De-obfuscating the code

As-is, the code is almost impossible to read, so let's fix that first.
We can use a tool such as `clang-format` to render the code in a more
human-friendly format, and we can run gcc's C pre-processor to get rid
of the `#define` directives. We do not want to pre-process the 
`#include` directives, so we filter them out using `grep -v`:

```
$ grep -v include age.c | gcc -E - | clang-format

...
char *o = "N'fj'~bfut'hkc)\r.";
void q(int b) {
  int v = b / 10;
  if (v)
    q(v);
  char u = 48 + b % 10;
  syscall(1, 1, &u, 1);
}
void p(int b, char *v) {
  if (b) {
    char u = *v ^ 7;
    syscall(1, 1, &u, 1);
    p(b - 1, v + 1);
  }
}
int main(int b, char **) {
  if (syscall(57)) {
    syscall(62, syscall(61, ~0, 0, 0, 0), 0);
    b = b * errno;
    syscall(2, o + 16, 0xc1, 0);
    b = b * errno;
    p(5, o);
    q(b);
    p(12, o + 4);
  }
  return 0;
}
```

As you can see there's a `main` function as expected, as well as two other
functions with the unhelpful names `p` and `q`. We also note that
the code makes heavy use of a function called `syscall`.

## Syscall
In Linux, `syscall` is used to do what is
called a system call: a call to the Linux kernel to perform a 
specific task. The first  argument to `syscall` is a number that
determines what system call to perform, and the other arguments are the 
actual arguments of that system call.

The code performs 5 different system calls, with numbers 1, 2, 57, 61
and 62. These numbers
can be looked up in the Linux kernel source code, but a more 
accessible resource is <https://filippo.io/linux-syscall-table/>. 
For every syscall a C library function exists that performs that
system call, and they can be found in the column with heading "Manual".

Substituting these in our code we obtain:
```
char *o = "N'fj'~bfut'hkc)\r.";
void q(int b) {
  int v = b / 10;
  if (v)
    q(v);
  char u = 48 + b % 10;
  write(1, &u, 1);
}
void p(int b, char *v) {
  if (b) {
    char u = *v ^ 7;
    write(1, &u, 1);
    p(b - 1, v + 1);
  }
}
int main(int b, char **) {
  if (fork()) {
    kill(wait4(~0, 0, 0, 0), 0);
    b = b * errno;
    open(o + 16, 0xc1, 0);
    b = b * errno;
    p(5, o);
    q(b);
    p(12, o + 4);
  }
  return 0;
}
```
(If you want to run this version of the code yourself you need to
include some additional header files; I refer you to the man pages
of the system calls we've just added)

## Functions `p` and `q`
Now let's turn our attention to the functions `p` and `q`. Both 
functions call the `write` system call, which has 3 arguments:
- A file descriptor, which is a number that refers to a file;
- The address of a buffer to write to that file;
- The number of bytes to write from the buffer to the file

Now, file descriptor 1 is reserved for `stdout` (i.e., the console), 
which acts like a file in Linux, and both times a single 
character is written to `stdout`. In other words, this is the same as `putchar()`
in standard C.

### Function `q`
Let's analyse `q` by means of an example. Let's 
imagine we call this function with the number 153 as argument. So, 
`b = 153`, and it first calculates `v` as 153 / 10 = 15 (rounding down)
before calling itself with the number 15. It then calculates 
`153 % 10` = 3, adds 48 and prints the result to the console as an ASCII
character. As it so happens, ASCII 48 + i (i = 0..9) is the digit i,
so this step will print the digit '3' to the console. 

So, what this function is doing is to call itself with a
number that is obtained by removing the final digit from the original
number, before printing that final digit. The overall effect is to 
print the entire number to the console, digit by digit. 

Let's now re-write this function to be more descriptive:
```
void print_number(int num) {
  int prefix = num / 10;
  if (prefix)
    print_number(prefix);
  char final_digit = num % 10;
  putchar('0' + final_digit);
}
```

### Function `p`
Let's similarly analyse `p`, and let's first focus on the
first parameter. As long as  the value of this parameter isn't zero, 
this function calls itself with a value for the first parameter that is
one lower. Hence, if you start out with a positive value N, the 
function will run N times.

Every time it runs, it prints a single character to the console, so in
total it prints a string of N characters. It obtains these characters 
from a buffer in its second argument, and for each character in the 
buffer it runs `u = *v ^ 7;` before printing it. This is a simple
decryption algorithm, where the ASCII value is XOR-ed with the number
7. For instance, since ASCII 'A' is 65 (= 0x41 in hexadecimal), 
when the buffer contains an 'A', it XORs 0x41 with 7 to obtain 0x46, and
prints an 'F' to the console, since 0x46 = 70 = the ASCII codepoint for
the letter F.

Let's now re-write this function into something more descriptive:
```
void print_encrypted_string(char *msg, int len) {
  if (len > 0) {
    char decrypted_char = *msg ^ 7;
    putchar(decrypted_char);
    print_encrypted_string(msg + 1, len - 1);
  }
}
```

### Calls to `p` and `q`
Now we turn to where the functions `p` and `q` are called in `main`:
```
char *o = "N'fj'~bfut'hkc)\r.";
    ...
    p(5, o);
    q(b);
    p(12, o + 4);
```
We see that in the first call to `p`, the first 5 characters of `o` are
decrypted and printed, whereas in the second call, the characters in 
positions 4 to 15 of `o` are printed (and remember that the first
character in a C string is at position 0). In other words, this code 
is equivalent to

```
   print_encrypted_string("N'fj'", 5);
   print_number(b);
   print_encrypted_string("'~bfut'hkc)\r", 12);
```
Now, by decrypting the strings by hand (or running the 
`print_encrypted_string` calls separately), we see
that the first one prints `"I am "` whereas the second one
prints `" years old.\n"`. In between those two strings, it prints
the value of the variable `b`. In other words, these lines do the same
as the following `printf` statement from standard C:

```
    printf("I am %d years old.\n", b);
```
If we replace the calls to `p` and `q` in `main` with this `printf`
statement, we no longer need `p` and `q`, and we are
left with the much simpler program
```
int main(int b, char **) {
  if (fork()) {
    kill(wait4(~0, 0, 0, 0), 0);
    b = b * errno;
    open(o + 16, 0xc1, 0);
    b = b * errno;
    printf("I am %d years old.\n", b);
  }
  return 0;
}
```
## Obtaining my current age
Now, obviously the variable `b` should, by the time the `printf` 
statement is reached, contain my current age. So,
how does that work?

We note that the variable `b` is, at two points in the code, multiplied
by `errno`. This is a global variable that is set to a certain error 
code each time a system call fails. What this code does is perform
two sequences of system calls that fail in predictable ways, so that
`errno` has a predictable value afterwards.

First a sequence of three system calls is run: 
- `fork()`
- `wait4()`
- `kill()`

The `fork` system call creates a new process that is a copy of the
current process. In both processes, program execution resumes just
after the call to fork, with one difference: in the newly created
process (the "child" process, to give it its proper name), the call
to fork returns a value of 0, whereas in the original process (the
"parent" process) fork returns with a value different from 0.

This means that in the child process the code inside the if-statement 
is not executed, and the child process therefore immediately terminates 
by `return`-ing 0. Meanwhile the parent process continues with the
`wait4()` and `kill()` calls. The `wait4()` call waits until the child 
process has terminated and removes the child process from the list
of running processes, whereas the very confusingly named `kill()` call
sends a signal to another process (the signal most often sent is the 
signal to terminate a running process, hence the name).

Now, since `wait4()` returns with the process id of the terminated child
process, `kill()` attempts to send a signal to that process, which no
longer exists. This results in the error code ESRCH (No such process).

After this, the `open()` call also fails, for a very different reason.
The open call attempts to open the file named from position 16 in the
`o` character buffer, which resolves to the name `"."`. In other words,
it attempts to open the current working directory. Moreover,
it attempts to open it with the value 0xc1 for the flags parameter.

The flags parameter is a set of options defining how the
file should be opened. The value 0xc1 is a combination of three options:
* `O_CREAT`
* `O_WRONLY`
* `O_EXCL`

Together they mean that the `open()` call attempts to create the file
if it does not exist, it opens the file for writing, and it opens the
file in exclusive mode, which means that it makes sure that the file is 
actually created by the current call to open. However, since the 
current directory already exists, this last part cannot be done, 
resulting in the error EEXIST (File exists).

And finally, the initial value of `b` is equal to 1, as `b` is the first 
argument to the `main` function. This argument counts the number of 
distinct parts in the command line with which the program was called,
which is 1 when a command is ran without any arguments: just the name
of the program itself.

All this means that my current age is the product of the numerical
values of ESRCH and EEXIST. These can be obtained by running
```
$ errno -l | grep -E "ESRCH|EEXIST"

ESRCH 3 No such process
EEXIST 17 File exists
```
