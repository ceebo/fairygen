/************************************************************************/
/*         Pieces-Only Tablebase Generator, by H.G. Muller              */
/************************************************************************/
/* Count to conversion, 8-symmetry, but uncompacted. 2-pass strategy.   */

#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION "1.2"
#define MEN 3
#define DIAGSYM
#define XWHITEDUPLE
#define CONV conv
#define SIZE (1<<6*MEN-2)
#define MAXDIR 24
#define MAX MEN+1
#define BOX 0x80
#define LABEL (~1)
#define RANKS 07070707070
#define FILES 00707070707
#define TICK (1./CLOCKS_PER_SEC)
#define board (bord+9)                  /* to allow negative index      */

/* piece names (K must have highest code of all leapers (=non-sliding)  */
#define K  17 /* King     (compound W+F)   */
#define Q  21 /* Queen    (compound R+B)   */
#define A  22 /* Archbis  (compound N+B)   */
#define R  20 /* Rook       = (1,0) rider  */
#define B  19 /* Bishop   * = (1,1) rider  */
#define N  9  /* Knight     = (1,2) leaper */
#define W  4  /* Wazir      = (1,0) leaper */
#define F  3  /* Ferz     * = (1,1) leaper */
#define H  18 /* Nightrider = (1,2) rider  */
/* The following pcs need additional directions in boardvec[], basevec[]*/
#define E  1  /* Elephant * = (2,2) leaper */
#define D  2  /* Dabbabah * = (2,0) leaper */
#define C  5  /* Camel    * = (1,3) leaper */
#define Z  8  /* Zebra      = (2,3) leaper */
#define G  7  /* Giraffe    = (1,4) leaper */
#define L  6  /* Flamingo   = (1,6) leaper */
#define WD 23
#define FA 24
#define DK 25 /* Dragon */
#define LT 26 /* Lieutenant */
#define BD 27
#define FAD 28
#define WA 29
/* The following compound pieces we will need larger codevec[] (>8 dirs)*/
#define NZ 11 /*          (compound N+Z)   */
#define GN 12 /* Gnu      (compound N+C)   */
#define BI 13 /* Bison    (compound C+Z)   */
#define BU 14 /* Buffalo  (compound N+C+Z) */
#define CT 15 /* Centaur  (compound W+F+N) */
#define SQ 16 /* Squirrel (compound E+D+N) */
#define RV 21 /* Raven    (compound R+H)   */
/* The following rider-leaper compounds need direction-dependent range  */
            /* Cardinal (compound B+N)   */
            /* Caliph   (compound B+C)   */
            /* Dragon   (compound B+W)   */
            /* Chancellor (compound R+N) */
            /* DragonKing (Compound R+F) */
            /* Canvasser  (compound R+C) */
            /* Amazon   (compound R+B+N) */
/* Note: which pieces are royal is determined by order, not type:       */
/* a K that is not first or last is treated as a Commoner               */

int nblack = 1;                         /* number of black pieces       */
int piece[MAX] = {K,R,K};            /* desired end-game:            */
char pcode[] = { 4,10,2,8,4};
int pos[MAX];                           /*      black pieces first,     */
int stride[MAX];                        /*      kings first and last!   */
char exch;
int conv;
int inhom, cs1, cs2, cs3;               /* color symmetry for bishops   */

char bord[138];
int tbindex;
int cnt, sample1, sample2;
clock_t t;
unsigned char *tb;
int best, nbest, mine, xboard, transform, move, staleMate=1, nullMove, dtc=1;
int mcnt, lcnt, vcnt, ccnt, bcheck;
int histo[600];
char xh[20],buf[20], *fname;

int boardvec[] = {1,-1,16,-16,0,
        14,-14,18,-18,31,-31,33,-33,    /* 5  */
        2,-2,32,-32,30,-30,34,-34,0,    /* 13 */
        14,-14,18,-18,31,-31,33,-33,    /* 22 */
        1,-1,16,-16,15,-15,17,-17,0,    /* 30 */
        1,-1,16,-16,2,-2,32,-32,0,      /* 39 */
        13,-13,19,-19,47,-47,49,-49,    /* 48 */
        14,-14,18,-18,31,-31,33,-33,    /* 56 */
        29,-29,35,-35,46,-46,50,-50,0,  /* 64 */
        29,-29,35,-35,46,-46,50,-50,    /* 73 */
        13,-13,19,-19,47,-47,49,-49,0,  /* 81 */
        10,-10,22,-22,95,-95,97,-97,0,  /* 90 */
        12,-12,20,-20,63,-63,65,-65,0,  /* 99 */
        13,-13,19,-19,47,-47,49,-49,    /* 108*/
        14,-14,18,-18,31,-31,33,-33,0,  /* 116*/
        15,-15,17,-17,0,                /* 125*/
        15,-15,17,-17,                  /* 130*/
        14,-14,18,-18,31,-31,33,-33,0,  /* 134*/
        15,-15,17,-17,                  /* 143*/
        1,-1,16,-16,0,                  /* 147*/
        1,-1,16,-16,-2,32,-32,2,0,      /* 152*/
	1,-1,                           /* 161*/
        15,-15,17,-17,30,-30,34,-34,0,  /* 163*/
	1,-1,16,-16,15,-15,17,-17,0,    /* 172*/
	2,-2,32,-32,15,-15,17,-17,0,    /* 181*/
	2,-2,32,-32,                    /* 190*/
        15,-15,17,-17,30,-30,34,-34,0,  /* 194*/
	1,-1,16,-16,30,-30,34,-34,0,    /* 203*/
//        15,-15,17,-17,
30,-30,34,-34,1,-1,16,-16,0,    /* 161*/
        1,-1,16,-16,15,-15,17,-17,0}; /* step vectors for 0x88 board  */
int basevec[] = {1,-1,8,-8,0,
        6,-6,10,-10,15,-15,17,-17,
        2,-2,16,-16,14,-14,18,-18,0,
        6,-6,10,-10,15,-15,17,-17,
        1,-1,8,-8,7,-7,9,-9,0,
        1,-1,8,-8,2,-2,16,-16,0,
        5,-5,11,-11,23,-23,25,-25,
        6,-6,10,-10,15,-15,17,-17,
        13,-13,19,-19,22,-22,26,-26,0,
        13,-13,19,-19,22,-22,26,-26,
        5,-5,11,-11,23,-23,25,-25,0,
        2,-2,14,-14,47,-47,49,-49,0,
        4,-4,12,-12,31,-31,33,-33,0,
        5,-5,11,-11,23,-23,25,-25,
        6,-6,10,-10,15,-15,17,-17,0,
        7,-7,9,-9,0,
        7,-7,9,-9,
        6,-6,10,-10,15,-15,17,-17,0,
        7,-7,9,-9,
        1,-1,8,-8,0,
        1,-1,8,-8,-2,16,-16,2,0,
	1,-1,
        7,-7,9,-9,14,-14,18,-18,0,
	1,-1,8,-8,7,-7,9,-9,0,
	2,-2,16,-16,7,-7,9,-9,0,
	2,-2,16,-16,
        7,-7,9,-9,14,-14,18,-18,0,
	1,-1,8,-8,14,-14,18,-18,0,
//        7,-7,9,-9,
14,-14,18,-18,1,-1,8,-8,0,
        1,-1,8,-8,7,-7,9,-9,0};   /* step vectors for 64-board    */
int mode[] = {3,3,3,3,3,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,0,
        3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,0,
        7,7,7,7,0,
        7,7,7,7,3,3,3,3,3,3,3,3,0,
        7,7,7,7,7,7,7,7,0,
3,3,3,3,3,3,3,3,0,
2,2,
3,3,3,3,3,3,3,3,0,
7,7,7,7,3,3,3,3,0,
3,3,3,3,7,7,7,7,0,
3,3,3,3,
3,3,3,3,3,3,3,3,0,
3,3,3,3,3,3,3,3,0,
3,3,3,3,3,3,3,3,2,2,2,2,0,
        1,1,1,1,2,2,2,2,0,
        2,2,2,2,1,1,1,1,0
        };
int first[] = {4,17,43,34,0,81,90,134,64,116,125, 56,108,73,48,22,5,30,116,125,147, 143,130,152,163,172,161,181,190,203}; /* vector directory per piece */
int codevec[MAXDIR*MEN];                /* step vectors in TB index     */

char cc[]=".edxwcfgzN*YGZUCSKHBRQADEGLXFW";
char bis[]={ 0,1,1,1,0,1,0,0,0,0,1, 0,0,0,0,0,0,0,0,1,0, 0,0,0,1,0,0,1,1,0};        /* color-resricted pcs(marked *)*/

char bcode[] = {
2,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,
0,2,0,0,0,0,0,1,0,0,0,0,0,2,0,0,
0,0,2,0,0,0,0,1,0,0,0,0,2,0,0,0,
0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,0,
0,0,0,0,2,0,0,1,0,0,2,0,0,0,0,0,
0,0,0,0,0,2,8,1,8,2,0,0,0,0,0,0,
0,0,0,0,0,8,6,5,6,8,0,0,0,0,0,0,
1,1,1,1,1,1,5,0,5,1,1,1,1,1,1,0,
0,0,0,0,0,8,6,5,6,8,0,0,0,0,0,0,
0,0,0,0,0,2,8,1,8,2,0,0,0,0,0,0,
0,0,0,0,2,0,0,1,0,0,2,0,0,0,0,0,
0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,0,
0,0,2,0,0,0,0,1,0,0,0,0,2,0,0,0,
0,2,0,0,0,0,0,1,0,0,0,0,0,2,0,0,
2,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0
};

char deltavec[] = {
-17,0,0,0,0,0,0,-16,0,0,0,0,0,0,-15,0,
0,-17,0,0,0,0,0,-16,0,0,0,0,0,-15,0,0,
0,0,-17,0,0,0,0,-16,0,0,0,0,-15,0,0,0,
0,0,0,-17,0,0,0,-16,0,0,0,-15,0,0,0,0,
0,0,0,0,-17,0,0,-16,0,0,-15,0,0,0,0,0,
0,0,0,0,0,-17,0,-16,0,-15,0,0,0,0,0,0,
0,0,0,0,0,0,-17,-16,-15,0,0,0,0,0,0,0,
-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,0,
0,0,0,0,0,0,15,16,17,0,0,0,0,0,0,0,
0,0,0,0,0,15,0,16,0,17,0,0,0,0,0,0,
0,0,0,0,15,0,0,16,0,0,17,0,0,0,0,0,
0,0,0,15,0,0,0,16,0,0,0,17,0,0,0,0,
0,0,15,0,0,0,0,16,0,0,0,0,17,0,0,0,
0,15,0,0,0,0,0,16,0,0,0,0,0,17,0,0,
15,0,0,0,0,0,0,16,0,0,0,0,0,0,17,0
};

int attack(int a, int v)
{
    int x=pos[a], y=pos[v], r, c;

    c = bcode[y-x+0x77];
    if(!(c&pcode[a])) return 0;
    if(c&4 | c&8) return 1;
    r = deltavec[y-x+0x77];
    while(!board[x+=r]);
    return x==y;
}
#if 0
int classify(int index)
{
    if(attack(0,4)) return 130; // Illegal
    if(attack(1,4)) return 131; // K-capt
    if(attack(4,1) && attack(2,0) && !attack(1,2)) return 132;
    if(attack(4,1) && attack(3,0) && !attack(1,3)) return 133;
    if(attack(2,1) && attack(3,0) && !attack(1,3)) return 134;
    if(attack(3,1) && attack(2,0) && !attack(1,2)) return 135;
    if(attack(2,1) && attack(2,0) && (attack(3,2) ||
    return 129;
}
#endif
void zgen(int start, int stop, int task, int n, int tbind, int symmask, int diag)
/* move generator, for both forward and backward moves. The forward     */
/* moves can capture enemy pieces, the backward moves can 'uncapture'   */
/* i.e. leave any of the captured foes on the square they 'came from',  */
/* or leave it empty. Uncapturing a king is mandatory (finds checks).   */
/* THERE ARE NO PROVISIONS FOR PAWNS WHATSOEVER; PIECES ONLY!           */
{
    int i, j, k, *p, nwind, vec, block, stind, unind, ustart,
        b, u, uu, h, h2=0666, tbadr, nwadr, nwmask, nwdiag, twice=0, mask2;
#ifdef WHITEDUPLE
	int duple=0;
#endif
    stind = unind = tbind;              /* TB index of origin position  */
    tbadr = tbind ^ symmask;
    ustart = start ? start : MEN-1;     /* end of opponents piece list  */
    u = ustart; uu = -1;                /* normally start w/o uncapture */
    if(task==0)                         /* called to generate in-checks */
        u = 0;                          /* request uncapture black king */
    do{                                 /* loop over pieces to uncapture*/
        for(i=start; i<stop; i++)       /* loop through own pieces      */
        {
            if(i!=MEN-1 && pos[i]==pos[MEN-1])
                continue;               /* piece is captured            */
            j = first[piece[i]];
            p = codevec + MAXDIR*i;
            board[pos[i]] = 0;          /* clear start square           */
            if(u<ustart)                /* uncapture requested          */
            {   pos[u] = pos[i];        /* make uncapt. piece appear    */
                board[pos[i]] = u+1;    /* and put it on the board      */
                /* and encode its presence in the TB index:             */
                stind = unind + ((pos[u] + (pos[u]&7))>>1)*stride[u];
                uu = u;
                if(dtc && pos[1]!=pos[MEN-1] && 
                   pos[MEN-2]!=pos[MEN-1]&&pos[MEN-3]!=pos[MEN-1]
                  )
                    n = CONV;
            }
            while(mode[j])
            {
                vec = boardvec[j++];
                b = k = pos[i];         /* start ray from current square*/
                nwind = stind;          /* and current TB index         */
                if(task<3 && (uu>=0 && (mode[j-1]&1)==0
                            || uu<0 && (mode[j-1]&2)==0)) { p++; continue; }
                do{
                nwmask = symmask; nwdiag = diag;
                    k += vec;           /* next square                  */
                    if(k & 0x88) break; /* off-board move               */
                    block = board[k];   /* occupant of target square    */
                    if(block &&         /* square not empty             */
                        (task<3         /* is invalid for backward move */
                      || block<mine)    /* or forward capture own piece */
                      ) break;
                    if(task>=3 && (block&&!(mode[j-1]&1) || !block&&!(mode[j-1]&2))) break;
                    nwind += *p;        /* move piece also in TB        */
                    if(block)           /* for (forward) captures       */
                    {   /* mov capt. piece in TB to 'prison' (= wK pos) */
                        nwind -= stride[block-1]*((k+(k&7)>>1)
                                 -(pos[MEN-1]+(pos[MEN-1]&7)>>1));
                    }
                    if(i>MEN-3)         /* white, worry about symmetry  */
                    {
                      nwmask = twice = 0;
                      if(i==MEN-1)    /* wK, drag along capt. pieces  */
                      { for(h=1;h<MEN-1;h++) 
                            if(h!=uu && pos[h]==pos[MEN-1])
                                nwind += basevec[j-1]*stride[h];
                        /* symmetry reduction, make sure wK in quadrant */
                        if(nwind&040<<6*MEN-6) nwmask  = RANKS&~(-1<<6*MEN);
                        if(nwind&004<<6*MEN-6) nwmask ^= FILES&~(-1<<6*MEN);
                      }
                      nwadr = nwind ^ nwmask;
                      mask2 = nwmask;   /* mask for no diagonal mirror  */
                      /* let mask also move most sign. bit to hole      */
                      if(nwadr&020<<6*MEN-6) mask2 ^= 024<<6*MEN-6;
#ifdef DIAGSYM
                      h = (nwadr>>6*MEN-6&7) - (nwadr>>6*MEN-3);
                      if(h<=0)
#else
		      if(0)
#endif
                      { /* treat wK & 1st other both on diag in 2 symm. */
                        h2 = (nwadr>>6*MEN-12&7) - (nwadr>>6*MEN-9&7);
                        twice = h==0 && h2==0 && (tbind>>3^tbind)&0707<<6*MEN-12;
                        if(h<0 || h2<0 || twice)
                        {   nwdiag=8*MEN;
                            if(nwadr&02<<6*MEN-6) nwmask ^= 042<<6*MEN-6;
                        }
                        else nwmask = mask2;
                      } else nwmask = mask2;
                    }
again:
                    nwadr = nwind ^ nwmask;
                    if(nwdiag) nwadr = (nwadr&RANKS)>>3 | (nwadr&FILES)<<3;
                    switch(task)        /* process move as requested    */
                    {
                        case 0:
                        case 1:
                        /* mark mothers as win.wtm and go on to grandm. */
                            if(!(tb[nwadr]&1))  /* only if not done yet */
                            {   tb[nwadr] |= 1; /* set win.wtm bit      */
                                mcnt++; sample1 = nwind;
                                /* put piece on board & recursion       */
                                board[pos[i]=k] = i+1;
                                if(i==MEN-1)    /* 'drag along' capt.   */    
                                {   for(h=1;h<MEN-1;h++)
                                        if(h!=uu && pos[h]==b)
                                            pos[h]=k;
                                }
                                zgen(0, start, 2, n, nwind, nwmask, nwdiag);
                                if(i==MEN-1)        
                                {   for(h=1;h<MEN-1;h++)
                                        if(h!=uu && pos[h]==k)
                                            pos[h]=b;
                                }
                                board[k] = 0; pos[i] = b;
				if(staleMate==2 && tb[nwadr] == 0xFF) tb[nwadr] = 2*n+1; // convert stalemate to loss
				if(nullMove && tb[nwadr] == 1) tb[nwadr] = 2*n+1;
                            }
                            if(twice)
                            {   twice = 0; nwmask = mask2; nwdiag = diag;
                                goto again; /* redo unmirrorred         */
                            }
                            break;
                        case 2:
                        /* label grandmothers as potential win(n).btm   */
                            if(tb[nwadr] < 2)
                            {   tb[nwadr] |= n<<1; lcnt++; vcnt++; }
                            break;
                        case 3:
                        /* check daughters, if all win.wtm              */
                            ccnt++;
                            if(block==MEN||!(tb[nwadr]&1))
                            {
                                /* black can capt wK: never win(n).btm  */
                                if(block==MEN) {
#ifdef WHITEDUPLE
				    // not so fast! King capture could still be losing if he has two...
				    if(pos[MEN-1] != pos[MEN]) { // and he has two...
					if(!duple) { // this is only the first one we can capture
					    duple = 1 + (tb[nwadr]&1); // remember if letting this king captured is losing
					    break;
					}
#endif
				    
				    tb[tbadr] = LABEL-2 | tb[tbadr] & 1;
				} else
				tb[tbadr] &= 1; vcnt--;
                                board[b] = i+1;
                                return; /* one non-win reachable, abort */
                            }
                            break;
                        case 4:
                            if(block>nblack) break;
			    { int bb = b, kk = k, s = tb[nwadr]>>1;
			      if(block == 1) strcpy(buf, "  # "); else
			      if(s == 126) strcpy(buf, "  - "); else
			      if(s) sprintf(buf, "%3d ", (tb[nwadr]>>1)-CONV); else s = 999, strcpy(buf, "draw");
			      if(transform < 0) bb = (b&7)<<4 | (b&0x70)>>4, kk = (k&7)<<4 | (k&0x70)>>4;
			      if(transform & 1) bb ^= 7, kk ^= 7;
			      if(transform & 8) bb ^= 0x70, kk ^= 0x70;
                              printf("%s%c%d%c%d:\t%s\t%10o\n",xh,(bb&7)+'a',(bb>>4)+1,(kk&7)+'a',(kk>>4)+1, buf, nwadr), fflush(stdout);
                              if(block == 1 || s<nbest)
                              { nbest = block == 1 ? 0 : s;
                                best = nwadr;
				move = 256*bb + kk;
                              }
			    }
                    }

                    block += (mode[j-1]&4) == 0;/* fake capt. to terminate PNK */
                } while(!block);        /* continue ray if not blocked  */
                p++;                    /* next ray, nex TB move vector */
            }                           
            board[b] = i+1;             /* put piece back               */

        }                               /* all moves done               */

        if(uu>=0) pos[uu] = pos[MEN-1];
        if(task+1&2)                    /* regular backward move gen.   */
        {   /* look if (other) piece to uncapture, if so, redo.         */
            while(--u && pos[u]!=pos[MEN-1]);
            if(!start && u<nblack) break;
            /* TB 'base' index, w piece on sq. 0 instead of 'in prison' */
            unind = tbind - (pos[MEN-1] + (pos[MEN-1]&7)>>1)*stride[u];
        } else u = 0;                   /* forward or in-check gen.     */
    }while(u);                          /* redo with next uncapture     */
    /* note that we never REdo for uncapturing a king (u= 0 or MEN-1)   */
}

void scan(int pass, int n, int stale)
/* scan through TB and board positions to perform task requested by     */
/* argument 'pass'. The board pos. is updated in synchronicty with the  */
/* TB index, so it can be immediately used for move generation. In order*/
/* not to waste too much time on this the fastest-scanning piece (the   */
/* black king) is put only on the board when we find a marked position  */
/* that we want to process, and otherwise only every 64 entries a board */
/* update is necessary.                                                 */
{
    int i, j, k, m, tbaddr, mask=0, ondiag, scnt, his[65], colsym;

    for(i=0;i<65;i++)his[i]=0;
    cnt = 0;
    for(i=0;i<129;i++) board[i] = 0;    /* clear board                  */
    for(i=1;i<=MEN;i++) pos[i] = -9;    /* set position to '-1'         */
    tbindex = 0;
    i = MEN-1;                          /* first board update all pieces*/

    do{
        /* differentially update board-position of 'higher order' pieces*/
        do{
            board[pos[i]] = 0;          /* erase piece                  */
            tbindex -= stride[i];       /* precompensation              */
            do{
                tbindex += stride[i];   /* skip invalid positions       */
                pos[i] = pos[i]+9&~8;   /* search: next square          */
                if(i>MEN-3) {
                if(i==MEN-1)
#ifdef DIAGSYM
                {   ondiag = pos[i]==0;         
                    if(pos[i]&4)        /* wK leaves symmetry sector    */
                    {   /* skip to diag of next rank                    */
                        pos[i] += 13+(pos[i]>>4); 
                        tbindex += (4+(pos[i]>>4))*stride[i];
                        ondiag = 1;
#else
                {   ondiag = 0;         
                    if(pos[i]&4)        /* wK leaves symmetry sector    */
                    {   /* skip to a-file                               */
                        pos[i] += 12; 
                        tbindex += 4*stride[i];
#endif
                        if(pos[i]&0x20) mask = 024*stride[i];
                    }
                } else if(ondiag && !(pos[i]&BOX+7))
                /* if wK on diag, next white piece must not be above it */
                {   tbindex += (pos[i]>>4)*stride[i];     /* skip       */
                    pos[i] += pos[i]>>4;     /* and start on diag.  */
                }
		}
            } while(board[pos[i]]       /* until empty square for it    */
                && pos[i]!=pos[MEN-1]); /* or captured (= @wK)          */
            /* this terminated as we run off-board, for BOX is empty    */
            board[pos[i]] = i+1;        /* put piece on found square    */
            board[pos[MEN-1]] = MEN;    /* but protect white king       */
            if(pos[i] == BOX)           /* if piece ran off board       */
            {
                board[BOX] = 0;         /* keep dummy square empty      */
                pos[i] = -9;            /* and prepare wrap around      */
                i++;                    /* but first move higher-order  */
            } else i--;                 /* if succesfully placed, finish*/
                                        /* placing lower-order pieces   */
        } while(i);                     /* until piece 1, ... n done    */
        if(tbindex>=2*SIZE) break;

        tbaddr = tbindex ^ mask;
        scnt = 0;
        if(pass>0)
        /* now pieces 1-n placed, try all positions for piece 0 (=bK)   */
        {
            do{                         /* for all positions of bK      */
                if(tb[tbaddr]>>1 == n)  /* look for properly marked pos */
                {
                    pos[0] = (tbaddr&63) + (tbaddr&0x38);
                    board[pos[0]] = 1;  /* put bK on board              */
                    switch(pass)
                    {
                        case 1:
                                /* generate parents (certain win.wtm)   */
                                /* and grandparents (potential win.btm) */
                                zgen(nblack,MEN,1,n+1,tbaddr^mask,mask,0);
                                break;
                        case 2:
                                /* screen potential win(n).btm          */
				if(nullMove && !stale && !(tb[tbaddr]&1)) { tb[tbaddr] = 0; break; } // if not win.wtm black would simply pass turn
                                zgen(0,nblack,3,n,tbaddr^mask,mask,0);
                                if(tb[tbaddr]>>1==n)
                                {   /* the position checked out, but if */
                                    /* n==1 it still might be stalemate */
                                    if(stale && !(tb[tbaddr]&1)) // not in check and no legal moves
                                    {   tb[tbaddr] = LABEL, vcnt--; }
                                    else cnt++;
                                    sample2 = tbaddr^mask;
                                }
                                break;
                    }
                    board[pos[0]] = 0;  /* and erase bK again           */
                    scnt++;
                }
            } while(++tbaddr & 63);
            his[scnt]++;
        } else if(pass==0)
            /* seed for black in-check positions has no bK at all   */
            zgen(nblack,MEN,0,n,tbindex,mask,0);
        else
        do{ if(pass==-1) tb[tbaddr]=0; else
            if(pass==-2)
            {   j = tbaddr ^ mask; k = 1; m = j>>6*MEN-6;
                if((m&7)!=m>>3 || (j>>6*MEN-12&7)!=(j>>6*MEN-9&7))
                    k = 2;
                if((j&63)==m || (j>>6&63)==m ||
                   (j>>6*MEN-12&63)==m || (j>>6*MEN-18&63)==m ||
                   (j&63)==(j>>6&63) || (j&63)==(j>>12&63) || (j&63)==(j>>6*MEN-12&63)
                  ) continue;   /* don't count pos. with capt. pieces   */
                colsym = 0;
                pos[0] = (tbaddr&070) + (tbaddr&077);
                if(cs1>=0)
                {   i = pos[cs1]^pos[cs1]>>4^pos[cs2]^pos[cs2]>>4;
                    if(i&1)
                    {   colsym = 150;   /* bisshops on different color  */
                        if(cs3>=0)
                        {    if(inhom && (pos[cs1]^pos[cs1]>>4^pos[cs3]^pos[cs3]>>4)&1)
                                colsym = 300;
                        }
                    } else              /* bisshops on same color       */
                    if(cs3>=0)
                    {   if((pos[cs1]^pos[cs1]>>4^pos[cs3]^pos[cs3]>>4)&1)
                        colsym = 450;   /* third different from pair    */
                    }
                }
                histo[(tb[tbaddr]>>1)+colsym] += k;
                if(tb[tbaddr]&1)
                {   histo[128+colsym] += k;
/*                    histo[colsym+classify(tbindex)] += k;*/
                }
            } else
            if(pass=-3) /* collapse DTM converted to simple 'won' mark  */
            {    i = tb[tbaddr];
                 if(i&~1)
                 {   if(i>>1 < n) tb[tbaddr] = i&1|2; else
                     if(i>>1 == n) tb[tbaddr] = i&1|2*stale;
                 }
            }
        } while(++tbaddr&63);
        tbindex += 64;

        i = 1;                          /* next board update for piece 1*/
    } while(tbaddr<SIZE);
    /*for(i=0;i<65;i++)if(his[i])printf("  %2d. %6d\n",i,his[i]);*/
}

void SetDTx(int index, int dtx)
/* convert the index to an address (or two), and set the element there to the given dtx */
{
	int i, j;

	if(index & 040<<6*MEN-6) index ^= RANKS&~(-1<<6*MEN);
	if(index &  04<<6*MEN-6) index ^= FILES&~(-1<<6*MEN);
	i = (index>>6*MEN-6&7) - (index>>6*MEN-3);
	j = (index>>6*MEN-12&7) - (index>>6*MEN-9&7);
#ifndef DIAGSYM
	i = 1; // suppress diagonal mirroring
#endif
	if(i > 0 || i == 0 && j >= 0) {
	    int nwind = index;
	    if(nwind & 020<<6*MEN-6) nwind ^= 024<<6*MEN-6;
	    tb[nwind] = tb[nwind] & 1 | dtx << 1;
	    cnt++;
	}
	if(i < 0 || i == 0 && j <= 0) { // for i=j=0 map into both sectors
	    index = index << 3 & RANKS | index >> 3 & FILES;
	    if(index & 020<<6*MEN-6) index ^= 024<<6*MEN-6;
	    tb[index] = tb[index] & 1 | dtx << 1;
	    cnt++;
	}
}

void seed(char *fname)
/* predefine the DTx for positions specified in the seeds file */
{
    char buf[80], c; int n;
    FILE *f;

    if(!fname) return;
    if(!(f = fopen(fname, "r"))) return;
    while(fscanf(f, "%s%d", buf, &n) == 2) {
	int index=0, i, j, wildcard= -1;
	for(i=0; i<MEN; i++) {
	    if(sscanf(buf+2*i, "%c%d", &c, &j) != 2) {
		if(buf[2*i] == '*') wildcard = 6*(MEN-1-i), c = 'a', j = 1;
		else break;
	    }
	    index = index*64 + (c-'a')*8 + j - 1;
	}
	if(wildcard < 0)
	   SetDTx(index, n);
	else for(i=0; i<64; i++) {
	   j = index + (i<<wildcard);
	   if(wildcard != 0  && i == (j & 63)) continue;
	   if(wildcard != 6  && i == (j>>6 & 63)) continue;
	   if(wildcard != 12 && i == (j>>12 & 63)) continue;
	   if(wildcard != 18 && i == (j>>18 & 63) && MEN > 3) continue;
	   if(wildcard != 24 && i == (j>>24 & 63) && MEN > 4) continue;
	   SetDTx(j, n);
	}
    }
    fclose(f);
}

void build()
/* build the TB by working backwards from king-captured positions.      */
/* Indicate only positions that are won for white. Each position has a  */
/* 1-byte entry, that has the '1' bit set if it is won with white to    */
/* move (win.wtm), and for which the other bits hold the DTM if it is   */
/* won with black to move (win(n).btm). A zero in this field means draw,*/
/* lost or undecided (the latter during building). DTM=1 means checkmate*/
{
        int n, i, ocnt, pcnt, dcnt, startCnt=0;
        clock_t t,t1=0,t2=0;

        mine = nblack+1;
        t = clock(); ocnt = lcnt; pcnt = mcnt; dcnt = ccnt;
        scan(0,1,0);                    /* label potential check-mates  */
        for(i=0;i<600;i+=150) histo[i+128] = 0;
        scan(-2,1,0);                   /* count check positions        */
        for(i=0;i<600;i+=150) histo[i+149] = histo[i+128];
        t1 += clock()-t; 
        scan(2,1,staleMate);                    /* screen checkmates            */
        if(dtc) scan(2,CONV,staleMate);         /* screen checkmates w. all pcs.*/
        t2 += clock()-t-t1;
	if(!dtc) seed(fname);
        n = 1;
        do{
	    if(xboard)
		printf("1 0 0 0 %3d.\t%10d\t%15d\t%6.2f\n", n-1-startCnt, cnt, mcnt-pcnt, (clock()-t)*TICK), fflush(stdout);
	    else
        	printf("%3d. %9d %9d %9d %9d %7.3f %7.3f %7.3f\n",
                        n-1,cnt,lcnt-ocnt,mcnt-pcnt,ccnt-dcnt,(clock()-t)*TICK,t1*TICK,t2*TICK);
            ocnt = lcnt; pcnt = mcnt; dcnt = ccnt;
            scan(1,n,0);                /* label potential win(n).btm   */
            t1 = clock()-t-t2;
            n++; 
            scan(2,n,0);                /* and screen them              */
            t2 = clock()-t-t1;
            if(dtc && cnt==0 && n<CONV)
            {   printf("%scomplete:\n", xh); fflush(stdout);
                scan(-3,CONV,10);
                n=CONV=startCnt=10; scan(2,n,0);
		seed(fname);
            }
            if(n==125) { char c, d;
		printf("%sMax DTC reached. Continue and overwrite (y/n)?\n", xboard ? "askuser Q " : ""); fflush(stdout);
		if(xboard) { while(getchar() != 'Q'); getchar(); }
		if(scanf("%c%*c", &c) > 0 && c == 'n') break;
		scan(-3,125,10); n=10;
	    }
        } while(cnt || dtc && n<=CONV);        /* until none is good           */
	if(xboard) {
		printf("1 0 0 0 %3d.\t%10d\t%15d\t%6.2f\n", n-1-CONV, cnt, mcnt-pcnt, (clock()-t)*TICK), fflush(stdout);
	} else {
            printf("%3d. %9d %9d %9d %9d %7.3f %7.3f %7.3f\n",
                        n-1,cnt,lcnt-ocnt,mcnt-pcnt,ccnt-dcnt,(clock()-t)*TICK,t1*TICK,t2*TICK);
	}
        printf("%s%9d marked win.wtm\n%s%9d labeled potential win(n).btm\n%s%9d verified win(n).btm\n%s%9d checked\n",
                xh, mcnt, xh, lcnt, xh, vcnt, xh, ccnt), fflush(stdout);
}


void play()
{
        int i, j, k, curpos, playpos=sample1, mask, col, force=0, nr=0;
	int history[500];

        mine = 0;
        while(1)
        {
		if(xboard) {
		    char line[80], command[80];
		    int c, i, rf, rt; char ff, ft;
		    i = 0; while((c = getchar()) != EOF) { line[i++] = c; if(c == '\n') break; } line[i] = 0;
		    sscanf(line,"%s", command);
		    if(sscanf(command, "%c%d%c%d", &ff, &rf, &ft, &rt) == 4 && ff >= 'a') { // move
			int from = 8*(rf-1) + ff-'a', to = 8*(rt-1) + ft - 'a';
		        for(i=0; i<MEN; i++) if(from == (playpos >> 6*i & 63)) break;
			if(i != MEN) { int v, x;
			    for(c=first[piece[i]]; v=boardvec[c]; c++) {
				x=from+(from&070);
				do x += v; while(x != to+(to&070) && mode[c]&4 && !(x&0x88)); // scan ray
				if(x == to+(to&070)) break;
			    }
			    if(x != to+(to&070)) i = MEN;
			}
			if(i == MEN) { printf("Illegal move\n"), fflush(stdout); continue; }
			history[++nr] = curpos = playpos = playpos & ~(077<<6*i) | to<<6*i;
		    } else if(!strcmp(command, "edit")) { col = nr = 0; force = 1;
		    } else if(!strcmp(command, "quit")) { exit(0);
		    } else if(!strcmp(command, "c")) { col = !col;
		    } else if(!strcmp(command, "#")) { playpos = 0;
		    } else if(sscanf(command, "%c%c%d", &ff, &ft, &rt) == 3 && ft >= 'a') {
			for(i=col?0:nblack; i<(col?nblack:MEN); i++) if((playpos & 077<<6*i) == 0 && cc[piece[i]] == ff) break;
			if(i == (col?nblack:MEN)) printf("telluser bad piece\n"), fflush(stdout); else {
			    history[0] = playpos |= 8*(rt-1) + ft - 'a' << 6*i;
			}
		    } else if(!strcmp(command, "go")) { force = 0;
		    } else if(!strcmp(command, "new")) { force = 1;
		    } else if(!strcmp(command, ".")) { ;
		    } else if(!strcmp(command, "remove")) { playpos = history[nr -= 2]; continue;
		    } else if(!strcmp(command, "")) {
		    }
                    else if(sscanf(command, "%o", &curpos) == 1) playpos = curpos;
		    else continue;
		} else {
                    printf("octal position number: "), scanf("%o", &j);
                    if(j&~077) playpos = j; else playpos = playpos&~077|j;
		}
		if(force) continue;
		curpos = playpos;
		// map into symmetry sector
		transform = 0;
		if(curpos & 040<<6*MEN-6) transform ^= RANKS&~(-1<<6*MEN);
		if(curpos &  04<<6*MEN-6) transform ^= FILES&~(-1<<6*MEN);
		curpos ^= transform;
		i = (curpos>>6*MEN-6&7) - (curpos>>6*MEN-3);
                if(i < 0 || i == 0 && (curpos>>6*MEN-12&7) < (curpos>>6*MEN-9&7))
		    transform |= 1<<31, curpos = curpos << 3 & RANKS | curpos >> 3 & FILES;
                j = curpos;
                for(i=0;i<MEN;i++) {pos[i]=j&63; pos[i]+=pos[i]&070; j>>=6; }
                for(i=0;i<128;i=i+9&~8) board[i] = 0;
                for(i=0;i<MEN;i++) {board[pos[i]]=i+1; }
                if(!xboard,1) for(i=0;i<121;i++)
                    printf(" %c",i&8&&(i+=7)?10:
                    board[i]?cc[piece[board[i]-1]]:'.');

                mask = curpos&020<<6*MEN-6 ? 024<<6*MEN-6 : 0;
                nbest = 1000;
                zgen(nblack,MEN,4,1,curpos,mask,0);
                curpos = best;
                mask = curpos&04<<6*MEN-6 ? 024<<6*MEN-6 : 0;
                curpos ^= mask;

                printf("%s%10o mate in %d\n", xh, curpos, nbest-CONV);
                j = curpos;
                for(i=0;i<MEN;i++) {pos[i]=j&63; pos[i]+=pos[i]&070; j>>=6; }
                for(i=0;i<128;i=i+9&~8) board[i] = 0;
                for(i=0;i<MEN;i++) {board[pos[i]]=i+1; }
                for(i=0;i<121;i++)
                    printf(" %c",i&8&&(i+=7)?10:
                    board[i]?cc[piece[board[i]-1]]:'.');
		if(xboard) printf("1 %d 0 1 tablebase\n", 100000+nbest-CONV);
		printf("move %c%d%c%d\n", (move>>8&7)+'a', (move>>12)+1, (move&7)+'a', (move>>4&7)+1), fflush(stdout);
		{	int from = move>>8 , to = move&255;
			from = from + (from&7) >> 1; to = to + (to&7) >> 1;
		        for(i=0; i<MEN; i++) if(from == (playpos >> 6*i & 63)) break;
			history[++nr] = playpos = playpos & ~(077<<6*i) | to<<6*i;
		}
        }
}

int main(int argc, char **argv)
{
        int i, j, k, l, m;
        char *p, name[80]; FILE *f;

        p = (char *) malloc(SIZE+4096);
        if(p == NULL) exit(1);

        for(tb=p;4095&(long)tb;) ++tb;    /* align with cash line         */
        printf("%x %x\n",p, tb);

	if(argc > 1 && !strcmp(argv[1], "-xboard")) xboard++, argc--, argv++, strcpy(xh, "1 0 0 0 "),
						    printf("feature myname=\"EGTgen %s.%d\" done=1\n", VERSION, MEN), fflush(stdout);

	if(argc > 1 && !strcmp(argv[1], "-m")) dtc = 0, argc--, argv++; // generate DTM

	if(argc > 1 && !strcmp(argv[1], "-0")) nullMove++, argc--, argv++; // turn passing allowed

	if(argc > 1 && !strcmp(argv[1], "-s")) staleMate =/*2*nullMove,*/ nullMove = 0, argc--, argv++; // 0: stalemate = win, 1: stalemate = pass turn

	if(argc > 2 && !strcmp(argv[1], "-f")) fname = argv[2], argc--, argv += 2; // turn passing allowed

	if(f = fopen("piecedef.ini", "r")) { // an ini file exists; use it to read piece definitions
	    char p, c,rights[80];
	    i = j = 0;
	    while(fscanf(f, "%c:%c", &p, &c) == 2 && c == ' ') { // format "R: 1,0,s*" = omni-directional (1,0) slider, "A: 1,2,* 1,1,s*" = Archbishop
		cc[i] = p;
		first[i] = j;
		m = 1; // start color
		while(fscanf(f, "%d,%d", &k, &l) == 2) {
		    mode[j] = 3; // assume leaper
		    if(fscanf(f, ",%s", rights) == 1) {
			if(strchr(rights, 'n')) mode[j] = 2;
			if(strchr(rights, 'c')) mode[j] = 1;
			mode[j] |= 4*!!strchr(rights, 's');
		    }
		    if(mode[j] & 2) m |= 1 + ((k ^ l) & 1); // color of non-capt target square
		    boardvec[j++] = 16*k + l;
		    if(strchr(rights, '*') || strchr(rights, '+')) {
			if(l) mode[j] = mode[j-1], boardvec[j++] =  16*k - l;
			if(k) mode[j] = mode[j-1], boardvec[j++] = -16*k + l;
			if(k && l) mode[j] = mode[j-1], boardvec[j++] = -16*k - l;
		        if(k != l
#ifndef DIAGSYM
				  && strchr(rights, '*') // only without diagonal symmetry * and + are different.
#endif
							) {
			    mode[j] = mode[j-1], boardvec[j++] =  16*l + k;
			    if(k) mode[j] = mode[j-1], boardvec[j++] =  16*l - k;
			    if(l) mode[j] = mode[j-1], boardvec[j++] = -16*l + k;
			    if(k && l) mode[j] = mode[j-1], boardvec[j++] = -16*l - k;
			}
		    }
		}
		boardvec[j] = mode[j] = 0; j++;
		bis[i++] = (m != 3); // color-bound
	    }
	    cc[i] = 0;
	    fclose(f);
for(i=0;cc[i];i++){printf("# %c: ",cc[i]);j=first[i];while(boardvec[j])printf(" %3d,%d",boardvec[j],mode[j]),j++;printf("\n");}
	}

	j = MEN-1; k = -1;
	if(argc > 1) for(i=0; argv[1][i]; i++) {
	    if(argv[1][i] == '.') { k = 1; j = 0; continue; }
	    piece[j] = strchr(cc, argv[1][i]) - cc;
	    nblack = j += k;
	}

        f = fopen("rep2.txt","w");
/*for(piece[1]=1;piece[1]<=H;piece[1]++)/*if(piece[1]!=4)*/
/*for(piece[2]=F;piece[2]<=C;piece[2]++)/*if(piece[2]!=4)*/
/*for(piece[3]=F;piece[3]<=C;piece[3]++)/*if(piece[3]!=4)*/
{
        j = mcnt = ccnt = vcnt = lcnt = 0; conv = 125; cs1 = cs2 = cs3 = -1;
        for(i=MEN-1;i>=nblack;i--) name[j++]=cc[piece[i]]; name[j++]='_';
        for(i=0;i<nblack;i++) name[j++] = cc[piece[i]]; name[j] = 0;
	if(nullMove) strcat(name, " nullmove");
	if(staleMate != 1) strcat(name, staleMate ? " stalepass" : " stalewin");
	if(fname) strcat(name, " seeded");
        printf("%s\n", name); fprintf(f, "\n%s\n", name);

        exch = MEN-nblack>2 && piece[MEN-2]==piece[MEN-3];

	for(i=0; i<sizeof(boardvec)/sizeof(int); i++) {
	    basevec[i] = boardvec[i] - ((boardvec[i]+8 & ~15)>>1); // convert 16x8 steps to 8x8 steps
	}

        stride[0] = 1;                  /* define TB mapping            */
        for(i=1;i<MEN;i++) stride[i] = 64*stride[i-1];
        /*(basically all 6-bit square numbers packed next to each other)*/

        scan(-1,0,0);                   /* clear TB                     */

        /* initialize stepvectors in TB for each piece, by pre-scaling  */
        for(i=0; i<MEN; i++)
        {   j = first[piece[i]]-1; m=MAXDIR*i;
            while(k = basevec[++j])
                codevec[m++] = stride[i]*k;       /* scaled 077 vectors */
        }

        for(i=0; i<MEN; i++) if(bis[piece[i]]) { cs3=cs2; cs2=cs1; cs1=i; }
        if(cs2<0) cs1 = -1; else                /* at most one 'bishop' */
        {   if(cs3>=0)
            {   if(cs2 < nblack)                /* two black bishops    */
                {    i=cs1; cs1=cs3; cs3=i;     /* cs1&cs2 same side    */
                }
                inhom = piece[cs1]!=piece[cs2];
            }
        }

        printf("bishops: %d %d %d (%d)\n", cs1, cs2, cs3, inhom);

        build();
        printf("%s%o %o\n", xh, sample1, sample2);

        for(i=0;i<149;i++) histo[i]=histo[150+i]=histo[300+i]=histo[450+i]=0;
        scan(-2,1,0);
        fprintf(f, "\nWON.wtm %10d", histo[128]);
        if(cs1>=0) fprintf(f, " %10d", histo[278]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[428], histo[578]);
        fprintf(f, "\nK capture %8d", histo[149]);
        if(cs1>=0) fprintf(f, " %10d", histo[299]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[449], histo[599]);
        fprintf(f, "\nother   %10d", histo[128]-histo[149]);
        if(cs1>=0) fprintf(f, " %10d", histo[278]-histo[299]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[428]-histo[449], histo[578]-histo[599]);
        for(i=0;i<126;i++) if(histo[i]+histo[150+i]+histo[300+i]+histo[450+i])
        {    fprintf(f, "\n%3d.    %10d", i, histo[i]);
             if(cs1>=0) fprintf(f, " %10d", histo[150+i]);
             if(cs3>=0) fprintf(f, " %10d %10d", histo[300+i], histo[450+i]);
             for(j=0;j<600;j+=150) histo[j+148] += histo[j+i];
        }
        fprintf(f, "\nWON.btm %10d", histo[148]-histo[0]);
        if(cs1>=0) fprintf(f, " %10d", histo[298]-histo[150]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[448]-histo[300], histo[598]-histo[450]);
        fprintf(f, "\nstalemate %8d", histo[127]);
        if(cs1>=0) fprintf(f, " %10d", histo[277]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[427], histo[577]);
        fprintf(f, "\nW check %10d", histo[126]);
        if(cs1>=0) fprintf(f, " %10d", histo[276]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[426], histo[576]);
        fprintf(f, "\nLEGAL   %10d", histo[148]+histo[127]);
        if(cs1>=0) fprintf(f, " %10d", histo[298]+histo[277]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[448]+histo[427], histo[598]+histo[577]);
        fprintf(f, "\nTOTAL   %10d", histo[148]+histo[126]+histo[127]);
        if(cs1>=0) fprintf(f, " %10d", histo[298]+histo[276]+histo[277]);
        if(cs3>=0) fprintf(f, " %10d %10d", histo[448]+histo[426]+histo[427],
                                  histo[598]+histo[576]+histo[577]);
        fprintf(f, "\n");
        fflush(f);
}
        fclose(f);
        play();
	return 0;
}
