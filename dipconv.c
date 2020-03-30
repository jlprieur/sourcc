/*
 *  Name:
 *     dipconv
 
 *  Purpose:
 *     Re-formats a native dipso VMS data file so that it can be read by
 *     the UNIX version of dipso.

 *  Invocation:
 *     dipconv [-spr] input_file output_file

 *  Flags:
 *     -s  The input file is a dipso stack file.
 *     -p  The input file is an SP0 file.
 *     -r  The input file was created using the WRITE or OWRITE dipso 
 *         command.

 *  Description:
 *     The input file should have been produced by the vbackup utility 
 *     (produced by Boston Business Computing) which restores VMS BACKUP 
 *     save sets. The -Q option should have been used with vbackup to 
 *     ensure that the original VMS file structure is retained.
 *
 *     The input file should be a "native" dipso binary data file (i.e. not 
 *     an NDF).
 *   
 *     The original VMS dipso data files were written with fortran variable 
 *     length records. This causes control words (each of 2 bytes) to be 
 *     stored in the output file along with the actual data. These control 
 *     words occur at the start and end of each record. A control word
 *     is also placed at the very start of the file. Records which are
 *     longer than 2048 bytes are split up into multiple records, with
 *     the appropriate control words at the start and end of each record.
 *
 *     Fortran unformatted records on UNIX are different. Each record is
 *     preceeded and followed by a control long-word (4 bytes). There 
 *     are no extra control bytes at the start of the file (over and
 *     above those required for the first record). The control long-words
 *     have the same value at each end of the record, equal to the number
 *     of bytes of data in the record. Records can be of any size and need 
 *     not be split into 2048 segments.
 *
 *     This application extracts the data bytes from each record of the 
 *     input file (throwing away the control words), converts the
 *     floating point or integer values to unix format, and writes them to
 *     the output file together with approriate UNIX control long-words.

 *  Authors:
 *     David Berry (DSB):

 *  History:
 *     13-APR-1995 (DSB):
 *        Original version.
 *     24-APR-1995 (DSB):
 *        Modified to handle SP0 and WRITE files as well as stack files.

 */



#include <stdio.h>
#include <math.h>

#define C_SIZE 200000
#define STACK 1
#define SP0 2
#define OWRITE 3

FILE *fd1;
FILE *fd2;

int readb( int n, char *buf );
float vmsreal( char *bytes );
int   vmsinteger( char *bytes );

void main( int argc, char *argv[] ){
      char c[ C_SIZE ];
      int n, nstore, i, bstnpt, stknpt, j, ival, itype, npoint, here;
      float rval;


/*  Check that some parameters were given on the command line. */

      printf("\n");
      if( argc < 2 ){
         printf( "DIPCONV: Usage is 'dipconv  [-spr] <input file>  <output file>'\n");
         printf("\n");
         exit(0);
      }
      

/*  See if the first argument on the command line is an option, if so 
 *  set a corresponding flag. */

      if( *( argv[1] ) == '-' ) {
   
         if( *( argv[1] + 1 ) == 's' ) {
            itype = STACK;                    

         } else if( *( argv[1] + 1 ) == 'p' ) {
            itype = SP0; 

         } else if( *( argv[1] + 1 ) == 'r' ) {
            itype = OWRITE; 

         } else {
            printf( "DIPCONV: Unknown flag -%c. Allowed flags are: \n", *( argv[1] + 1 ) );
            printf( "            -s for stack files\n");
            printf( "            -p for SP0 files\n");
            printf( "            -r for files created with (O)WRITE \n");
            printf("\n");
            exit(0);
         }


/*  Check that there are sufficient remaining command-line argumemts. */

         if( argc < 4 ){
            printf( "DIPCONV: Usage is 'dipconv  [-spr] <input file>  <output file>'\n");
            printf("\n");
            exit(0);
         }


/*  Move the arguments giving the input and output files to positions 1
 *  and 2. */

         argv[1] = argv[2];
         argv[2] = argv[3];
 

/*  If no option was given, assume stack files are being converted, and
 *  check that both input and output file names have been supplied on the
 *  command line */

      } else {
         itype = STACK;

         if( argc < 3 ){
            printf( "DIPCONV: Usage is 'dipconv  [-spr] <input file>  <output file>'\n");
            printf("\n");
            exit(0);
         }

      }


/*  Open the input and output binary files. */

      if( ( fd1 = fopen( argv[1], "r" ) ) == 0 ){
         perror("DIPCONV");
         printf("DIPCONV: Unable to open input file '%s'.\n", argv[1] );
         printf("\n");
         exit(0);
      }


      if( ( fd2 = fopen( argv[2], "w" ) ) == 0 ){
         perror("DIPCONV");
         printf("DIPCONV: Unable to open output file '%s'.\n", argv[2] );
         printf("\n");
         exit(0);
      }



/*  Skip over the two control bytes at the start of the file. */

      fread( c, 1, 2, fd1 );


/*  Now process the particular type of dipso file...*/

      if( itype == STACK ) {


/*  The first VMS record contains a single longword giving the no. of 
 *  stack entries stored in the file. Get the 4 bytes and calculate the 
 *  number of stack entries. */

         readb( 4, c );
         nstore = vmsinteger( c );
         printf("\n");
         printf("No. of stacks: %d\n", nstore );

         if( nstore < 1 || nstore > 10000 ) {
            printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
            printf("\n");
            exit(0);
         }


/*  Write the no. of stack entries to the output file. UNIX unformatted
 *  sequential records start and finish with a longword byte count. */

         n=4;
         fwrite( &n, sizeof( int ), 1, fd2 );
         fwrite( &nstore, 1, 4, fd2 );
         fwrite( &n, sizeof( int ), 1, fd2 );
 

/*  Copy each stack entry. */

         for( i = 1; i <= nstore; i++ ) {
            printf("\n");
            printf("Stack %d ...\n", i );


/*  Title... */

            n=80;
            readb( n, c );

            j=79;
            while( c[j] == 32 && j >= 0 ) j--; 
            j++;
            c[j]=0;
            printf( "  Title        : %s\n",c);
            c[j]=32;

            fwrite( &n, sizeof( int ), 1, fd2 );
            fwrite( c, 1, n, fd2 );
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  worvst... */

            n=4;
            readb( n, c );
            rval = vmsreal( c );
            printf( "  Worvst       : %f\n", rval );
            fwrite( &n, sizeof( int ), 1, fd2 );
            fwrite( &rval, n, 1, fd2 );
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  Stack size... */

            n=4;
            readb( n, c );
            stknpt = vmsinteger( c );
            printf( "  Size         : %d\n", stknpt );
            if( stknpt > C_SIZE/4 ) {
               printf( "DIPCONV: Stack array too big\n" );
               printf("\n");
               exit(0);
            }
 
            if( stknpt < 1 ) {
               printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
               printf("\n");
               exit(0);
            }

            fwrite( &n, sizeof( int ), 1, fd2 );
            fwrite( &stknpt, 4, 1, fd2 );
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  No. of breaks... */

            n=4;
            readb( n, c );
            bstnpt = vmsinteger( c );
            printf( "  No. of breaks: %d\n", bstnpt );
            if( bstnpt > C_SIZE/4 ) {
               printf( "DIPCONV: Too many breaks\n" );
               printf("\n");
               exit(0);
            }

            if( bstnpt < 1 ) {
               printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
               printf("\n");
               exit(0);
            }

            fwrite( &n, sizeof( int ), 1, fd2 );
            fwrite( &bstnpt, 4, 1, fd2 );
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  The X stack data... */

            n=stknpt*4;
            readb( n, c );
            fwrite( &n, sizeof( int ), 1, fd2 );
            for( j=0; j<n; j += 4 ) {
               rval = vmsreal( &c[j] );
               fwrite( &rval, 4, 1,  fd2 );
            }
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  The Y stack data... */

            n=stknpt*4;
            readb( n, c );
            fwrite( &n, sizeof( int ), 1, fd2 );
            for( j=0; j<n; j += 4 ) {
               rval = vmsreal( &c[j] );
               fwrite( &rval, 4, 1,  fd2 );
            }
            fwrite( &n, sizeof( int ), 1, fd2 );


/*  The breaks array... */

            n=bstnpt*4;
            readb( n, c );
            fwrite( &n, sizeof( int ), 1, fd2 );
            for( j=0; j<n; j += 4 ) {
               ival = vmsinteger( &c[j] );
               fwrite( &ival, 4, 1,  fd2 );
            }
            fwrite( &n, sizeof( int ), 1, fd2 );


         }


/*  Now do SP0 files ... */

      } else if( itype == SP0 ) {


/*  Title... (read 80 characters from the VMS file even though the title
 *  is only declared in dipso as 79 characters long because VMS pads out
 *  strings with nulls to a word boundary ). */

         n = 80;
         readb( n, c );

         j=78;
         while( c[j] == 32 && j >= 0 ) j--; 
         j++;
         c[j]=0;
         printf( "  Title        : %s\n",c);
         c[j]=32;

         n = 79;
         fwrite( &n, sizeof( int ), 1, fd2 );
         fwrite( c, 1, n, fd2 );
         fwrite( &n, sizeof( int ), 1, fd2 );


/*  IHHEAD... */

         n=80;
         readb( n, c );

         j=78;
         while( c[j] == 32 && j >= 0 ) j--; 
         j++;
         c[j]=0;
         printf( "               : %s\n",c);
         c[j]=32;

         n=79;
         fwrite( &n, sizeof( int ), 1, fd2 );
         fwrite( c, 1, n, fd2 );
         fwrite( &n, sizeof( int ), 1, fd2 );


/*  Array size */

         n=4;
         readb( n, c );
         npoint = vmsinteger( c );
         printf( "  Size         : %d\n", npoint );
         if( npoint > C_SIZE/8 ) {
            printf( "DIPCONV: Stack array too big\n" );
            printf("\n");
            exit(0);
         }
 
         if( npoint < 1 ) {
            printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
            printf("\n");
            exit(0);
         }

         fwrite( &n, sizeof( int ), 1, fd2 );
         fwrite( &npoint, 4, 1, fd2 );
         fwrite( &n, sizeof( int ), 1, fd2 );



/*  The data... */

         n=2*( npoint*4 );
         readb( n, c );
         fwrite( &n, sizeof( int ), 1, fd2 );
         for( j=0; j<n; j += 4 ) {
            rval = vmsreal( &c[j] );
            fwrite( &rval, 4, 1,  fd2 );
         }
         fwrite( &n, sizeof( int ), 1, fd2 );


/*  Now do files created with the WRITE or OWRITE commands ... */

      } else if( itype == OWRITE ) {


/*  Title... */

         n = 80;
         readb( n, c );

         j=79;
         while( c[j] == 32 && j >= 0 ) j--; 
         j++;
         c[j]=0;
         printf( "  Title        : %s\n",c);
         c[j]=32;

         fwrite( &n, sizeof( int ), 1, fd2 );
         fwrite( c, 1, n, fd2 );
         fwrite( &n, sizeof( int ), 1, fd2 );


/* Store the current file position in the input file. */

         here = ftell( fd1 );


/*  No. of breaks (do not write it to the output file yet). This is 
 *  stored in the first longword of a record which also contains the 
 *  break values. */

         n=4;
         readb( n, c );
         bstnpt = vmsinteger( c );
         printf( "  No. of breaks: %d\n", bstnpt );
         if( bstnpt > C_SIZE/4 - 1 ) {
            printf( "DIPCONV: Too many breaks\n" );
            printf("\n");
            exit(0);
         }
 
         if( bstnpt < 1 ) {
            printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
            printf("\n");
            exit(0);
         }

/*  Move the file pointer back to the start of the record containing the
 *  breaks. */

         if( fseek( fd1, here, SEEK_SET ) != 0 ) {
            perror("DIPCONV");
            printf("DIPCONV: Unable to re-position pointer within input file\n" );
            printf("\n");
            exit(0);
         }


/*  Copy the break data (this includes the no. of breaks)... */

         n= ( 1 + bstnpt )*4;
         readb( n, c );

         fwrite( &n, sizeof( int ), 1, fd2 );
         for( j=0; j<n; j += 4 ) {
            ival = vmsinteger( &c[j] );
            fwrite( &ival, 4, 1,  fd2 );
         }
         fwrite( &n, sizeof( int ), 1, fd2 );


/*  The last break point gives the number of points in the X and Y
 *  arrays. */

         npoint = ival;
         printf( "  Size         : %d\n", npoint );
         if( npoint > C_SIZE/8 ) {
            printf( "DIPCONV: Stack array too big\n" );
            printf("\n");
            exit(0);
         }
 
         if( npoint < 1 ) {
            printf( "  WHAT!!  (looks like a corrupted or inappropriate input file) \n");
            printf("\n");
            exit(0);
         }


/*  X and Y data ... */

         n= ( 2*npoint )*4;
         readb( n, c );

         fwrite( &n, sizeof( int ), 1, fd2 );
         for( j=0; j<n; j += 4 ) {
            rval = vmsreal( &c[j] );
            fwrite( &rval, 4, 1,  fd2 );
         }
         fwrite( &n, sizeof( int ), 1, fd2 );


/*  WORV - this is not present in all files, so check the success of the
 *  read operation. */
         
         n = 4;
         if( readb( n, c ) ) {
            rval = vmsreal( c );
            printf( "  Worvst       : %f\n", rval );
            fwrite( &n, sizeof( int ), 1, fd2 );
            fwrite( &rval, n, 1, fd2 );
            fwrite( &n, sizeof( int ), 1, fd2 );

         } else {
            printf( "  Worvst       : <not present>\n", rval );

         }

     }


/*  Close the files. */

      fclose( fd1 );
      fclose( fd2 );

      printf("\n");

}



int readb( int n, char *buf ){
/*
 *  Read n bytes from a VMS variable length record and return them in 
 *  argument buf, skipping control words.
 */
      int ib, i;
      char c[2];


/*  Skip the first two bytes in the record which is a control word.*/

      if( fread( c, 1, 2, fd1 ) != 2 ) return(0);


/*  Initialise the byte offset into the current 2048 byte VMS variable
 *  length record. */

      ib = 0;


/*  Transfer all the requested bytes. */

      for( i=0; i<n; i++ ){


/*  There is a control word at the start and end of every 2048 byte
 *  record. If the control words have been reached, skip them. */

         if( ib == 2042 ) {
            if( fread( c, 1, 2, fd1 ) != 2 ) return(0);
            if( fread( c, 1, 2, fd1 ) != 2 ) return(0);
            ib = 0;
         }


/*  Read the next required byte. */

         if( fread( &buf[ i ], 1, 1, fd1 ) !=1 ) return(0);


/*  Increment the offset into the current 2048 byte record. */

         ib++;

      }


/*  Skip over the control word at the end. */

      fread( c, 1, 2, fd1 );

      return(1);

}



float vmsreal( char *bytes ){
/*
 * Interpret the first 4 bytes in the supplied array as a VMS real
 * value and return the corresponding unix floating value.
 */

      unsigned int frac;
      int exp;
      float ans;

/*  Set up a structure which overlays the supplied 4 bytes and divides
 *  then up into the various required fields. Solaris packs bit fields
 *  from the most significant bit downwards, where as OSF packs them from 
 *  the least significant bit upwards. Therefore separate structures are 
 *  required for the two operating systems. 

 *  Solaris... */

#if defined( sun4_Solaris )

      struct VMS {
         unsigned f6 : 1; /* exponent bit 0 (lsb) */
         unsigned f5 : 7; /* mantissa bits 16 to 22 (msb) */
         unsigned f4 : 1; /* sign bit */
         unsigned f3 : 7; /* exponent bits 1 to 7 (msb) */
         unsigned f2 : 8; /* mantissa bits 0 (lsb) to 7 */
         unsigned f1 : 8; /* mantissa bits 8 to 15 */
             } *vms;

/*  OSF ... */

#elif defined( alpha_OSF1 ) 

      struct VMS {
         unsigned f5 : 7; /* mantissa bits 16 to 22 (msb) */
         unsigned f6 : 1; /* exponent bit 0 (lsb) */
         unsigned f3 : 7; /* exponent bits 1 to 7 (msb) */
         unsigned f4 : 1; /* sign bit */
         unsigned f2 : 8; /* mantissa bits 0 (lsb) to 7 */
         unsigned f1 : 8; /* mantissa bits 8 to 15 */
            } *vms;
#endif


/*  Overlay the structure on top of the supplied bytes */

      vms = (struct VMS *) bytes;


/*  Extract the mantissa as an unsigned integer value */

      frac = ( vms->f2 ) + ( vms->f1 << 8 ) + ( vms->f5 << 16 ) + 
             ( 1 << 23 );


/*  Extract the exponent as an unsigned integer value. Subtract off 128
 *  to account for the exponent bias used in VMS. Also subtract of 24
 *  to shift the mantissa to the right of the binary point. */

      exp = ( vms->f6 ) + ( vms->f3 << 1 ) - 152;


/*  Form the answer as the mantissa times two to the power of exponent */

      ans = (float) ldexp( (double) frac, exp );


/*  Invert the answer if the sign bit is set. */

      if( vms->f4 ) ans = -ans ;

      return( ans );

}

int vmsinteger( char *bytes ){
/*
 * Interpret the first 4 bytes in the supplied array as a VMS integer
 * value and return the corresponding unix integer value.
 */
      union {
         unsigned int unsgn;
         int sgn;
            } ans;

      struct VMS {
         unsigned f4 : 8; /* vms bits 0 (lsb) to 7 */
         unsigned f3 : 8; /* vms bits 8 to 15 */
         unsigned f2 : 8; /* vms bits 16 to 23 */
         unsigned f1 : 8; /* vms bits 24 to 31 (msb) */
            } *vms;

      vms = (struct VMS *) bytes;
      ans.unsgn = ( vms->f4 ) + ( vms->f3 << 8 ) + ( vms->f2 << 16 ) + 
                  ( vms->f1 << 24 );
      
      return( ans.sgn );

}
