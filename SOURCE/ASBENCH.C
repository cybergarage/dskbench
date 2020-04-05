//
//  ASBENCH : ASPI SCSI benchmark test for hard-disk / MO / CD-ROM
//      copyright(c) by TsuruZoh Tachibanaya
//
//      V0.1    Oct.30,1992 ;first release
//      V0.2    Nov.23,1992 ;add bar-graph display
//      V0.3    Nov.04,1996 ;test end of disk sequential, Win32 console port
//      V0.5    Aug.14,1998 ;fix error on very large disk
//      V0.5b   Aug.26,1998 ;fix error on more than 7 ID
//      V0.6    Sep.14,1998 ;accept also see CDWriter
//      V0.7    Jan.30,1999 ;fix error with very fast disk
//

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#ifndef _MSC_VER
#include <alloc.h>
#else
#include <malloc.h>
#define farmalloc _fmalloc
#define farfree _ffree
#endif
#else
#define far
#define farmalloc malloc
#define farfree free
int random(int imax);
#pragma warning (disable:4005)
#ifdef putchar
#undef putchar
#endif
#define putchar(a) (printf("%c",(a)))
#endif
#include <time.h>
#include "aspi.h"

#define TESTTIME 5  // measurement length is 5 sec.
#define MAXID (15)

static int commandRank[ 2 ] = { 25, 35 };
static int seekRank[ 2 ] = { 100, 400 };
static int readRank[ 3 ] = { 20000, 12000,20000 };

int main( void );
int GetTarget( void );
int GetHostNo( void );
int GetInquiry( int, unsigned char[15][15][40] );
void DispInquiry( int, int, unsigned char * );
int CommandBench( int, int, int );
int SeekBench( int, int, unsigned long, int );
int ReadBench( int, int, unsigned char far *, int, unsigned long, int );


int main()
{
    int hostNo, targetId, sectorSize;
    unsigned char far *dataBuffer;
    unsigned char tempBuf[ 80 ];
    unsigned long lbaMax;

    printf( "\nASPI SCSI benchmark test V0.7  -  http://www.winimage.com/asbench.htm\n" );
    #ifndef WIN32
    printf( " copyright(c) by Tsuru-Zoh, Sep. 14,1998, modified by Gilles Vollant\n\n" );
    #else
    printf( " copyright(c) by Tsuru-Zoh, Sep. 14,1998, Win32 Version by Gilles Vollant\n\n" );
    #endif    
    if( AspiInit() ){
        return( -1 );
    }
    
    if( ( targetId = GetTarget() ) == -1 ){
        return( 0 );
    }
    hostNo = targetId >> 4;
    targetId = targetId & MAXID;

    if( ReadCapacity( hostNo, targetId, tempBuf ) ){
        printf( "Can't get sector size.\n" );
        return( -1L );
    }
    sectorSize = tempBuf[ 6 ] * 256 + tempBuf[ 7 ];
    lbaMax = tempBuf[0] * 65536L * 256L + tempBuf[ 1 ] * 65536L + tempBuf[ 2 ] * 256L + tempBuf[ 3 ];
    printf( "  %d Bytes per sector, capacity is %ld MBytes.\n",
            sectorSize, lbaMax * (sectorSize/256) / 1024L / (1024L/256) );
    printf( "------------------------+-------------+-------+-------+-------+-------+-------+\n" );
    printf( "        Test mode       :    result   |  Poor |   OK  |  Good | Great | Superb|\n" );


    if( CommandBench( hostNo, targetId, TESTTIME ) ){
        /* command error occured */
        return( -1 );
    }


    if( SeekBench( hostNo, targetId, lbaMax, TESTTIME ) ){
        /* seek error occured */
        return( -1 );
    }

    if( ( dataBuffer = (unsigned char far *)farmalloc( 65536L ) ) == NULL ){
        printf( "Error : Can't allocate buffer.\n" );
        return( -1 );
    }

    if( ReadBench( hostNo, targetId, dataBuffer, sectorSize, lbaMax, TESTTIME )  ){
        /* read error occured */
        farfree( dataBuffer );
        return( -1 );
    }
    printf( "------------------------+-------------+-------+-------+-------+-------+-------+\n" );

    farfree( dataBuffer );
    AspiClose();
    return( 0 );
}


//
//  get target device
//      parameter:
//          non
//      return :
//          -1  if error occured
//          else bit 7..4 : host adapter no.
//               bit 3..0 : scsi id
//
int GetTarget()
{
    int hostNo, host, targetId, targetType, selPtr;
    unsigned char inqBuf[ 15 ][ 15 ][ 40 ], buf[ 80 ];
    struct{
        int host;
        int id;
    }selNo[ 49 ];
    
    if( ( hostNo = HostInquiry( 0, buf ) ) == -1 ){
        return( -1 );
    }
    if( GetInquiry( hostNo, inqBuf ) ){
        return( -1 );
    }
    selPtr = 0;
    for( host = 0; host < hostNo; host++ ){
        for( targetId = 0; targetId < MAXID; targetId++ ){
            targetType = inqBuf[ host ][ targetId ][ 0 ] & 0x1f;
            if( targetType == 0 || targetType == 4 || targetType == 5 || targetType == 7 ){
                /* HDD or MO or CD-ROM */
                /* or CDRW (4) (new 0.05b) */
                selNo[ selPtr ].host = host;
                selNo[ selPtr ].id = targetId;
                printf( "Target No.%d : ", selPtr );
                DispInquiry( host, targetId, inqBuf[ host ][ targetId ] );
                selPtr++;
            }
        }
    }
    do{
        printf( "\nEnter target number ( 0 to %d ) : ", selPtr - 1 );
        gets( buf );
        targetId = atoi( buf );
        if( targetId == -1 ){
            return( -1 );
        }
    }while( ( targetId < 0 ) || ( targetId >= selPtr ) );
    host = selNo[ targetId ].host;
    targetId = selNo[ targetId ].id;

    HostInquiry( host, buf );
    printf( "\n\n  Host Adapter is " );

    for( selPtr = 16; selPtr < 32; selPtr++ ){
        if (buf[ selPtr ]=='\0')
            break;
        putchar( buf[ selPtr ] );
    }

    printf( "\n  Target device is " );
    DispInquiry( host, targetId, inqBuf[ host ][ targetId ] );
    
    return( host * 16 + targetId );
}


//
//  scsi command benchmark test
//      parameter :
//          int hostNo              : host adapter no.
//          int targetId            : target scsi id
//          int benchTime           : benchmark measurement time length
//
int CommandBench( hostNo, targetId, benchTime )
    int hostNo, targetId, benchTime;
{
    long commandCount;
    time_t stt;
    int benchMode, rankCount, i;
    static char *commandModeStr[] = {
        " Test Unit Ready command",
        " No Motion Seek command "
    };

    printf( "------------------------+-------------+-------+-------+-------+-------+-------+\n" );
    
    for( benchMode = 0; benchMode < 2; benchMode++ ){
        
        printf( "%s|", commandModeStr[ benchMode ] );
        
        commandCount = 0L;
        
        if( ScsiSeek( hostNo, targetId, 0L ) ){
            printf( "Seek error occured.\n" );\
            return( -1 );
        }
        stt = time( NULL );
        while( stt == time( NULL ) ){
            /* wait until time change */
        }
        stt += ( benchTime + 1 );
        while( time( NULL ) < stt ){
            if( benchMode == 0 ){
                for( i = 0; i < 100; i++ ){
                    if( TestUnitReady( hostNo, targetId ) ){
                        return( -1 );
                    }
                }
            }else{
                for( i = 0; i < 100; i++ ){
                    if( ScsiSeek( hostNo, targetId, 0L ) ){
                        printf( "Seek error occured.\n" );\
                        return( -1 );
                    }
                }
            }
            commandCount++;
        }
        commandCount = (long)benchTime * 100L / commandCount;
        printf( "  %3ld.%1ld[ms]  :",
                     commandCount / 10L, commandCount % 10L );
        rankCount = ( commandRank[ benchMode ] - commandCount ) * 40 / commandRank[ benchMode ] + 1;
        for( i = 0; i < rankCount; i++ ){
            putchar( '*' );
        }
        putchar( '\n' );
    }
    return( 0 );
}


//
//  seek benchmark test
//      parameter :
//          int hostNo              : host adapter no.
//          int targetId            : target scsi id
//          unsigned long lbaMax    : maximum block number of target device
//          int benchTime           : benchmark measurement time length
//
int SeekBench( hostNo, targetId, lbaMax, benchTime )
    int hostNo, targetId, benchTime;
    unsigned long lbaMax;
{
    long logicalAddr, randomScale, seekCount;
    time_t stt;
    int benchMode, rankCount, i;
    static char *seekModeStr[] = {
        " Sequential Seek command",
        " Random Seek command    "
    };
    

    randomScale = lbaMax / 32767L;
    if( randomScale == 0 ){
        randomScale = 1;
    }
    
    printf( "------------------------+-------------+-------+-------+-------+-------+-------+\n" );
    for( benchMode = 0; benchMode < 2; benchMode++ ){
        
        printf( "%s|", seekModeStr[ benchMode ] );
        
        logicalAddr = 0L;
        seekCount = 0L;
        
        stt = time( NULL );
        while( stt == time( NULL ) ){
            /* wait until time change */
        }
        stt += ( benchTime + 1 );
        while( time( NULL ) < stt ){
            if( ScsiSeek( hostNo, targetId, logicalAddr ) ){
                printf( "Seek error occured.\n" );\
                return( -1 );
            }
            if( benchMode == 0 ){
                logicalAddr += 32L;
                if( logicalAddr > (long)lbaMax ){
                    logicalAddr = 0L;
                }
            }else{
                do{
                    logicalAddr = random( 32768 ) * randomScale;
                }while( logicalAddr > (long)lbaMax );
            }
            seekCount++;
        }
        seekCount = (long)benchTime * 10000L / seekCount;
        printf( "  %3ld.%1ld[ms]  :",
                     seekCount / 10L, seekCount % 10L );
        rankCount = ( seekRank[ benchMode ] - seekCount ) * 40 / seekRank[ benchMode ] + 1;
        for( i = 0; i < rankCount; i++ ){
            putchar( '*' );
        }
        putchar( '\n' );
    }
    return( 0 );
}


//
//  data read benchmark test
//      parameter :
//          int hostNo                      : host adapter no.
//          int targetId                    : target scsi id
//          unsigned char far *dataBuffer   : data buffer ptr. (65536 bytes)
//          int sectorSize                  : logical block size of target
//          unsigned long lbaMax            : maximum block number of target
//          int benchTime                   : benchmark measurement time length
//
int ReadBench( hostNo, targetId, dataBuffer, sectorSize, lbaMax, benchTime )
    int hostNo, targetId, sectorSize, benchTime;
    unsigned char far *dataBuffer;
    unsigned long lbaMax;
{
    long logicalAddr, randomScale, readCount;
    int benchMode, blockCount, maxBlockCount, rankCount, i;
    time_t stt;
    static char *readModeStr[] = {
        " Seq. Read",
        " Rnd. Read",
        " SeqRd End"
    };
    
    randomScale = ( lbaMax - 65536L / sectorSize ) / 32767L;
    if( randomScale == 0 ){
        randomScale = 1;
    }
    maxBlockCount = 65536L / sectorSize;


    for( benchMode = 0; benchMode < 3; benchMode++ ){
        printf( "------------------------+-------------+-------+-------+-------+-------+-------+\n" );

        for( blockCount = 1; ; ){
            logicalAddr = 0L;
            if (benchMode==2)
                            if (((65536L*1024*4) / sectorSize) < (long)lbaMax)
                    logicalAddr = lbaMax  - ((65536L*1024*4) / sectorSize) ;
            readCount = 0L;

            printf( "%s %6ldB/read |",
                readModeStr[ benchMode ], (long)blockCount * sectorSize );
            stt = time( NULL );
            while( stt == time( NULL ) ){
                /* wait until time change */
            }
            stt += ( benchTime + 1 );
            while( time( NULL ) < stt ){
                if( ScsiRead( hostNo, targetId, logicalAddr, sectorSize, blockCount, dataBuffer ) ){
                    printf( "Read error occured.\n" );
                    return( -1 );
                }
                if((benchMode  == 0 )||( benchMode  == 2)){
                    logicalAddr += blockCount;
                    if (logicalAddr >= lbaMax)
                        logicalAddr = lbaMax  - ((65536L*1024*4*4) / sectorSize) ;
                }else{
                    do{
                        logicalAddr = random( 32768 ) * randomScale;
                    }while( logicalAddr > (long)lbaMax );
                }
                readCount += blockCount;
            }
            readCount = readCount * ((10L * sectorSize)/256) / benchTime / (1024L/256);
            printf( "%5ld.%1ld[KB/s]:",
                 readCount / 10L, readCount % 10L );
            rankCount = readCount * 40 / readRank[ benchMode ] + 1;
            if( rankCount > 40 ){
                rankCount = 40;
            }
            for( i = 0; i < rankCount; i++ ){
                putchar( '*' );
            }
            putchar( '\n' );
            if( blockCount == 1 ){
                blockCount = 16384L / sectorSize;
            }else if( blockCount != maxBlockCount ){
                blockCount = maxBlockCount;
            }else{
                break;
            }
        }
    }
    return( 0 );
}


//
//  get inquiry data from all host adapters, all scsi targets
//      parameter :
//          int hostNo : number of host adapters
//          unsigned char inqbuf[15][15][40] : data buffer for inquiry
//      return :
//           0  if no error
//          -1  if error occured
//
int GetInquiry( hostNo, inqBuf )
    int hostNo;
    unsigned char inqBuf[][ 15 ][ 40 ];
{
    int host, targetId;
    
    for( host = 0; host < hostNo; host++ ){
        for( targetId = 0; targetId < MAXID; targetId++ ){
            inqBuf[ host ][ targetId ][ 0 ] = 0xff;
            if( Inquiry( host, targetId, inqBuf[ host ][ targetId ] ) == -1 ){
                printf( "Host adapter error occured.\n" );
                return( -1 );
            }
        }
    }
    return( 0 );
}


//
//  display inquiry data
//      parameter :
//          int hostno : host adapter no.
//          int targetid : target scsi is
//          unsigned char *inqdata : inquiry data
//      return :
//          non
//
void DispInquiry( hostNo, targetId, inqData )
    int hostNo, targetId;
    unsigned char *inqData;
{
    int i;
    
    if( inqData[ 0 ] == 0xff ){
        return;
    }
    printf( "HA#%d ID%d ", hostNo, targetId );
    if( inqData[ 4 ] ){
        putchar( ' ' );
        for( i = 8; i < 36; i++ ){
            if( inqData[ i ] == 0x00 ){
                break;
            }else{
                putchar( inqData[ i ] );
            }
        }
        putchar( ' ' );
    }else{
        printf( " Device name not available" );
    }
    printf( " CCS%d ",inqData[ 2 ] & 0x03 );
    if( inqData[ 1 ] & 0x80 ){
        printf( "Removable\n" );
    }else{
        printf( "Rigid\n" );
    }
}
