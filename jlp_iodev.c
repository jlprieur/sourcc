/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.TYPE        Module
.NAME        iodev
.LANGUAGE    C
.AUTHOR      IPG-ESO Garching
.COMMENTS    	Tape management. 
             	The functions of this module perform basic i/o to
             	magnetic tapes on Ultrix enviroments
.ENVIRONMENT 	UNIX
	ioinfo, ioopen, ioclose, ioread, iowrite, iorew, ioweof,
	iofsf,	* File Move Forward  	*
	iobsf,	* File Move Backward 	*
	iofsr,	* Block Move Forward  	*
	iobsr,	* Block Move Backward 	*
	ioeom	* Move to EOMedia 	*

Used by jlp_ctape...
------------------------------------------------------------*/
/*#define  DEBUG	1*/
#include <osudef.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <errno.h>
#include <osparms.h>

static struct mtget mt_stat;	/* MagTape Status	*/

extern int errno;
extern int oserror;
extern char *oserrmsg;

/* Definitions for the 1/2" Tape, SCSI controller SUN4/110 */
#define M_ST_MASK   077		/* Status code mask: ?? */
#define M_ST_SUCC   000		/* Command Successed: ?? */
#define M_ST_DATA   010		/* No Data to read: ?? */
#define M_ST_TAPEM  022		/* Tape Mark Encountared: ?? */
#define M_ST_BADBUF 024		/* Too short buffer ??  */
#define M_ST_BOT    077		/* Beginning of Tape: ?? */
#define M_ST_SEX    077		/* Serious Exception: ?? */
#define M_ST_RDTRN  043		/* Record Data Truncated: ?? */
#define M_ST_EOM    005		/* End Of Media: ?? */


int iostat(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Get status.
.RETURNS None
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
{
        oserror = 0;

        if (ioctl(fd,MTIOCGET,&mt_stat) == -1) {
           	/*     
		oserror = -1;
                oserrmsg = "Error in getting tape status\n";
		*/	
		oserror = errno;
                return(-1);
                }

#if DEBUG
        printf("type=0x%x\n",mt_stat.mt_type);
        printf("drive status=0%o\n",mt_stat.mt_dsreg);
        printf("error register=0%o\n",mt_stat.mt_erreg);
        printf("residual count=0x%x\n",mt_stat.mt_resid);
#endif

	/* If there is no error, check residual count. */

	if ((mt_stat.mt_erreg == 0) && (mt_stat.mt_resid > 0))
		mt_stat.mt_erreg = M_ST_BADBUF;

        return(mt_stat.mt_erreg & M_ST_MASK);
}

int clear_exc(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Clear a serious exception.
.RETURNS 0
.REMARKS Serious exception does not exist in ALLIANT
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
{
#if 0	/* This part is useless ? ?  */
        struct mtop mtop;
        mtop.mt_op = MTCSE;
        mtop.mt_count = 1;
        ioctl(fd,MTIOCTOP,&mtop);
#endif

        return(0);      /* MTCSE does not exit */
}

int ioctop(fd,op,count)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Interface to ioctl routine.
.RETURNS 0 (OK) / -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
int op;                 /* IN:  Operation */
int count;              /* IN:  How many of them */
{
        struct mtget mtget;
        struct mtop mtop;
        int stat, ret;

        mtop.mt_op = op;
        mtop.mt_count = count;

        /*
        ** First try to execute the command.
        ** Even if fails, try to read the status
        */
        if ((ret = ioctl(fd,MTIOCTOP,&mtop)) == -1)
                oserror = errno;

        /*
        ** Get status of last command
        */
        if ( (stat = iostat(fd)) == -1)
                return(-1);

        /*
        ** If there was an error,
        ** the error condition must be cleared before continue
        */
        if (ret == -1) {
                oserror = errno;
                clear_exc(fd);
                }

        return(stat);
}

int ioinfo(fd, s, fileno, blkno)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Retrieve Info concerning an opened device
.RETURNS 0 (success) / -1 (error)
------------------------------------------------------------*/
  	int	fd;	/* IN: File Descriptor		*/
	struct osustat	*s;	/* OUT: The filled components */
        int     *fileno;        /* OUT: Where we are    */
        long    *blkno;         /* OUT: Where we are    */
{
        struct stat buf;

        oserror = 0;
        if ( fstat(fd,&buf) == -1) {
                oserror= errno;
                return(-1);
                }

        s->usize     = 0;
        s->blocksize = 0;
        s->density   = 0;
	s->isda = 0;		/* Not direct access */
	s->istm = ((buf.st_mode & S_IFMT) == S_IFCHR);
	if (s->istm == 0) {
		oserror = -1;
		oserrmsg = "Device can't be a tape...";
		return(-1);
	}
	
		/* Get current position ... */
/* JLP 91 PB when compiling, so I supress: !!!!!
	if (fileno) {
		iostat(fd);
		*fileno = mt_stat.mt_fileno;
		*blkno  = mt_stat.mt_blkno;
		}
*/
	
	return(0);
}

int ioopen(name,mode,den)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Open a tape device
.RETURNS File descriptor / -1 (error)
------------------------------------------------------------*/
char *name;             /* IN:  Physical name of tape device */
int mode;               /* IN:  Open mode */
int den;                /* IN:  Density. Not used */
{
        struct stat buf;
        int fd;
        int t;

        oserror = 0;
        switch(mode) {          /* Open operations */
        case READ:      t = O_RDONLY; break;
        case WRITE:     t = O_WRONLY; break;
        case READ_WRITE: t = O_RDWR; break;
        case APPEND:    t = O_RDWR; break;
        default:        oserror = EINVAL; return(-1);
        }

        if ( (fd = open(name,t)) == -1) {
                oserror= errno;
                return(-1);
                }

        if ( fstat(fd,&buf) == -1) {
                oserror= errno;
                return(-1);
                }

        if ( (buf.st_mode & S_IFMT) != S_IFCHR) {
                oserror = -1;
                oserrmsg = "Osuopen: Not a character device";
                return(-1);
                }
        return(fd);
}

int ioclose(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Close the opened device
.RETURNS 0
------------------------------------------------------------*/
int fd;                 /* IN:  Tape file descriptor */
{
        oserror=0;

        if (close(fd) == -1) {
#if 1 			/* Because a bug in the driver, I think */
 		return(0);
#else
                oserror = errno;
                return(-1);
#endif
                }
        return(0);
}

int ioread(fd,buffer,size)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Read a block from a magnetic tape.
.RETURNS Bytes read / -1 if error
.REMARKS 0 Bytes read, means a File Mark was detected.
.REMARKS oserror set to -2 if buffer too small
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
char *buffer;           /* IN:  Buffer for reading */
int size;               /* IN:  Length of bytes to be read */
{
        struct mtget mtget;
        int stat, length, the_error;

        oserror = 0;

        if ((length = read(fd,buffer,size)) == -1)  
		the_error = oserror = errno;
#if DEBUG
	printf("ioread read %d to buffer of %d bytes\n", length, size);
#endif
        /*
        ** Get status of last command
        */
        if ( (stat = iostat(fd)) == -1)
                return(-1);

        /*
        ** If there was an error,
        ** the error condition must be cleared before continue
        */
        if (length == -1 || stat != M_ST_SUCC)
                clear_exc(fd);

	/*
	** If there is no error, return
	*/
	if (length >= 0)	return(length);

	/*
	** HERE WHEN ERROR
	** Try to find an explanation of the error
	*/
	switch(stat){
           case M_ST_DATA: 
           	/*   oserror = EIO;   */
                return(0);

		/*
		** if a file mark detected, then skip it. 
		** read, can not skip file mark by itself .
       		   if (stat == M_ST_TAPEM) 
			iofsf(fd,1);
		*/
           case M_ST_BADBUF: 
		oserror = -2;	/* TOO SHORT BUFFER	*/
		oserrmsg = "Too short buffer";
		break;
	   default:		/* Get the original error */
		oserror = the_error;
		break;
	}
	
        return(-1);
}

int iowrite(fd,buffer,size)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Write a block on a magnetic tape.
.RETURNS Bytes written / -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
char *buffer;           /* IN:  Buffer for reading */
int size;               /* IN:  Length of bytes to be read */
{
        struct mtget mtget;
        int stat, length;

        oserror =0;

        if ((length = write(fd,buffer,size)) == -1 )         
		{ oserror = errno; return(-1); }

#if DEBUG
	printf("iowrite %d to buffer of %d bytes\n", length, size);
#endif
        /*
        ** Get status of last command
        */
        if ( (stat = iostat(fd)) == -1)
                return(-1);

        /*
        ** If there was an error,
        ** the error condition must be cleared before continue
        */
        if (length == -1 || stat != M_ST_SUCC)
                clear_exc(fd);

        return(length);
}

int ioweof(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Write end-of-file record (tape_mark) on the tape.
.RETURNS 0 / -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
{
	oserror = 0;

        if (ioctop(fd,MTWEOF,1) == -1)
                return(-1);
        else 
                return(0);
}

int iofsf(fd,ntm)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Skip forward space file on a tape. 
.RETURNS Tape marks skipped/ -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
int ntm;                /* IN:  Number of tape marks */
{
        int ret, stat;

        oserror = 0;

        stat = ioctop(fd,MTFSF,ntm);

        switch(stat) {
        case M_ST_SUCC:                 /* Command OK */
                ret = ntm; 
                break;
        case M_ST_DATA:                 /* End of data */
                printf("fsf/error: end of data \n");
                oserror = 0;
                ret = 0; 
                break;
        default: 
                ret = -1; 
                break;
        }
        return(ret);
}

int iobsf(fd,ntm)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Skip backward space file on a tape. 
.RETURNS Tape marks skipped/ -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
int ntm;                /* IN:  Number of tape marks */
{
        int ret, stat;


        oserror = 0;
        stat = ioctop(fd,MTBSF,ntm);

        switch(stat) {
        case M_ST_SUCC:                 /* Command OK */
                ret = ntm; 
                break;
        case M_ST_BOT:                  /* Beggining of tape */
                oserror = 0;
                ret = 0; 
                break;
        default: 
                ret = -1; 
                break;
        
        }
        return(ret);

}

int iorew(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Rewind tape
.RETURNS 0 / -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
{
	int res;

        oserror = 0;

        res=ioctop(fd,MTREW,0);
        if ((res != M_ST_SUCC) && (res != M_ST_BOT))
                return(-1);
        else 
                return(0);
}

int iofsr(fd,count)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Forward space record on tape
.RETURNS records skipped/ -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
int count;              /* IN:  Number of records */
{
        int ret, stat;

        oserror = 0;

        stat = ioctop(fd,MTFSR,count);

        switch(stat) {
        case M_ST_SUCC:                 /* Command OK */
                ret = count; 
                break;
        case M_ST_DATA:                 /* End of records */
                oserror = 0;
                ret = 0; 
                break;
        default: 
                ret = -1; 
                break;
        }
        return(ret);

}

int iobsr(fd,count)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Backward space record on tape
.RETURNS records skipped/ -1 (error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
int count;              /* IN:  Number of records */
{
        int ret, stat;

        oserror = 0;

        stat = ioctop(fd,MTBSR,count);

        switch(stat) {
        case M_ST_SUCC:                 /* Command OK */
                ret = count; 
                break;
        case M_ST_BOT:                  /* Beggining of tape */
                oserror = 0;
                ret = 0; 
                break;
        default: 
                ret = -1; 
                break;
        }
        return(ret);

}

#ifdef MTEOM	/* Does the driver know the End-Of-Media ? */
int ioeom(fd)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.PURPOSE Move to EOMedia
.RETURNS 0 (OK) / -1 (not done, error)
------------------------------------------------------------*/
int fd;                 /* IN:  Tape device file descriptor */
{
        int stat;

        oserror = 0;
        stat = ioctop(fd,MTEOM,1);
	if (stat == -1)	oserror = errno;

        switch(stat) {
        case M_ST_SUCC:                 /* Command OK */
                break;
        case M_ST_EOM:                  /* End of tape */
                oserror = 0;
                stat = 0; 
                break;
        default: 
                stat = -1; 
	}
	return(stat);
}
#endif

