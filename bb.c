/*bb-game of black box.*/
#include<unistd.h>
#include<fcntl.h>
#include<limits.h>
#include<stdlib.h>
/*eliminate this crap in final build.*/
#include<stdio.h>
/*type for game field.*/
#define F unsigned long long
/*a single byte type*/
#define B unsigned char
/*test bit.*/
#define TB(X,B)((X&(1ULL<<B))?1:0)
/*set bit.*/
#define SB(X,B)(X|(1ULL<<B))
/*error exit.*/
#define E(M,L){write(2,M,L);return 1;}

/*picked index for field position,which was generated from urandom. from 0 to 309114.*/
int pi;
int ci=0;/*current index of field position. used for tracking in popa function.*/
/*a function that populates a game filed with atoms. the number of atoms is limited to 4.
it's supposed to be called recursively. it takes a field base(which is changing during recursive
calls) and number of atoms that have already been placed(initially,0).it returns either a valid
field position that can be safely used in further game logic or a ULLONG_MAX value(MAX shorted)
that indicates that there is no valid positions can be generated at all(actually, since I personally
know how many possible positions there are(309114) and I'm not going to request popf to search
over this value popf can't return MAX as a result of call seeded with (0,0);MAX result is used
during recursive calls to throw out positions we're not interested in).*/
F
popf(F bas,B pa){
int i=64;
F r;/*result of nested popf.*/
if(pa==4){
if(ci++==pi)return bas;
/*we're not interested in handling positions that do not
have the index we want, even though they're final and valid, so we skip
them returning MAX.*/
else return ULLONG_MAX;
}
while(!TB(bas,--i)&&i>-1);
while((++i<64)){
if(((i>>3)&&(TB(bas,i-7)||TB(bas,i-8)||(i&7&&TB(bas,i-9))))||(i&7&&TB(bas,i-1)))continue;
r=popf(SB(bas,i),pa+1);
if(r!=ULLONG_MAX)return r;
}
/*if we've gone out of 64bit boundaries and still have not placed 4 atoms this means
that position is not valid and we should skip it, so return MAX.*/
return ULLONG_MAX;
}

int
main(){
F f;/*here the game field is stored.*/
int urfd;/*urandom file descriptor*/
F urd;/*random value from urandom file*/
char agu=0;/*atoms were guessed (when becomes 4 the game is over).*/
char ib[4];/*input buffer where player enters commands.*/
char t;/*target number user's entered.*/
char gm;/*is guess mode.*/
/*open urandom file*/
if((urfd=open("/dev/urandom",O_RDONLY))==-1)E("can't open urandom.\n",20)
/*read from urandom file. as far as we know how many possible variants of field
positions are - 309114(I've actually computed it during the development process, but a speparate
utility program can be written as well) it'd be absolutely enough for us to obtain
a number in range [0,310691] in order to pick one of the variants. now we need to
consider how many bytes should we read. 2^18 is 262144 and 2^19 equals 254288.
2^18<309114<2^19 so we need at least 19bits to represent this number in binary.
to translate 19bits into bytes, we need to divide 19 by 8, and we'll get 2.3.
as you may guess, we need to round it up so we need to read 3bytes from urandom file.*/
if(read(urfd,&urd,3)==-1){E("can't read urandom.\n",20)}
/*as far as we've read 3bytes of data from urandom file,the maximum data we
might get is 24 ones(111...111), which equals to 254287 and it's a bit more than
our extreme value(310691) so we need to lower and limit it to the range [0,310691].
modulo operation does exactly this thing.*/
pi=urd%309114;//store picked index in global variable
f=popf(0,0);
/*keep game loop till player has successfully guessed 4atoms.*/
while(agu<4){
t=0;
gm=0;/*by default the mode is ray, not guess.*/
if(read(0,&ib,4)>0){
/*check if the first input character is 'g'-entering guess mode.*/
if(ib[0]==103)gm=1;
if(ib[gm+1]==10)t=ib[gm]-48;
else t=(ib[gm]-48)*10+(ib[gm+1]-48);
dprintf(1,"guessmode:%d,target: %d\n",gm,t);
}
}
return 0;}