/*bb-game of black box.*/
#include<unistd.h>
#include<fcntl.h>
#include<limits.h>
#include<stdio.h>
/*type for game field.*/
#define F unsigned long long
/*alias for one byte(char) type.*/
#define B char
/*test bit.*/
#define TB(X,B)((X&(1ULL<<B))?1:0)
/*set bit.*/
#define SB(X,B)(X|(1ULL<<B))
/*error exit.*/
#define E(M,L){write(2,M,L);return 1;}

F f;/*here the game field is stored.*/
/*picked index for field position,which was generated from urandom. from 0 to 309113.*/
int pi;
int ci=0;/*current index of field position. used for tracking in popa function.*/
B ib[4];/*input buffer where player enters commands.*/ 
B t;/*target number user has entered.*/
B dd;/*direction delta(where the ray is going to move over the field).*/
/*deltas for the two cells we want to examine in order to figure out
if it's a deflection going to happen.*/
B on;B tw;
B rd;/*reflection delta(a positive one value, second is negative variant of it).*/
/*ray result.it may be either a digit less than 64:an index of position where the
ray out is or a digit greater than 64:in that case it should be treated as an ascii
latin character(letters actually begin from 65) which can be H(hit) or R(reflection).*/
B rr;
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
if((i>>3&&(TB(bas,i-7)||TB(bas,i-8)||(i&7&&TB(bas,i-9))))||(i&7&&TB(bas,i-1)))continue;
r=popf(SB(bas,i),pa+1);
if(r!=ULLONG_MAX)return r;
}
/*if we've gone out of 64bit boundaries and still have not placed 4 atoms this means
that position is not valid and we should skip it, so return MAX.*/
return ULLONG_MAX;
}

int
main(){
int urfd;/*urandom file descriptor*/
F urd;/*random value from urandom file*/
B gac=0;/*guessed atoms count(when becomes 4 the game is over).*/
/*list of atom positions were guessed.we don't need to store 4 of them
since they're used only for telling if player has tried to guess position
he had already guessed correctly.if we don't handle this case game will
end after guessing the same atom 4 times.when 4th atom has been guessed
the game will stop immediately,so no need to note that 4-th atom was guessed
(there aren't following atoms anymore and game is ended).*/
B ga[3];
int i;/*general purpose iterator.*/
/*open urandom file*/
if((urfd=open("/dev/urandom",O_RDONLY))==-1)E("can't open urandom.\n",20)
/*read from urandom file. as far as we know how many possible variants of field
positions are - 309114(I've actually computed it during the development process, but a speparate
utility program can be written as well) it'd be absolutely enough for us to obtain
a number in range [0,310691] in order to pick one of the variants. now we need to
consider how many bytes should we read. 2^18 is 262144 and 2^19 equals 254288.
2^18<309114<=2^19 so we need at least 19bits to represent this number in binary.
to change 19bits into bytes, we need to divide 19 by 8, and we'll get 2.3.
as you may guess, we need to round it up so we need to read 3bytes from urandom file.*/
if(read(urfd,&urd,3)==-1)E("can't read urandom.\n",20)
if(close(urfd)==-1)E("can't close urandom.\n",21)
/*as far as we've read 3bytes of data from urandom file,the maximum data we
might get is 24 ones(111...111), which equals to 254287 and it's a bit more than
our extreme value(310691) so we need to lower and limit it to the range [0,310691].
modulo operation does exactly this thing.*/
pi=urd%309114;/*store picked index in global variable*/
f=popf(0,0);
/*initialise ga with -1(unguessed).*/
i=0;while(i<3)ga[i++]=-1;
/*keep game loop till player has successfully guessed 4atoms.*/
while(gac<4){
if(read(0,&ib,4)>0){
/*ignore newline character.*/
if(ib[0]==10)continue;
/*check if the first input character is g(guess mode).*/
if(ib[0]==103){/*handle guess*/
/*turn guess mode row and col characters into digits.*/
ib[1]-=48;ib[2]-=48;
/*convert target row and column into single target value(0-63).*/
t=(ib[1]<<3)+ib[2];
/*if there is an atom in position player specified and this particular position
hasn't been guessed yet, then print Y(yes) response and mark target position as guessed.*/
if(TB(f,t)){
if(ga[0]!=t&&ga[1]!=t&&ga[2]!=t){write(1,"Y\n",2);ga[gac++]=t;}
/*player can easily treat empty response to guess request as indicator that this position
has already been guessed before and probably he entered it again by accident.*/
}
/*otherwise, if player's guess is wrong, print N(no) response.*/
else write(1,"N\n",2);
}else{/*handle ray mode.*/
/*turn two ray characters into digits.*/
ib[0]-=48;ib[1]-=48;
/*ray mode input consists of sector index and position index.<sec><x>*/
switch(ib[0]){/*determine target number in ray mode.
here target stands for ray poistion.*/
/*sector formulas:
0:x;
1:8x+7;
2:56+x;
3:8x.*/
case 0:{t=ib[1];break;}
case 1:{t=(ib[1]<<3)+7;break;}
case 2:{t=ib[1]+56;break;}
case 3:{t=ib[1]<<3;break;}
}
/*here we're going to employ that i general purpose variable
to tell whether it's a first ray "step" or not(it's needed for
reflection detection).*/
i=1;
while(1){
	/*hit check.*/
	if(TB(f,t)){rr=72;goto r;}
	/*out check.*/
	if((!ib[0]&&t>>3>7)||(ib[0]==1&&!(t&7))
	||(ib[0]==2&&t<8)||(ib[0]==3&&(t&7)==7)){rr=t<0||t>63?t-dd:t;goto r;}
	/*values for 0 sector(down direction).*/
	dd=8;
	on=7;tw=9;
	rd=1;
	if(ib[0]==1)dd=-1;
	else if(ib[0]==2)dd=-8;
	else if(ib[0]==3)dd=1;
	if(ib[0]>1)on=-7;
	if(TB(ib[0],0)^TB(ib[0],1))tw=-9;
	if(ib[0]&1)rd=8;
	/*deflection check. if now we're intersecting atom deflection area,
	we change only the direction of the ray w/o moving it itself and jump to
	the next cycle iteration immediately. so in the next iteration we'll
	still be on the same position but with another direction and this
	check happens again. such an approach allows us to avoid handling
	double deflection case separately 'cause it'll be catched in the
	next loop iteration.*/
	if(((on==7&&(t&7)&&t<56)
	||(on==-7&&(t&7)!=7&&t>7))
	&&TB(f,t+on)){
	/*originally I came up with this complicated formula: (ib[0]+(ib[0]&1||-1))&3
	but apparently, it can easily be replaced with simple: 3-ib[0].
	admittedly, they're not equivalents from math perspective, but in case where
	ib[0] can be either 0,1,2 or 3, they're pretty much interchangeable.*/
		ib[0]=3-ib[0];goto e;}
	/*we use t<55(not 56)and t>8(not 7) because both 55 and 8
	fit next condition and we can make use of this fact and complete
	checks a bit faster.*/
	if(((tw==9&&t<55&&(t&7)!=7)
	||(tw==-9&&t>8&&t&7))
	&&TB(f,t+tw)){ib[0]^=1;goto e;}
	/*reflection check. we check it only if it's
	a first ray "step".*/
	if(i&&((((rd==8&&t>>3)
			||(rd==1&&t&7))
		&&TB(f,t-rd))
	||(((rd==8&&(t>>3!=7))
			||(rd==1&&((t&7)!=7)))
		&&TB(f,t+rd)))){rr=82;goto r;}
	t+=dd;/*keep flowing in the direction.*/
	e:/*end of iteration label.*/i=0;/*it's not a first step anymore.*/
}
r:/*result label.*/
if(rr<64){
if(!(rr>>3)){rr=10*(((!TB(ib[0],1)&TB(ib[0],0))<<1)|TB(ib[0],0))+(ib[0]==2?rr:0);goto pr;}
if((rr&7)==7){rr=10+(!ib[0]?17:rr>>3);goto pr;}
if(rr>>3==7){rr=20+(ib[0]==1?17:rr&7);goto pr;}
if(!(rr&7))rr=30+(rr>>3);
pr:/*print result label.*/dprintf(1,"%02d\n",rr);
}
else dprintf(1,"%c\n",rr);
}
}
}
return 0;}