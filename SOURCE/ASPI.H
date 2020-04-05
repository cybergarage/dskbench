//
//	headder file for ASPI handler
//		copyright(c) by Tsuru-Zoh, Oct.30,1992
//

#ifndef WIN32
#define SENSE_LENGTH 14

typedef struct {
	unsigned char commandCode;
	unsigned char commandStatus;
	unsigned char hostAdapterNo;
	unsigned char scsiRequestFlag;
	unsigned char reserved[ 4 ];
	unsigned char noOfAdapter;
	unsigned char idOfAdapter;
	unsigned char managerId[ 16 ];
	unsigned char adapterId[ 16 ];
	unsigned char adapterPrm[ 16 ];
} srbInquiry;


typedef struct {
	unsigned char commandCode;
	unsigned char commandStatus;
	unsigned char hostAdapterNo;
	unsigned char scsiRequestFlag;
	unsigned char reserved1[ 4 ];
	unsigned char targetId;
	unsigned char targetLun;
	unsigned long dataAllocLength;
	unsigned char senseAllocLength;
	unsigned int dataBufferOffset;
	unsigned int dataBufferSegment;
	unsigned int srbLinkOffest;
	unsigned int srbLinkSegment;
	unsigned char scsiCdbLength;
	unsigned char hostAdapterStatus;
	unsigned char targetStatus;
	unsigned int postRoutineOffset;
	unsigned int postRoutineSegment;
	unsigned char reserved2[ 34 ];
	unsigned char cdb[ 6 ];
	unsigned char sense[ SENSE_LENGTH ];
} srbCdb6;


typedef struct {
	unsigned char commandCode;
	unsigned char commandStatus;
	unsigned char hostAdapterNo;
	unsigned char scsiRequestFlag;
	unsigned char reserved1[ 4 ];
	unsigned char targetId;
	unsigned char targetLun;
	unsigned long dataAllocLength;
	unsigned char senseAllocLength;
	unsigned int dataBufferOffset;
	unsigned int dataBufferSegment;
	unsigned int srbLinkOffest;
	unsigned int srbLinkSegment;
	unsigned char scsiCdbLength;
	unsigned char hostAdapterStatus;
	unsigned char targetStatus;
	unsigned int postRoutineOffset;
	unsigned int postRoutineSegment;
	unsigned char reserved2[ 34 ];
	unsigned char cdb[ 10 ];
	unsigned char sense[ SENSE_LENGTH ];
} srbCdb10;

#endif


int AspiInit( void );
void AspiClose( void );
int	HostInquiry( int, unsigned char * );
int TestUnitReady( int, int );
int Inquiry( int, int, unsigned char * );
int ReadCapacity( int, int, unsigned char * );
int ScsiSeek( int, int, unsigned long );
int ScsiRead( int, int, unsigned long, int, int, unsigned char far * );
void SrbInit( unsigned char *, int );

