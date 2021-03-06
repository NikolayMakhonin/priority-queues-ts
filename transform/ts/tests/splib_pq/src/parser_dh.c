/********************************************************************/
/*                                                                  */
/*  parse (...) :                                                   */
/*       1. Reads shortest path problem in extended DIMACS format.  */
/*       2. Prepares internal data representation #1.               */
/*                                                                  */
/********************************************************************/
       
/* files to be included: */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
import {} from 'types_dh.h'


/* ----------------------------------------------------------------- */

int16 parse( n_ad, m_ad, nodes_ad, arcs_ad,
           source_ad, node_min_ad, problem_name )

/* all parameters are output */
let n_ad: [];                 /* address of the number of nodes */
let m_ad: [];                 /* address of the number of arcs */
let nodes_ad: node*[];            /* address of the array of nodes */
let arcs_ad: arc*[];             /* address of the array of arcs */
let source_ad: node*[];           /* address of the pointer to the source */
let node_min_ad: [];          /* address of the minimal node */
let problem_name: string;         /* pointer to the string with problem name */

{

export const MAXLINE = 100;	/* max line length in the input file */
export const ARC_FIELDS = 3;	/* no of fields in arc input  */
export const P_FIELDS = 3;       /* no of fields in problem line */
#define PROBLEM_TYPE "sp"       /* name of problem type*/
#define DEFAULT_NAME "unknown"  /* default name of the problem */


n: int32,                      /* internal number of nodes */
        node_min,               /* minimal no of node  */
        node_max,               /* maximal no of nodes */
       *arc_first,              /* internal array for holding
                                     - node degree
                                     - position of the first outgoing arc */
       *arc_tail,               /* internal array: tails of the arcs */
        source,                 /* no of source */
        /* temporary variables carrying no of nodes */
        head, tail, i;

let m: int32,                      /* internal number of arcs */
        /* temporary variable carrying no of arcs */
        last, arc_num, arc_new_num;

let nodes: node*,                 /* pointer to the node structure */
        *head_p;

let arcs: arc*,                  /* pointer to the arc structure */
        *arc_current,
        arc_new;

let length: int32;                 /* length of the current arc */

int32    no_lines=0,             /* no of current input line */
        no_plines=0,            /* no of problem-lines */
        no_tlines=0,            /* no of title(problem name)-lines */
        no_nlines=0,            /* no of node(source)-lines */
        no_alines=0;            /* no of arc-lines */

let in_line: char[MAXLINE],       /* for reading input line */
        pr_type[3];             /* for reading type of the problem */

let k: int16,                      /* temporary */
        err_no;                 /* no of detected error */

/* -------------- error numbers & error messages ---------------- */
export const EN1 = 0;
export const EN2 = 1;
export const EN3 = 2;
export const EN4 = 3;
export const EN6 = 4;
export const EN10 = 5;
export const EN7 = 6;
export const EN8 = 7;
export const EN9 = 8;
export const EN11 = 9;
export const EN12 = 10;
export const EN13 = 11;
export const EN14 = 12;
export const EN16 = 13;
export const EN15 = 14;
export const EN17 = 15;
export const EN18 = 16;
export const EN21 = 17;
export const EN19 = 18;
export const EN20 = 19;
export const EN22 = 20;

static err_message: string[] =
  { 
/* 0*/    "more than one problem line.",
/* 1*/    "wrong number of parameters in the problem line.",
/* 2*/    "it is not a Shortest Path problem line.",
/* 3*/    "bad value of a parameter in the problem line.",
/* 4*/    "can't obtain enough memory to solve this problem.",
/* 5*/    "more than one line with the problem name.",
/* 6*/    "can't read problem name.",
/* 7*/    "problem description must be before source description.",
/* 8*/    "this parser doesn't support multiply sources.",
/* 9*/    "wrong number of parameters in the source line.",
/*10*/    "wrong value of parameters in the source line.",
/*11*/    "this parser doesn't support destination description.",
/*12*/    "source description must be before arc descriptions.",
/*13*/    "too many arcs in the input.",
/*14*/    "wrong number of parameters in the arc line.",
/*15*/    "wrong value of parameters in the arc line.",
/*16*/    "unknown line type in the input.",
/*17*/    "reading error.",
/*18*/    "not enough arcs in the input.",
/*19*/    "source doesn't have output arcs.",
/*20*/    "can't read anything from the input file."
  };
/* --------------------------------------------------------------- */

/* The main loop:
        -  reads the line of the input,
        -  analises its type,
        -  checks correctness of parameters,
        -  puts data to the arrays,
        -  does service functions
*/

while ( gets ( in_line ) != null )
  {
  no_lines ++;


  switch (in_line[0])
    {
      case 'c':                  /* skip lines with comments */
      case '\n':                 /* skip empty lines   */
      case '\0':                 /* skip empty lines at the end of file */
                break;

      case 'p':                  /* problem description      */
                if ( no_plines > 0 )
                   /* more than one problem line */
                   { err_no = EN1 ; goto error; }

                no_plines = 1;
   
                if (
        /* reading problem line: type of problem, no of nodes, no of arcs */
                    sscanf ( in_line, "%*c %2s %ld %ld", pr_type, &n, &m )
                !== P_FIELDS
                   )
		    /*wrong number of parameters in the problem line*/
		    { err_no = EN2; goto error; }

                if ( strcmp ( pr_type, PROBLEM_TYPE ) )
		    /*wrong problem type*/
		    { err_no = EN3; goto error; }

                if ( n <= 0  || m <= 0 )
		    /*wrong value of no of arcs or nodes*/
		    { err_no = EN4; goto error; }

        /* allocating memory for  'nodes', 'arcs'  and internal arrays */
                nodes    = new Array<node>(n+2);
		arcs     = new Array<arc>(m+1);
	        arc_tail = new Array<int32>(m);
		arc_first= new Array<int32>(n+2);
                /* arc_first [ 0 .. n+1 ] = 0 - initialized by calloc */

                if ( nodes == null || arcs == null ||
                     arc_first == null || arc_tail == null )
                    /* memory is not allocated */
		    { err_no = EN6; goto error; }
		     
		/* setting pointer to the current arc */
		arc_current = arcs;
                break;

      case 't':                  /* name of the problem  */
                if ( no_tlines )
                   /* more than one name of the problem */
                   { err_no = EN10 ; goto error; }

                no_tlines = 1;
   
                if (
                /* reading problem name */
	        sscanf ( in_line, "%*c %30s", problem_name )
                !== 1
                   )
                /* t-line doesn't contain problem name */
		    { err_no = EN7; goto error; }

                break;

      case 'n':		         /* source(s) description */
		if ( no_plines === 0 )
                  /* there was not problem line above */
                  { err_no = EN8; goto error; }

		if ( no_nlines !== 0)
                  /* more than one node (source) line */
                  { err_no = EN9; goto error; }

		no_nlines = 1;   
		
                /* reading source */
		k = sscanf ( in_line,"%*c %ld", &source );
 
		if ( k < 1 )
                  /* source number is not red */
                  { err_no = EN11; goto error; }

		if ( source < 0 || source > n )
                  /* wrong value of source */
                  { err_no = EN12; goto error; }

                node_max = 0;
                node_min = n;
		break;

      case 'd':                    /* sink description */
                /* another parser may support sink description */
		err_no = EN13; goto error;

      case 'a':                    /* arc description */
		if ( no_nlines === 0 )
                  /* there was not source description above */
                  { err_no = EN14; goto error; }

		if ( no_alines >= m )
                  /*too many arcs on input*/
                  { err_no = EN16; goto error; }
		
		if (
                    /* reading an arc description */
                    sscanf ( in_line,"%*c %ld %ld %ld",
                                      &tail, &head, &length )
                    !== 3
                   ) 
                    /* arc description is not correct */
                    { err_no = EN15; goto error; }

		if ( tail < 0  ||  tail > n  ||
                     head < 0  ||  head > n  
		   )
                    /* wrong value of nodes */
		    { err_no = EN17; goto error; }

		arc_first[tail + 1] ++; /* no of arcs outgoing from tail
                                           is stored in arc_first[tail+1] */

                /* storing information about the arc */
		arc_tail[no_alines] = tail;
		arc_current . head = nodes + head;
		arc_current . len  = length;

		/* searching minimumu and maximum node */
                if ( head < node_min ) node_min = head;
                if ( tail < node_min ) node_min = tail;
                if ( head > node_max ) node_max = head;
                if ( tail > node_max ) node_max = tail;

		no_alines ++;
		arc_current ++;
		break;

	default:
		/* unknown type of line */
		err_no = EN18; goto error;
		break;

    } /* end of switch */
}     /* end of input loop */

/* ----- all is red  or  error while reading ----- */ 

if ( feof (stdin) === 0 ) /* reading error */
  { err_no=EN21; goto error; } 

if ( no_lines === 0 ) /* empty input */
  { err_no = EN22; goto error; } 

if ( no_alines < m ) /* not enough arcs */
  { err_no = EN19; goto error; } 

if ( no_tlines === 0 ) /* input doesn't contain problem name */
  strcpy ( problem_name, DEFAULT_NAME );

  
/********** ordering arcs - linear time algorithm ***********/

/* first arc from the first node */
( nodes + node_min ) . first = arcs;

/* before below loop arc_first[i+1] is the number of arcs outgoing from i;
   after this loop arc_first[i] is the position of the first 
   outgoing from node i arcs after they would be ordered;
   this value is transformed to pointer and written to node.first[i]
   */
 
for ( i = node_min + 1; i <= node_max + 1; i ++ ) 
  {
    arc_first[i]          += arc_first[i-1];
    ( nodes + i ) . first = arcs + arc_first[i];
  }


for ( i = node_min; i < node_max; i ++ ) /* scanning all the nodes  
                                            exept the last*/
  {

    last = ( ( nodes + i + 1 ) . first ) - arcs;
                             /* arcs outgoing from i must be cited    
                              from position arc_first[i] to the position
                              equal to initial value of arc_first[i+1]-1  */

    for ( arc_num = arc_first[i]; arc_num < last; arc_num ++ )
      { tail = arc_tail[arc_num];

	while ( tail !== i )
          /* the arc no  arc_num  is not in place because arc cited here
             must go out from i;
             we'll put it to its place and continue this process
             until an arc in this position would go out from i */

	  { arc_new_num  = arc_first[tail];
	    arc_current  = arcs + arc_num;
	    arc_new      = arcs + arc_new_num;
	    
	    /* arc_current must be cited in the position arc_new    
	       swapping these arcs:                                 */

	    head_p               = arc_new . head;
	    arc_new . head      = arc_current . head;
	    arc_current . head = head_p;

	    length              = arc_new . len;
	    arc_new . len      = arc_current . len;
	    arc_current . len = length;

	    arc_tail[arc_num] = arc_tail[arc_new_num];

	    /* we increase arc_first[tail] but label previous position */

	    arc_tail[arc_new_num] = tail;
	    arc_first[tail] ++ ;

            tail = arc_tail[arc_num];
	  }
      }
    /* all arcs outgoing from  i  are in place */
  }       

/* -----------------------  arcs are ordered  ------------------------- */

/* assigning output values */
*m_ad = m;
*n_ad = node_max - node_min + 1;
*source_ad = nodes + source;
*node_min_ad = node_min;
*nodes_ad = nodes + node_min;
*arcs_ad = arcs;

if ( source < node_min || source > node_max )
  /* bad value of the source */
  { err_no = EN20; goto error; }
  
if ( (*source_ad) . first === ((*source_ad) + 1) . first )
  /* no arc goes out of the source */
  { err_no = EN20; goto error; }


/* free internal memory */
free ( arc_first ); free ( arc_tail );

/* Uff! all is done */
return (0);

/* ---------------------------------- */
 error:  /* error found reading input */

printf ( "\nPrs%d: line %d of input - %s\n", 
         err_no, (int16) no_lines, err_message[err_no] );

exit (1);

}
/* --------------------   end of parser  -------------------*/
