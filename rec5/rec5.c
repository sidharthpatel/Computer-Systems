#include <stdio.h>   /* printf  */
#include <string.h>  /* strncmp, strdup */
#include <stdlib.h>  /* atoi, strtol    */

extern long        mul_20 (long);
extern long          add3 (long, long, long );
extern long          max2 (long,long);
extern long       sumUpTo (long);
extern long collatzLength (long);
extern long        lookup (long[], long, long);
extern long          fact (long);
extern long      maxArray (long[], long);
extern long        caller (long, long);

extern long         func1 (long );
extern long         func2 (long,long,long,long);
extern long         func3 (long*, long*, long);
extern long         func4 (long, long);
extern long         func5 (long);

/*
receives command line arguments for:
 #1 function to be tested
 #2 first argument
 (#3+: more arguments, or array values for the last argument)
*/
int main(int argc, char ** argv){
  /* check for minimum number of arguments */
  if (argc<3){
    printf("error - not enough arguments.\n\n\tusage: ./rec4 funcname arg1 arg2 arg3 ...\n\n");
    return 1;
  }
  
  /* read in first function argument (always a long) */
  long n;
  sscanf(argv[2],"%ld",&n);
  
  /* dispatch to the correct function */
  char* funcName = argv[1];
  
  
  if ( ! strncmp("mul_20", funcName,10)){ printf("%ld\n",mul_20 (n)); }
  
  
  else if ( ! strncmp("add3",funcName,10)){
    long a = n;
    long b, c ;
    sscanf(argv[3],"%ld",&b);
    sscanf(argv[4],"%ld",&c);
    printf("%ld\n",add3(a,b,c));
  }
  
  
  else if ( ! strncmp("max2",  funcName,10)){
    long x = n;
    long y;
    sscanf(argv[3],"%ld",&y);
    printf("%ld\n",max2  (x,y));
  }
  
  
  else if ( ! strncmp("sumUpTo",funcName,10)){
    printf("%ld\n",sumUpTo(n));
  }
  
  
  else if ( ! strncmp("collatzLength",funcName,15)){
    printf("%ld\n",collatzLength(n));
  }
  
  
  else if ( ! strncmp("fact",funcName,10)){
    printf("%ld\n",fact(n));
  }
  
  
  
  else if ( ! strncmp("caller",funcName,10)){
    long x = n;
    long y;
    sscanf(argv[3],"%ld",&y);
    printf("%ld\n",caller(x,y)); 
  }
  
  
else if ( ! strncmp("func2",funcName,10)){
    long a = n;
    long b, c, d;
    sscanf(argv[3],"%ld",&b);
    sscanf(argv[4],"%ld",&c);
    sscanf(argv[5],"%ld",&d);
    printf("%ld\n",func2(a,b,c,d));
  }

  else if ( ! strncmp("func1", funcName,10)){
    printf("%ld\n",func1 (n));
  }


 else if ( ! strncmp("func3",funcName,10)){
    long x = n;
    long y;
    long z;
    sscanf(argv[3],"%ld",&y);
    sscanf(argv[4],"%ld",&z);
    printf("%ld\n",func3(&x,&y,z));
  }


  /* give a helpful message when the function is misspelled. */
  else {
    printf("error - unrecognized command '%s'.\n",argv[1]);
    return 2;
  }
}
