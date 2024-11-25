/*bb -- game of black box.*/


#include<unistd.h>
#include<fcntl.h>
#include<limits.h>
#include<stdio.h>


/*
	set this flag up and recompile the program in order
	to enter a debug mode. more exactly, it'll print an internal
	representation of a field (which pretty much readable and intuitive)
	and gives you an opportunity to use MF macro (which is for "Mock Field"),
	using which you can place atoms in desirable positions.
*/
#define DBG 0


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

#if DBG
#define MF(A,B,C,D,E)f=1ULL<<A|1ULL<<B|1ULL<<C|1ULL<<D|1ULL<<E
#endif


/*here the game field is stored.*/
F f;

/*
	picked index for field position,which was generated
	from random(4) device (we'll use /dev/urandom, 'cause it's
	an unblocking one).
	its value is from 0 to 7624511.
*/
int pi;

/*
	current index of field position.
	used for tracking in popa function.
*/
int ci=0;

/*
	input buffer where player enters commands.
	we actually need only 4 characters:
	1, 2 -- for an input number itself (row+col / sector+cell)
	3 -- for a newline character
	4 -- for a NULL-terminator.
*/
B ib[4];

/*target number user has entered.*/
B t;

/*direction delta (where the ray is going to move over the field).*/
B dd;

/*
	deltas for the four cells we want to examine in order to
	figure out if it's a deflection going to happen.
	one, two, three, four - in this exact order we will check them.
	one and two - are two horizontal neighbours,
	three and four - are front-diagonal ones.
*/
B on;B tw;
B th;B fo;

/*
	reflection delta. a positive one value; second is
	negative variant of it.
*/
B rd;

/*
	ray result.
	it may be either a digit less than 64: an index of position
	where the ray out is
	or a digit greater than 64: in that case it should be treated
	as an ASCII latin character (letters actually begin from 65)
	which can be H(hit) or R(reflection).
*/
B rr;

#if DBG
	/*print field function.*/
	void pf(){
	int r=0;
	dprintf(1,"  0 1 2 3 4 5 6 7\n");
	while(r<8){
	int c=0;
	dprintf(1,"%d",r);
       	while(c<8){
       	dprintf(1," %d",TB(f,8*r+c));
       	if(c==7)dprintf(1,"\n");
       	++c;
       	}
	++r;
	}
	}
#endif


/*
	a function that populates a game filed with atoms. the number
	of atoms is limited to 5. it's supposed to be called recursively.
	it takes a field base(which is changing during recursive calls)
	and number of atoms that have already been placed(initially,0).
	it returns either a valid field position that can be safely used
	in further game logic or a ULLONG_MAX value(MAX shorted) that
	indicates that there is no valid positions can be generated
	at all(actually, since I personally know how many possible
	positions there are(7624512) and I'm not going to request
	popf to search over this value popf can't return MAX as
	a result of call seeded with (0,0);MAX result is used during
	recursive calls to throw out positions we're not interested in).
*/
F
popf(F bas,B pa){
int i=64;

/*result of nested popf.*/
F r;
if(pa==5){
if(ci++==pi)return bas;
/*
	we're not interested in handling positions that do not
	have the index we want, even though they're final and
	valid, so we skip them returning MAX.
*/
else return ULLONG_MAX;
}
while(!TB(bas,--i)&&i>-1);
while((++i<64)){
r=popf(SB(bas,i),pa+1);
if(r!=ULLONG_MAX)return r;
}
/*
	if we've gone out of 64 bit boundaries and still have not
	placed 5 atoms this means that position is not valid and we
	should skip it, so return MAX.
*/
return ULLONG_MAX;
}

int main(){
/*/dev/urandom file descriptor.*/
int urfd;

/*random value from urandom file.*/
F urd;

/*
	guessed atoms count(when becomes 5 the game
	is over and program halts).
*/
B gac=0;

/*
	list of atom positions were guessed.we don't need to
	store 5 of them since they're used only for telling
	if player has tried to guess position he had already
	guessed correctly.if we don't handle this case game will
	end after guessing the same atom 5 times.when 5th atom
	has been guessed the game will stop immediately,so no
	need to note that 5th atom was guessed
	(there aren't following atoms anymore and game is ended).
*/
B ga[5];

/*general purpose iterator.*/
int i;

/*open /dev/urandom special file.*/
if((urfd=open("/dev/urandom",O_RDONLY))==-1){
	E("can't open urandom.\n",20)
}

/*
	read from random file. as far as we know
	how many possible variants of field positions are - 7624512
	(I've actually computed it during the development process,
	but a speparate utility program can be written as well) it'd
	be absolutely enough for us to obtain a number in range
	[0,7624511] in order to pick one of the variants. now we need
	to decide how many bytes should we read. 2^22 is 4194304 and
	2^23 equals 8388608. 2^22<7624512<=2^23 so we need at least 23
	bits to represent this number in binary. to change 23 bits into
	bytes, we need to divide 23 by 8, and we'll get 2.8. as you may
	guess, we need to round it up so we need to read 3 bytes from
	/dev/urandom file.
*/
if(read(urfd,&urd,3)==-1){
	E("can't read urandom.\n",20)
}

/*do not forget to close files.*/
if(close(urfd)==-1){
	E("can't close urandom.\n",21)
}

/*
	as far as we've read 3 bytes of data from urandom file, the
	maximum data we might get is 24 ones (111...111), which equals
	to 16777215 and it's a bit more than our extreme value(7624511)
	is, so we need to lower and limit it to the range [0,7624511].
	modulo operation does exactly this thing.
*/

/*store picked index in global variable.*/
pi=urd%7624512;

#if DBG
	/*
		here you can give a particular value for `pi`
		variable, which holds an index of position to be generated.
	*/
#endif

/*generate our field (this functions uses global `pi` variable).*/
f=popf(0,0);

#if DBG
	/* put you MF macro call here if you want. */
	pf();
#endif

/*initialize ga with -1(unguessed).*/
i=0;while(i<3)ga[i++]=-1;

/*keep game loop till player has successfully guessed 5 atoms.*/
while(gac<5){
/*read player input.*/
if(read(0,&ib,4)>0){
/*ignore newline character.*/
if(ib[0]==10||ib[1]==10)continue;

/*check if the first input character is 'g'(guess mode).*/
if(ib[0]==103){
/*handle guess mode from here.*/

/*turn guess mode row and col characters into digits.
you rememeber, that 48 is an ASCII code for '0'.*/
ib[1]-=48;ib[2]-=48;

/*convert target row and column into single target value(0-63).
it's important, that we assume input to be a combination of
row and column. and since out field is a 8-side square, together
the form a two-digit octal number. and we convert this octal number
into decimal one.*/
t=(ib[1]<<3)+ib[2];

/*if there is an atom in position player specified and this
particular position hasn't been guessed yet, then print
Y(yes) response and mark target position as guessed.*/
if(TB(f,t)){
if(ga[0]!=t&&ga[1]!=t&&ga[2]!=t&&ga[3]!=t){
	write(1,"Y\n",2);
	ga[gac++]=t;
}
/*
	note: player can easily treat empty response to guess request
	as indicator that this position has already been guessed
	before and probably he entered it again by accident.
*/
}
/*otherwise, if player's guess is wrong, print N(no) response.*/
else write(1,"N\n",2);
}
/*handle ray mode from here.*/
else{
/*turn two ray characters into digits.*/
ib[0]-=48;ib[1]-=48;

/*ray mode input consists of sector index and position index.<sec><x>*/
/*determine target number in ray mode.
here target stands for ray poistion.*/
switch(ib[0]){
/*
	sector formulas:
	0:x;
	1:8x+7;
	2:56+x;
	3:8x.
*/
case 0:{t=ib[1];break;}
case 1:{t=(ib[1]<<3)+7;break;}
case 2:{t=ib[1]+56;break;}
case 3:{t=ib[1]<<3;break;}
}

/*here we're going to employ that `i` general purpose variable
to tell whether it's a first ray "step" or not(it's needed for
reflection detection).*/
i=1;

while(1){
	/*initial values for 0 sector(down direction).*/
	dd=8;
	on=-1;tw=1;
	th=7;fo=9;
	rd=1;
	
	/*set dd.*/
	if(ib[0]==1)dd=-1;
	if(ib[0]==2)dd=-8;
	if(ib[0]==3)dd=1;
	
	/*set on,tw,th,fo.*/
	if(ib[0]>1)th=-7;
	if(TB(ib[0],0)^TB(ib[0],1))fo=-9;
	if(ib[0]&1){on=-8;tw=8;}
	
	/*set rd.*/
	if(ib[0]&1)rd=8;
	
	/*reflection check. we check it only if it's
	a first ray "step".*/
	if(i&&((((rd==8&&t>>3)
			||(rd==1&&t&7))
		&&TB(f,t-rd))
	||(((rd==8&&(t>>3!=7))
			||(rd==1&&((t&7)!=7)))
		&&TB(f,t+rd)))){rr=82;goto r;}
	
	/*hit check.*/
	if(TB(f,t)){
		rr=72;
		goto r;
	}
	
	/*out check.*/
	if(
		(!ib[0]&&t>>3>7)||(ib[0]==1&&!(t&7))
		||(ib[0]==2&&t<0)||(ib[0]==3&&(t&7)==7)
	) {
		rr=t<0||t>63?t-dd:t;
		goto r;
	}
	
	/*
		deflection check. if now we're intersecting atom
		deflection area, we change only the direction of
		the ray w/o moving it itself (as you can see, we're
		doing it right in-place, by means of modifying input
		buffer first char, which, as you remember, is in charge
		of sector we've shot the ray from - within this loop
		we treat it as a current direction) and jump to the
		next cycle iteration immediately. so in the next
		iteration we'll still be on the same position but
		with another direction and this check happens again.
		such an approach allows us to avoid handling double
		deflection case separately 'cause it'll be catched
		in the next loop iteration.
	*/
	
	/*
		first we chech horizontal cells, then diagonal
		([on,tw], [th,fo]).
		
		on -- always negative numbers (-1, -8),
		tw -- always positive (1, 8).
		
		th -- always 7's (+/-),
		fo -- always 9's (+/-).
	*/
	
	/*handle "on".*/
	if(
		(((on==-1)&&(t&7))
		||
		((on==-8)&&(t>7)))
		&&
		TB(f,t+on)
	) {
		ib[0]=(on==-8?0:3);
		goto e;
	}
	
	/*handle tw.*/
	if(
		(((tw==1)&&((t&7)!=7))
		||
		((tw==8)&&(t<56)))
		&&
		TB(f,t+tw)
	) {
		ib[0]=(tw==8?2:1);
		goto e;
	}
	
	/*handle th.*/
	if((((th==7)&&(t&7)&&t<56)
	||(th==-7&&(t&7)!=7&&t>7))
	&&TB(f,t+th)){
		/*
		originally I came up with this complicated formula:
		(ib[0]+(ib[0]&1||-1))&3, but apparently, it can easily be
		replaced with simple: 3-ib[0]. admittedly, they're not
		equivalents from math perspective, but in case where `ib[0]`
		can be either 0,1,2 or 3, they're pretty much interchangeable.
		*/
		ib[0]=3-ib[0];
		goto e;
	}
	
	/*
		handle fo.
		we use t<55(not 56)and t>8(not 7) because both 55 and 8
		fit next condition and we can make use of this fact and complete
		checks a bit faster.
	*/
	if(((fo==9&&t<55&&(t&7)!=7)
	||(fo==-9&&t>8&&t&7))
	&&TB(f,t+fo)){
		ib[0]^=1;
		goto e;
	}
	
	/*keep flowing in the direction.*/
	t+=dd;
	
	/*end of iteration label.*/
	e:
	/*it's not a first step anymore.*/
	i=0;
}

/*result label.*/
r:
if(rr<64){
if(!(rr>>3)){
	rr=10*(((!TB(ib[0],1)&TB(ib[0],0))<<1)|TB(ib[0],0))+(ib[0]==2?rr:0);
	goto pr;
}
if((rr&7)==7){
	rr=10+(!ib[0]?17:rr>>3);
	goto pr;
}
if(rr>>3==7){
	rr=20+(ib[0]==1?17:rr&7);
	goto pr;
}
if(!(rr&7)){
	rr=30+(rr>>3);
}
pr:/*print result label.*/
dprintf(1,"%02d\n",rr);
}
else dprintf(1,"%c\n",rr);
}
}
}
return 0;
}
