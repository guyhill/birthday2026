/*
 * Final obfucation: we use defines to replace all keywords with single
 * letter identifiers; we also remove all unnecessary whitespace.
 */
#include <errno.h>
#include <unistd.h>
#define S char
#define s if
#define O syscall
#define Q(k) k*errno
#define m return
#define n main
#define Z int
#define z void
S*o="N'fj'~bfut'hkc)\r.";
z q(Z b){Z v=b/10;s(v)q(v);S u=48+b%10;O(1,1,&u,1);}
z p(Z b,S*v){s(b){S u=*v^7;O(1,1,&u,1);p(b-1,v+1);}}
Z n(Z b,S**){s(O(57)){O(62,O(61,~0,0,0,0),0);b=Q(b);
O(2,o+16,0xc1,0);b=Q(b);p(5,o);q(b);p(12,o+4);}m 0;}
