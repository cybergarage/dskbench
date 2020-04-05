//
//	SCSI handling routine for ASPI
//  	copyright(c) by Tsuru-Zoh, Oct.30,1992
//
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include "aspi.h"


void ( _far *Aspi )( unsigned char _far * ) = ( void _far * )0;


//
//	Initialize ASPI routine
//	open "ASPIMGR$", get address of function Aspi(), close "ASPIMGR$"
//		param.	: non
//		return	: 0 if succeed
//				 -1 if failured
//
int AspiInit()
{
	int handle;
	union REGS rg;
	
	if( ( handle = open( "SCSIMGR$", O_RDONLY ) ) == -1 ){
		printf( "Error : ASPI SCSI manager not found.\n" );
		printf( " Please add ASPI SCSI manager to your config.sys.\n" );
		return( -1 );
	}
	rg.x.ax = 0x4402;		/* IOCTL in */
	rg.x.bx = handle;
	rg.x.cx = 4;
	rg.x.dx = (unsigned int)&Aspi;
	intdos( &rg, &rg );
	
	close( handle );
	
	return( 0 );
}

void AspiClose( void )
{
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
	struct SREGS sr;
	srbInquiry srb;
	
	segread( &sr );
	SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
	srb.commandCode = 0x00;
	srb.hostAdapterNo = hostNo;
	srb.scsiRequestFlag = 0x00;
	Aspi( (unsigned char _far *)&srb );
	while( !srb.commandStatus ){
		/* wait until command complete */
	}
	if( srb.commandStatus == 0x81 ){
		printf( "ASPI Error at Host adapter inquiry : Invalid host adapter number.\n" );
		return( -1 );
	}
	memcpy( buf, &srb.managerId, 48 );
	return( srb.noOfAdapter );
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
	srbCdb6 srb;
	
	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.commandCode = 0x02;
		srb.hostAdapterNo = hostNo;
		srb.scsiRequestFlag = 0x00;
		srb.targetId = targetId;
		srb.senseAllocLength = SENSE_LENGTH;
		srb.scsiCdbLength = 6;
		srb.cdb[ 0 ] = 0x00;
		srb.cdb[ 1 ] = 0x00;
		srb.cdb[ 2 ] = 0x00;
		srb.cdb[ 3 ] = 0x00;
		srb.cdb[ 4 ] = 0x00;
		srb.cdb[ 5 ] = 0x00;
	
		Aspi( (unsigned char _far *)&srb );
		while( !srb.commandStatus ){
			/* wait until command complete */
		}
		switch( srb.hostAdapterStatus ){
		case 0x00:
			break;
		default:
			printf( "ASPI Error : Host adapter error %02x.\n", srb.hostAdapterStatus );
			return( -1 );
		}
		switch( srb.commandStatus ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		}
		switch( srb.targetStatus ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.sense[ 12 ] == 0x28 ) || (srb.sense[ 12 ] == 0x29 ) ){
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
	struct SREGS sr;
	srbCdb6 srb;
	
	segread( &sr );
	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.commandCode = 0x02;
		srb.hostAdapterNo = hostNo;
		srb.scsiRequestFlag = 0x00;
		srb.targetId = targetId;
		srb.dataAllocLength = 40;
		srb.senseAllocLength = SENSE_LENGTH;
		srb.dataBufferOffset = (unsigned)inqData;
		srb.dataBufferSegment = sr.ds;
		srb.scsiCdbLength = 6;
		srb.cdb[ 0 ] = 0x12;
		srb.cdb[ 1 ] = 0x00;
		srb.cdb[ 2 ] = 0x00;
		srb.cdb[ 3 ] = 0x00;
		srb.cdb[ 4 ] = 40;
		srb.cdb[ 5 ] = 0x00;
	
		Aspi( (unsigned char _far *)&srb );
		while( !srb.commandStatus ){
			/* wait until command complete */
		}
		switch( srb.hostAdapterStatus ){
		case 0x00:
			break;
		case 0x11:			/* selection timeout */
			inqData[ 0 ] = 0xff;
			return( 0 );
		default:
			printf( "ASPI Error : Host adapter error %02x.\n", srb.hostAdapterStatus );
			return( -1 );
		}
		switch( srb.commandStatus ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		case 0x82:			/* SCSI device not installed */
			inqData[ 0 ] = 0xff;
			return( 0 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		}
		switch( srb.targetStatus ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.sense[ 12 ] == 0x28 ) || (srb.sense[ 12 ] == 0x29 ) ){
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
	struct SREGS sr;
	srbCdb10 srb;

	segread( &sr );
	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.commandCode = 0x02;
		srb.hostAdapterNo = hostNo;
		srb.scsiRequestFlag = 0x00;
		srb.targetId = targetId;
		srb.dataAllocLength = 8;
		srb.senseAllocLength = SENSE_LENGTH;
		srb.dataBufferOffset = (unsigned)capBuf;
		srb.dataBufferSegment = sr.ds;
		srb.scsiCdbLength = 10;
		srb.cdb[ 0 ] = 0x25;
		srb.cdb[ 1 ] = 0x00;
		srb.cdb[ 2 ] = 0x00;
		srb.cdb[ 3 ] = 0x00;
		srb.cdb[ 4 ] = 0x00;
		srb.cdb[ 5 ] = 0x00;
		srb.cdb[ 6 ] = 0x00;
		srb.cdb[ 7 ] = 0x00;
		srb.cdb[ 8 ] = 0x00;
		srb.cdb[ 9 ] = 0x00;
	
		Aspi( (unsigned char _far *)&srb );
		while( !srb.commandStatus ){
			/* wait until command complete */
		}
		if( srb.hostAdapterStatus ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.hostAdapterStatus );
			return( -1 );
		}
		switch( srb.commandStatus ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		}
		switch( srb.targetStatus ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.sense[ 12 ] == 0x28 ) || (srb.sense[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.sense[ 2 ], srb.sense[ 12 ] );
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
	srbCdb6 srb;

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.commandCode = 0x02;
		srb.hostAdapterNo = hostNo;
		srb.scsiRequestFlag = 0x00;	/* no data transfer */
		srb.targetId = targetId;
		srb.senseAllocLength = SENSE_LENGTH;
		srb.scsiCdbLength = 6;
		srb.cdb[ 0 ] = 0x0b;
		srb.cdb[ 1 ] = ( logicalAddr >> 16 ) & 0x00ff;
		srb.cdb[ 2 ] = ( logicalAddr >> 8 ) & 0x00ff;
		srb.cdb[ 3 ] = logicalAddr & 0x00ff;
		srb.cdb[ 4 ] = 0x00;
		srb.cdb[ 5 ] = 0x00;
	
		Aspi( (unsigned char far *)&srb );
		while( !srb.commandStatus ){
			/* wait until command complete */
		}
		if( srb.hostAdapterStatus ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.hostAdapterStatus );
			return( -1 );
		}
		switch( srb.commandStatus ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		}
		switch( srb.targetStatus ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.sense[ 12 ] == 0x28 ) || (srb.sense[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.sense[ 2 ], srb.sense[ 12 ] );
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
	srbCdb10 srb;

	for( ; ; ){
		SrbInit( (unsigned char *)&srb, sizeof( srb ) );
		
		srb.commandCode = 0x02;
		srb.hostAdapterNo = hostNo;
		srb.scsiRequestFlag = 0x08;	/* target to host */
		srb.targetId = targetId;
		srb.dataAllocLength = (long)blockSize * blockCount;
		srb.senseAllocLength = SENSE_LENGTH;
		srb.dataBufferOffset = FP_OFF( dataBuffer );
		srb.dataBufferSegment = FP_SEG( dataBuffer );
		srb.scsiCdbLength = 10;
		srb.cdb[ 0 ] = 0x28;
		srb.cdb[ 1 ] = 0x00;
		srb.cdb[ 2 ] = ( logicalAddr >> 24 ) & 0x00ff;
		srb.cdb[ 3 ] = ( logicalAddr >> 16 ) & 0x00ff;
		srb.cdb[ 4 ] = ( logicalAddr >> 8 ) & 0x00ff;
		srb.cdb[ 5 ] = logicalAddr & 0x00ff;
		srb.cdb[ 6 ] = 0x00;
		srb.cdb[ 7 ] = ( blockCount >> 8 ) & 0x00ff;
		srb.cdb[ 8 ] = blockCount & 0x00ff;
		srb.cdb[ 9 ] = 0x00;
	
		Aspi( (unsigned char _far *)&srb );
		while( !srb.commandStatus ){
			/* wait until command complete */
		}
		if( srb.hostAdapterStatus ){
			printf( "ASPI Error : Host adapter error %02x.\n", srb.hostAdapterStatus );
			return( -1 );
		}
		switch( srb.commandStatus ){
		case 0x01:			/* completed without error */
			return( 0 );
		case 0x04:			/* completed with error */
			break;
		case 0x80:			/* invalid scsi request */
		case 0x81:			/* invalid host adapter no. */
		case 0x82:			/* SCSI device not installed */
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		default:
			printf( "ASPI Error : Command error status = %02x.\n", srb.commandStatus );
			return( -1 );
		}
		switch( srb.targetStatus ){
		case 0x00:
			return( 0 );
		case 0x02:
			if( ( srb.sense[ 12 ] == 0x28 ) || (srb.sense[ 12 ] == 0x29 ) ){
				/* reset condition */
				continue;
			}else{
				printf( "SCSI Error : ID=%d sense key = %02x , sense code = %02x\n",
					targetId, srb.sense[ 2 ], srb.sense[ 12 ] );
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

