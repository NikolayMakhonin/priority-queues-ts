/* parser for getting  DIMACS format input and transforming the
   data to the internal representation */

export const MAXLINE = 100;	/* max line length in the input file */
export const ARC_FIELDS = 3;	/* no of fields in arc line  */
export const P_FIELDS = 3;       /* no of fields in problem line */
#define PROBLEM_TYPE "cut"      /* name of problem type*/

void setLastArcs ()

{
  let v: node*;
  let a: arc*;

  ForAllNodes ( v ) {
    for ( a = v . first; a . next != null; a = a . next ) ;
    v . last = a;
  }

}

void setPrevArcs ()

{
  let v: node*;
  let a: arc*;

  ForAllNodes ( v ) {
    v . first . prev = null;
    ForAllArcs ( v, a )
      if ( a . next != null )
	a . next . prev = a;
  }
}

/* initial initialization after parsing */
void initInit ()

{
  let v: node*;

  setLastArcs ();
  setPrevArcs ();

}

export function parse( ): int16 {

let n: int32,                      /* internal number of nodes */
        node_min,               /* minimal no of node  */
        node_max,               /* maximal no of nodes */
        head, tail, i;

let acap: double;                   /* input arc capacity */
let m: int32,                      /* internal number of arcs */
        /* temporary variables carrying no of arcs */
        last, arc_num, arc_new_num;

let pnodes: node*,                 /* pointers to the node structure */
        *head_p,
        ndp,
        i_n,
        j_n;

let parcs: arc*,
        arc_current, r_arc_current,
        arc_new,
        arc_tmp;

int32    no_lines=0,             /* no of current input line */
        no_plines=0,            /* no of problem-lines */
        no_tlines=0,            /* no of title(problem name)-lines */
        no_nlines=0,            /* no of node lines */
        no_alines=0;            /* no of arc-lines */

let in_line: char[MAXLINE],       /* for reading input line */
        pr_type[3];             /* for reading type of the problem */

let k: int32;                      /* temporary */
let err_no: int16;                 /* no of detected error */

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
export const EN23 = 21;

static err_message: string[] =
  { 
/* 0*/    "more than one problem line",
/* 1*/    "wrong number of parameters in the problem line",
/* 2*/    "it is not a Min-cost problem line",
/* 3*/    "bad value of a parameter in the problem line",
/* 4*/    "can't obtain enough memory to solve this problem",
/* 5*/    "",
/* 6*/    "can't read problem name",
/* 7*/    "problem description must be before node description",
/* 8*/    "wrong lowest flow or capasity",
/* 9*/    "wrong number of parameters in the node line",
/*10*/    "wrong value of parameters in the node line",
/*11*/    "unbalanced problem",
/*12*/    "node descriptions must be before arc descriptions",
/*13*/    "too many arcs in the input",
/*14*/    "wrong number of parameters in the arc line",
/*15*/    "wrong value of parameters in the arc line",
/*16*/    "unknown line type in the input",
/*17*/    "reading error",
/*18*/    "not enough arcs in the input",
/*19*/    "warning: too big capasities - possible excess overflow",
/*20*/    "can't read anything from the input file",
/*21*/    "capacities must be positive"
  };
/* --------------------------------------------------------------- */

while ( gets ( in_line ) != null )
  {
  no_lines ++;


  switch (in_line[0])
    {
      case 'c':                  /* skip lines with comments */
      case '\0':                 /* skip empty lines at the end of file */
                break;

      case 'p':                  /* problem description      */
                if ( no_plines > 0 )
                   /* more than one problem line */
                   { err_no = EN1 ; goto error; }

                no_plines = 1;
   
                if (
        /* reading problem line: type of problem, no of nodes, no of arcs */
                    sscanf ( in_line, "%*c %3s %ld %ld", pr_type, 
			    &input_n, &input_m )
                !== P_FIELDS
                   )
		    /*wrong number of parameters in the problem line*/
		    { err_no = EN2; goto error; }

                if ( strcmp ( pr_type, PROBLEM_TYPE ) )
		    /*wrong problem type*/
		    { err_no = EN3; goto error; }

                if ( input_n <= 0 )
		    /*wrong value of no of arcs or nodes*/
		    { err_no = EN4; goto error; }

		n = input_n + 1;
		m = 2 * input_m;


		/* allocating memory */
                pnodes    = (node *) new Array(n+1);
		parcs     = (arc *)  new Array(m+3);

                if ( pnodes == null || parcs == null )
                    /* memory is not allocated */
		    { err_no = EN6; goto error; }
		     
		for ( i_n = pnodes; i_n < pnodes + n + 1; i_n++ )
		  i_n . first = null;

		/* setting pointer to the first arc */
		sentinel_arc = parcs + input_m + 1;
		r_arcs   = sentinel_arc + 1;
		arc_current = sentinel_arc - 1;
		r_arc_current = r_arcs;
                node_max = 0;
                node_min = n;
                break;

      case 'a':                    /* arc description */

		if ( no_alines >= input_m )
                  /*too many arcs on input*/
                  { err_no = EN16; goto error; }
		
		if (
                    /* reading an arc description */
                    sscanf ( in_line,"%*c %ld %ld %lf ",
                                      &tail, &head, &acap )
                    !== ARC_FIELDS
                   ) 
                    /* arc description is not correct */
                    { err_no = EN15; goto error; }

		if ( acap <= 0 )
		    { err_no = EN17; goto error; }

		if ( tail < 0  ||  tail > input_n  ||
                     head < 0  ||  head > input_n  
		   )
                    /* wrong value of nodes */
		    { err_no = EN17; goto error; }

		i_n    = pnodes + tail;
		j_n    = pnodes + head;

                /* storing information about the arc */
		arc_current       . head    = j_n;
		arc_current       . cap     = acap;
		arc_current       . next    = i_n . first;
		i_n . first = arc_current;
		
		r_arc_current . head    = i_n;
		r_arc_current . cap     = acap;
		r_arc_current . next    = j_n . first;
		j_n . first = r_arc_current;

		/* searching for minimum and maximum node */
                if ( head < node_min ) node_min = head;
                if ( tail < node_min ) node_min = tail;
                if ( head > node_max ) node_max = head;
                if ( tail > node_max ) node_max = tail;

		no_alines   ++;
		arc_current --;
		r_arc_current ++;

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

if ( no_alines < input_m ) /* not enough arcs */
  { err_no = EN19; goto error; } 

sentinelNode = pnodes + node_max + 1;

arcs = arc_current + 1;
last_r_arc = r_arc_current - 1;
nodes = pnodes + node_min;

initInit ();

return (0);

/* ---------------------------------- */
 error:  /* error found reading input */

fprintf ( stderr, "\nline %d of input - %s\n", 
         no_lines, err_message[err_no] );

exit (1);

}
/* --------------------   end of parser  -------------------*/
