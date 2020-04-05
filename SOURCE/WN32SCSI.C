//
//	SCSI handling routine for ASPI
//  	copyright(c) by Tsuru-Zoh, Oct.30,1992

#include <windows.h>
//
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>

#include "scsidefs.h"
#include "wnaspi32.h"

#include "aspi.h"




static HANDLE hEventEnd = NULL;


/*
void APIENTRY  ASPIPostProc (PSRB_ExecSCSICmd DoneSRB )
   {

   //
   // Send POST message
   //
   if (hEventEnd!=NULL)
       SetEvent(hEventEnd);

   return;
   }

void GetMakePostProc(PSRB_ExecSCSICmd DoneSRB )
{
    if (hEventEnd!=NULL)
        printf("error");
    hEventEnd = CreateEvent(NULL,FALSE,FALSE, NULL);

    //DoneSRB->SRB_Flags |=SRB_POSTING;
    //DoneSRB->SRB_PostProc=(void*)ASPIPostProc;

    DoneSRB->SRB_Flags |=SRB_EVENT_NOTIFY;
    DoneSRB->SRB_PostProc=hEventEnd;
}

void WaitEndAspi()
{
   if (hEventEnd!=NULL)
   {
       WaitForSingleObject(hEventEnd,INFINITE);
       CloseHandle(hEventEnd);
       hEventEnd=NULL;
   }
}
*/
//


void /*APIENTRY*/  ASPIPostProc (PSRB_ExecSCSICmd DoneSRB )
   {

   //
   // Send POST message
   //
   if (hEventEnd!=NULL)
       SetEvent(hEventEnd);

   return;
   }

void GetMakePostProc(PSRB_ExecSCSICmd DoneSRB )
{
    DoneSRB->SRB_Flags |=SRB_POSTING;
    DoneSRB->SRB_PostProc=(void*)ASPIPostProc;
}

void WaitEndAspi()
{
   if (hEventEnd!=NULL)
   {
       WaitForSingleObject(hEventEnd,INFINITE);
   }
}

int AspiInit()
{
    BYTE  AdapterCount;
    DWORD ASPI32Status;

    ASPI32Status 			= GetASPI32SupportInfo();
    AdapterCount 			= (LOBYTE(LOWORD(ASPI32Status)));

    hEventEnd = CreateEvent(NULL,FALSE,FALSE, NULL);

    if ((AdapterCount != 0) && (HIBYTE(LOWORD(ASPI32Status)) == SS_COMP))
        return 0;
    else

	printf( "Error : ASPI SCSI manager not found.\n" );
	printf( " Please add ASPI SCSI manager to your config.sys.\n" );
	return( -1 );
}

void AspiClose( void )
{
       if (hEventEnd!=NULL)
            CloseHandle(hEventEnd);
       hEventEnd=NULL;
}

//
//	Get Host Adapter Inquiry
//		param.	:
//			int hostNo			: host adapter no.
//			unsigned char *buf	: buffer for host inquiry data ( 24 bytes )
//		return	:
//			returns number of host adapters if succeed
//				inquiry data stored to buf
//			-1 if error occured
//
int HostInquiry( hostNo, buf )
	int hostNo;
	unsigned char *buf;
{
	SRB_HAInquiry srb;
	

	SrbInit( (unsigned char *)&srb, sizeof( srb ) );
    SrbInit( (unsigned char *)buf, 48 );
		
	srb.SRB_Cmd = 0x00;
	srb.SRB_HaId = hostNo;
	srb.SRB_Flags = 0x00;

    SendASPI32Command(&srb);
	while( !srb.SRB_Status ){
		/* wait until command complete */
	}
	if( srb.SRB_Status == 0x81 ){
		printf( "ASPI Error at Host adapter inquiry : Invalid host adapter number.\n" );
		return( -1 );
	}
	memcpy( buf, &srb.HA_ManagerId, 48 );
	return( srb.HA_Count );
}


//
//	Test Unit Ready command
//		param.	:
//			int hostNo		: host adapter no.
//			int targetId	: target SCSI ID
//		return	:
//			0 if no error
//			8 if target is busy
//			-1 if error occured
//
int TestUnitReady( hostNo, targetId )
	int hostNo, targetId;
{
	SRB_ExecSCSICmd srb;
	
	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.SRB_Cmd = 0x02;
		srb.SRB_HaId = hostNo;
		srb.SRB_Flags = 0x00;
		srb.SRB_Target = targetId;
		srb.SRB_SenseLen = SENSE_LEN;
		srb.SRB_CDBLen = 6;
		srb.CDBByte[ 0 ] = 0x00;
		srb.CDBByte[ 1 ] = 0x00;
		srb.CDBByte[ 2 ] = 0x00;
		srb.CDBByte[ 3 ] = 0x00;
		srb.CDBByte[ 4 ] = 0x00;
		srb.CDBByte[ 5 ] = 0x00;
	
        GetMakePostProc(&srb);
		SendASPI32Command(&srb);
        WaitEndAspi();


		while( !srb.SRB_Status ){
			/* wait until command complete */
		}
		switch( srb.SRB_HaStat ){
		case 0x00:
			break;
		default:
			printf( "ASPI Error : Host adapter error %02x.\n", srb.SRB_HaStat );
			return( -1 );
		}
		switch( srb.SRB_Status ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		}
		switch( srb.SRB_TargStat ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.SenseArea[ 12 ] == 0x28 ) || (srb.SenseArea[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				return( -1 );
			}
		case 0x08:
			/* target busy */
			return( 8 );
		default:
			return( -1 );
		}
	}
}


//
//	Inquiry command
//		param.	:
//			int hostNo				: host adapter no.
//			int targetId			: target SCSI ID
//			unsigned char *inqData	: data buffer for inquiry data ( 36 bytes )
//		return	:
//			 0 if no error, inquiry data sotred to inqData
//			-1 if error occured
//
int Inquiry( hostNo, targetId, inqData )
	int hostNo, targetId;
	unsigned char *inqData;
{

	SRB_ExecSCSICmd srb;
	
    SrbInit( inqData, 40 );

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.SRB_Cmd = 0x02;
		srb.SRB_HaId = hostNo;
		srb.SRB_Flags = SRB_DIR_SCSI;//0x00;
		srb.SRB_Target = targetId;
		srb.SRB_BufLen = 40-8;
		srb.SRB_SenseLen = SENSE_LEN;
		srb.SRB_BufPointer = inqData;
		//srb.dataBufferSegment = sr.ds;
		srb.SRB_CDBLen = 6;
		srb.CDBByte[ 0 ] = 0x12;
		srb.CDBByte[ 1 ] = 0x00;
		srb.CDBByte[ 2 ] = 0x00;
		srb.CDBByte[ 3 ] = 0x00;
		srb.CDBByte[ 4 ] = 40-8;
		srb.CDBByte[ 5 ] = 0x00;

        
        
        GetMakePostProc(&srb);
		SendASPI32Command(&srb);
        WaitEndAspi();



		while( !srb.SRB_Status ){
			/* wait until command complete */
		}
		switch( srb.SRB_HaStat ){
		case 0x00:
			break;
		case 0x11:			/* selection timeout */
			inqData[ 0 ] = 0xff;
			return( 0 );
		default:
			printf( "ASPI Error : Host adapter error %02x.\n", srb.SRB_HaStat );
			return( -1 );
		}
		switch( srb.SRB_Status ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		case 0x82:			/* SCSI device not installed */
			inqData[ 0 ] = 0xff;
			return( 0 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		}
		switch( srb.SRB_TargStat ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.SenseArea[ 12 ] == 0x28 ) || (srb.SenseArea[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				inqData[ 0 ] = 0xff;
				return( 0 );
			}
		default:
			return( -1 );
		}
	}
}


//
//	Read Capacity command
//		param.	:
//			int hostNo				: host adapter no.
//			int targetId			: target SCSI ID
//			unsigned char *capBuf	: buffer for capacity data ( 8 bytes )
//		return	:
//			 0 if no error, capacity data sotred to *capBuf
//			-1 if error occured
//
int ReadCapacity( hostNo, targetId, capBuf )
	int hostNo, targetId;
	unsigned char *capBuf;
{
	SRB_ExecSCSICmd srb;

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.SRB_Cmd = 0x02;
		srb.SRB_HaId = hostNo;
		srb.SRB_Flags = 0x00;
		srb.SRB_Target = targetId;
		srb.SRB_BufLen = 8;
		srb.SRB_SenseLen = SENSE_LEN;
		srb.SRB_BufPointer = capBuf;
		//srb.dataBufferSegment = sr.ds;
		srb.SRB_CDBLen = 10;
		srb.CDBByte[ 0 ] = 0x25;
		srb.CDBByte[ 1 ] = 0x00;
		srb.CDBByte[ 2 ] = 0x00;
		srb.CDBByte[ 3 ] = 0x00;
		srb.CDBByte[ 4 ] = 0x00;
		srb.CDBByte[ 5 ] = 0x00;
		srb.CDBByte[ 6 ] = 0x00;
		srb.CDBByte[ 7 ] = 0x00;
		srb.CDBByte[ 8 ] = 0x00;
		srb.CDBByte[ 9 ] = 0x00;
	
        GetMakePostProc(&srb);
		SendASPI32Command(&srb);
        WaitEndAspi();


		while( !srb.SRB_Status ){
			/* wait until command complete */
		}
		if( srb.SRB_HaStat ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.SRB_HaStat );
			return( -1 );
		}
		switch( srb.SRB_Status ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		}
		switch( srb.SRB_TargStat ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.SenseArea[ 12 ] == 0x28 ) || (srb.SenseArea[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.SenseArea[ 2 ], srb.SenseArea[ 12 ] );
				return( -1 );
			}
		default:
			return( -1 );
		}
	}
}


//
//	Seek command
//		param.	:
//			int hostNo					: host adapter no.
//			int targetId				: target SCSI ID
//			unsigned long logicalAddr	: logical address to seek
//		return	:
//			 0 if no error
//			-1 if error occured
//
int ScsiSeek( hostNo, targetId, logicalAddr )
	int hostNo, targetId;
	unsigned long logicalAddr;
{
	SRB_ExecSCSICmd srb;

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.SRB_Cmd = 0x02;
		srb.SRB_HaId = hostNo;
		srb.SRB_Flags = 0x00;	/* no data transfer */
		srb.SRB_Target = targetId;
		srb.SRB_SenseLen = SENSE_LEN;
		srb.SRB_CDBLen = 6;
		srb.CDBByte[ 0 ] = (BYTE)0x0b;
		srb.CDBByte[ 1 ] = (BYTE)(( logicalAddr >> 16 ) & 0x00ff);
		srb.CDBByte[ 2 ] = (BYTE)(( logicalAddr >> 8 ) & 0x00ff);
		srb.CDBByte[ 3 ] = (BYTE)(logicalAddr & 0x00ff);
		srb.CDBByte[ 4 ] = 0x00;
		srb.CDBByte[ 5 ] = 0x00;
	
        
        GetMakePostProc(&srb);
		SendASPI32Command(&srb);
        WaitEndAspi();



		while( !srb.SRB_Status ){
			/* wait until command complete */
		}
		if( srb.SRB_HaStat ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.SRB_HaStat );
			return( -1 );
		}
		switch( srb.SRB_Status ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		}
		switch( srb.SRB_TargStat ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.SenseArea[ 12 ] == 0x28 ) || (srb.SenseArea[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.SenseArea[ 2 ], srb.SenseArea[ 12 ] );
				return( -1 );
			}
		default:
			return( -1 );
		}
	}
}


//
//	Read command
//		param.	:
//			int hostNo						: host adapter no.
//			int targetId					: target SCSI ID
//			unsigned long logicalAddr		: logical address to seek
//			int blockSize					: logical block size of target
//			int blockCount					: no. of block to read
//			unsigned char far *dataBuffer	: read data buffer (max.65536)
//		return	:
//			 0 if no error
//			-1 if error occured
//
int ScsiRead( hostNo, targetId, logicalAddr, blockSize, blockCount, dataBuffer )
	int hostNo, targetId;
	unsigned long logicalAddr;
	int blockSize, blockCount;
	unsigned char far *dataBuffer;
{
    
	SRB_ExecSCSICmd srb;

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.SRB_Cmd = 0x02;
		srb.SRB_HaId = hostNo;
		srb.SRB_Flags = 0x08;	/* target to host */
		srb.SRB_Target = targetId;
		srb.SRB_BufLen = (long)blockSize * blockCount;
		srb.SRB_SenseLen = SENSE_LEN;
		srb.SRB_BufPointer = ( dataBuffer );
		//srb.dataBufferSegment = FP_SEG( dataBuffer );
		srb.SRB_CDBLen = 10;
		srb.CDBByte[ 0 ] = 0x28;
		srb.CDBByte[ 1 ] = 0x00;
		srb.CDBByte[ 2 ] = (BYTE)(( logicalAddr >> 24 ) & 0x00ff);
		srb.CDBByte[ 3 ] = (BYTE)(( logicalAddr >> 16 ) & 0x00ff);
		srb.CDBByte[ 4 ] = (BYTE)(( logicalAddr >> 8 ) & 0x00ff);
		srb.CDBByte[ 5 ] = (BYTE)(logicalAddr & 0x00ff);
		srb.CDBByte[ 6 ] = 0x00;
		srb.CDBByte[ 7 ] = (BYTE)(( blockCount >> 8 ) & 0x00ff);
		srb.CDBByte[ 8 ] = (BYTE)(blockCount & 0x00ff);
		srb.CDBByte[ 9 ] = 0x00;

        
        GetMakePostProc(&srb);
		SendASPI32Command(&srb);
        WaitEndAspi();

		while( !srb.SRB_Status ){
			/* wait until command complete */
		}

		if( srb.SRB_HaStat ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.SRB_HaStat );
			return( -1 );
		}
		switch( srb.SRB_Status ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.SRB_Status );
			return( -1 );
		}
		switch( srb.SRB_TargStat ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.SenseArea[ 12 ] == 0x28 ) || (srb.SenseArea[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.SenseArea[ 2 ], srb.SenseArea[ 12 ] );
				return( -1 );
			}
		default:
			return( -1 );
		}
	}
}


//
//	clear SRB
//		param. :
//			unsigned char *srb	: area to fill with '0'
//			int count			: clear up size
//
void SrbInit( srb, count )
	unsigned char *srb;
	int count;
{
	int i;
	
	for( i = 0; i < count; i++ ){
		*srb++ = 0;
	}
}

