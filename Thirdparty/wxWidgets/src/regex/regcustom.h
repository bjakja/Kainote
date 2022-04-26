/*
 * Copyright (c) 1998, 1999 Henry Spencer.  All rights reserved.
 * 
 * Development of this software was funded, in part, by Cray Research Inc.,
 * UUNET Communications Services Inc., Sun Microsystems Inc., and Scriptics
 * Corporation, none of whom are responsible for the results.  The author
 * thanks all of them. 
 * 
 * Redistribution and use in source and binary forms -- with or without
 * modification -- are permitted for any purpose, provided that
 * redistributions in source form retain this entire copyright notice and
 * indicate the origin and nature of any modifications.
 * 
 * I'd appreciate being given credit for this package in the documentation
 * of software which uses it, but that is not a requirement.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * HENRY SPENCER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* overrides for regguts.h definitions, if any */
/* regguts only includes standard headers if NULL is not defined, so do it
 * ourselves here */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

/* must include this after ctype.h inclusion for CodeWarrior/Mac */
#include "wx/defs.h"
#include "wx/chartype.h"
#include "wx/wxcrtbase.h"

/*
 * Do not insert extras between the "begin" and "end" lines -- this
 * chunk is automatically extracted to be fitted into regex.h.
 */
/* --- begin --- */
/* ensure certain things don't sneak in from system headers */
#ifdef __REG_WIDE_T
#undef __REG_WIDE_T
#endif
#ifdef __REG_WIDE_COMPILE
#undef __REG_WIDE_COMPILE
#endif
#ifdef __REG_WIDE_EXEC
#undef __REG_WIDE_EXEC
#endif
#ifdef __REG_REGOFF_T
#undef __REG_REGOFF_T
#endif
#ifdef __REG_VOID_T
#undef __REG_VOID_T
#endif
#ifdef __REG_CONST
#undef __REG_CONST
#endif
#ifdef __REG_NOFRONT
#undef __REG_NOFRONT
#endif
#ifdef __REG_NOCHAR
#undef __REG_NOCHAR
#endif
#if wxUSE_UNICODE
#   define  __REG_WIDE_T        wxChar
#   define  __REG_WIDE_COMPILE  wx_re_comp
#   define  __REG_WIDE_EXEC     wx_re_exec
#   define  __REG_NOCHAR        /* don't want the char versions */
#endif
#define __REG_NOFRONT           /* don't want regcomp() and regexec() */
#define _ANSI_ARGS_(x)          x
/* --- end --- */

/* internal character type and related */
typedef wxChar chr;             /* the type itself */
typedef int pchr;               /* what it promotes to */
typedef unsigned uchr;          /* unsigned type that will hold a chr */
typedef int celt;               /* type to hold chr, MCCE number, or NOCELT */
#define NOCELT  (-1)            /* celt value which is not valid chr or MCCE */
#define UCHAR(c) ((unsigned char) (c))
#define CHR(c)  (UCHAR(c))      /* turn char literal into chr literal */
#define DIGITVAL(c) ((c)-'0')   /* turn chr digit into its value */
#if !wxUSE_UNICODE
#   define CHRBITS 8            /* bits in a chr; must not use sizeof */
#   define CHR_MIN 0x00         /* smallest and largest chr; the value */
#   define CHR_MAX 0xff         /*  CHR_MAX-CHR_MIN+1 should fit in uchr */
#elif SIZEOF_WCHAR_T == 4
#   define CHRBITS 32           /* bits in a chr; must not use sizeof */
#   define CHR_MIN 0x00000000   /* smallest and largest chr; the value */
#   define CHR_MAX 0xffffffff   /*  CHR_MAX-CHR_MIN+1 should fit in uchr */
#else
#   define CHRBITS 16           /* bits in a chr; must not use sizeof */
#   define CHR_MIN 0x0000       /* smallest and largest chr; the value */
#   define CHR_MAX 0xffff       /*  CHR_MAX-CHR_MIN+1 should fit in uchr */
#endif

/*
 * I'm using isalpha et al. instead of wxIsalpha since BCC 5.5's iswalpha
 * seems not to work on Windows 9x? Note that these are only used by the
 * lexer, and although they must work for wxChars, they need only return
 * true for characters within the ascii range.
 */
#define iscalnum(x)     ((wxUChar)(x) < 128 && isalnum(x))
#define iscalpha(x)     ((wxUChar)(x) < 128 && isalpha(x))
#define iscdigit(x)     ((wxUChar)(x) < 128 && isdigit(x))
#define iscspace(x)     ((wxUChar)(x) < 128 && isspace(x))

/* name the external functions */
#define compile         wx_re_comp
#define exec            wx_re_exec

/* enable/disable debugging code (by whether REG_DEBUG is defined or not) */
#if 0           /* no debug unless requested by makefile */
#define REG_DEBUG       /* */
#endif

/* and pick up the standard header */
#include "regex.h"

/* token type codes, some also used as NFA arc types */
#define	EMPTY	'n'		/* no token present */
#define	EOS	'e'		/* end of string */
#define	PLAIN	'p'		/* ordinary character */
#define	DIGIT	'd'		/* digit (in bound) */
#define	BACKREF	'b'		/* back reference */
#define	COLLEL	'I'		/* start of [. */
#define	ECLASS	'E'		/* start of [= */
#define	CCLASS	'C'		/* start of [: */
#define	END	'X'		/* end of [. [= [: */
#define	RANGE	'R'		/* - within [] which might be range delim. */
#define	LACON	'L'		/* lookahead constraint subRE */
#define	AHEAD	'a'		/* color-lookahead arc */
#define	BEHIND	'r'		/* color-lookbehind arc */
#define	WBDRY	'w'		/* word boundary constraint */
#define	NWBDRY	'W'		/* non-word-boundary constraint */
#define	SBEGIN	'A'		/* beginning of string (even if not BOL) */
#define	SEND	'Z'		/* end of string (even if not EOL) */
#define	PREFER	'P'		/* length preference */

#define	INCOMPATIBLE	1	/* destroys arc */
#define	SATISFIED	2	/* constraint satisfied */
#define	COMPATIBLE	3	/* compatible but not satisfied yet */
struct vars {
	regex_t* re;
	chr* now;		/* scan pointer into string */
	chr* stop;		/* end of string */
	chr* savenow;		/* saved now and stop for "subroutine call" */
	chr* savestop;
	int err;		/* error code (0 if none) */
	int cflags;		/* copy of compile flags */
	int lasttype;		/* type of previous token */
	int nexttype;		/* type of next token */
	chr nextvalue;		/* value (if any) of next token */
	int lexcon;		/* lexical context type (see lex.c) */
	int nsubexp;		/* subexpression count */
	struct subre** subs;	/* subRE pointer vector */
	size_t nsubs;		/* length of vector */
	struct subre* sub10[10];	/* initial vector, enough for most */
	struct nfa* nfa;	/* the NFA */
	struct colormap* cm;	/* character color map */
	color nlcolor;		/* color of newline */
	struct state* wordchrs;	/* state in nfa holding word-char outarcs */
	struct subre* tree;	/* subexpression tree */
	struct subre* treechain;	/* all tree nodes allocated */
	struct subre* treefree;		/* any free tree nodes */
	int ntree;		/* number of tree nodes */
	struct cvec* cv;	/* interface cvec */
	struct cvec* cv2;	/* utility cvec */
	struct cvec* mcces;	/* collating-element information */
#		define	ISCELEADER(v,c)	(v->mcces != NULL && haschr(v->mcces, (c)))
	struct state* mccepbegin;	/* in nfa, start of MCCE prototypes */
	struct state* mccepend;	/* in nfa, end of MCCE prototypes */
	struct subre* lacons;	/* lookahead-constraint vector */
	int nlacons;		/* size of lacons */
};
#define	FEWSTATES	20	/* must be less than UBITS */
#define	FEWCOLORS	15

#define	WORK	1		/* number of work bitvectors needed */
struct smalldfa {
	struct dfa dfa;
	struct sset ssets[FEWSTATES * 2];
	unsigned statesarea[FEWSTATES * 2 + WORK];
	struct sset* outsarea[FEWSTATES * 2 * FEWCOLORS];
	struct arcp incarea[FEWSTATES * 2 * FEWCOLORS];
};

#define	DOMALLOC	((struct smalldfa *)NULL)	/* force malloc */

struct vars1 {
	regex_t* re;
	struct guts* g;
	int eflags;		/* copies of arguments */
	size_t nmatch;
	regmatch_t* pmatch;
	rm_detail_t* details;
	chr* start;		/* start of string */
	chr* stop;		/* just past end of string */
	int err;		/* error code if any (0 none) */
	regoff_t* mem;		/* memory vector for backtracking */
	struct smalldfa dfa1;
	struct smalldfa dfa2;
};

/* lazy-DFA representation */
struct arcp {			/* "pointer" to an outarc */
	struct sset* ss;
	color co;
};

struct sset {			/* state set */
	unsigned* states;	/* pointer to bitvector */
	unsigned hash;		/* hash of bitvector */
#		define	HASH(bv, nw)	(((nw) == 1) ? *(bv) : hash(bv, nw))
#	define	HIT(h,bv,ss,nw)	((ss)->hash == (h) && ((nw) == 1 || \
		memcmp(VS(bv), VS((ss)->states), (nw)*sizeof(unsigned)) == 0))
	int flags;

	struct arcp ins;	/* chain of inarcs pointing here */
	chr* lastseen;		/* last entered on arrival here */
	struct sset** outs;	/* outarc vector indexed by color */
	struct arcp* inchain;	/* chain-pointer vector for outarcs */
};

struct dfa {
	int nssets;		/* size of cache */
	int nssused;		/* how many entries occupied yet */
	int nstates;		/* number of states */
	int ncolors;		/* length of outarc and inchain vectors */
	int wordsper;		/* length of state-set bitvectors */
	struct sset* ssets;	/* state-set cache */
	unsigned* statesarea;	/* bitvector storage */
	unsigned* work;		/* pointer to work area within statesarea */
	struct sset** outsarea;	/* outarc-vector storage */
	struct arcp* incarea;	/* inchain storage */
	struct cnfa* cnfa;
	struct colormap* cm;
	chr* lastpost;		/* location of last cache-flushed success */
	chr* lastnopr;		/* location of last cache-flushed NOPROGRESS */
	struct sset* search;	/* replacement-search-pointer memory */
	int cptsmalloced;	/* were the areas individually malloced? */
	char* mallocarea;	/* self, or master malloced area, or NULL */
};

#define	VISERR(vv)	((vv)->err != 0)	/* have we seen an error yet? */
#define	ISERR()	VISERR(v)
#define	VERR(vv,e)	(((vv)->err) ? (vv)->err : ((vv)->err = (e)))
#define	ERR(e)	(void)VERR(v, e)		/* record an error */
#define	NOERR()	{if (ISERR()) return v->err;}	/* if error seen, return it */
#define	OFF(p)	((p) - v->start)
#define	LOFF(p)	((long)OFF(p))

#		define	STARTER		01	/* the initial state set */
#		define	POSTSTATE	02	/* includes the goal state */
#		define	LOCKED		04	/* locked in cache */
#		define	NOPROGRESS	010	/* zero-progress state set */