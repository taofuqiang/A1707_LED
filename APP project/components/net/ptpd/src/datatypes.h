#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <stdio.h> 

/*Struct defined in spec*/


/**
*\file
* \brief Main structures used in ptpdv2
*
* This header file defines structures defined by the spec,
* main program data structure, and all messages structures
 */


/**
* \brief The TimeInterval type represents time intervals
 */
typedef struct {
	Integer64 scaledNanoseconds;
} TimeInterval;

/**
 * \brief 5.3.3 The Timestamp type represents a positive time with respect to the epoch
 */

typedef struct
{
    UInteger48 secondsField;
    UInteger32 nanosecondsField;
} Timestamp;

/**
 * \brief 5.3.4 The ClockIdentity type identifies a clock
 */
typedef Octet ClockIdentity[CLOCK_IDENTITY_LENGTH];

/**
 * \brief 5.3.5 The PortIdentity identifies a PTP port.
 */

typedef struct
{
    ClockIdentity clockIdentity;
    UInteger16 portNumber;
} PortIdentity;

/**
 * \brief 5.3.6 The PortAdress type represents the protocol address of a PTP port
 */

typedef struct
{
    Enumeration16 networkProtocol;
    UInteger16 addressLength;
    Octet* addressField;
} PortAddress;


/**
* \brief 5.3.7 The ClockQuality represents the quality of a clock
 */

typedef struct
{
    UInteger8 clockClass;
    Enumeration8 clockAccuracy;
    UInteger16 offsetScaledLogVariance;
} ClockQuality;

/**
 * \brief 5.3.8 The TLV type represents TLV extension fields
 */

typedef struct
{
    Enumeration16 tlvType;
    UInteger16 lengthField;
    Octet* valueField;
} TLV;

/**
 * \brief 5.3.9 The PTPText data type is used to represent textual material in PTP messages
 * textField - UTF-8 encoding
 */

typedef struct
{
    UInteger8 lengthField;
    Octet* textField;
} PTPText;

/**
* \brief 5.3.10 The FaultRecord type is used to construct fault logs
 */

typedef struct
{
    UInteger16 faultRecordLength;
    Timestamp faultTime;
    Enumeration8 severityCode;
    PTPText faultName;
    PTPText faultValue;
    PTPText faultDescription;
} FaultRecord;

/**
* \brief The PhysicalAddress type is used to represent a physical address
 */
typedef struct {
	UInteger16 addressLength;
    Octet* addressField;
} PhysicalAddress;


/**
 * \brief The common header for all PTP messages (Table 18 of the spec)
 */
typedef struct
{
    NibbleUpper transportSpecific;
    Enumeration4Lower messageType;
    NibbleUpper reserved0;
    NibbleUpper versionPTP;
    UInteger4Lower messageLength;
    UInteger8 domainNumber;
    NibbleUpper reserved1;
    Octet flagField0;
    Octet flagField1;
    Integer64 correctionField;
    UInteger32 reserved2;
    PortIdentity sourcePortIdentity;
    UInteger16 sequenceId;
    UInteger8 controlField;
    Integer8 logMessageInterval;
} MsgHeader;

/**
 * \brief Announce message fields (Table 25 of the spec)
 */

typedef struct
{
    Timestamp originTimestamp;
    Integer16 currentUtcOffset;
    UInteger8 grandmasterPriority1;
    ClockQuality grandmasterClockQuality;
    UInteger8 grandmasterPriority2;
    ClockIdentity grandmasterIdentity;
    UInteger16 stepsRemoved;
    Enumeration8 timeSource;
}MsgAnnounce;


/**
 * \brief Sync message fields (Table 26 of the spec)
 */

typedef struct
{
    Timestamp originTimestamp;
}MsgSync;

/**
 * \brief DelayReq message fields (Table 26 of the spec)
 */

typedef struct
{
    Timestamp originTimestamp;
}MsgDelayReq;

/**
 * \brief DelayResp message fields (Table 30 of the spec)
 */

typedef struct
{
    Timestamp receiveTimestamp;
    PortIdentity requestingPortIdentity;
}MsgDelayResp;

/**
 * \brief FollowUp message fields (Table 27 of the spec)
 */

typedef struct
{
    Timestamp preciseOriginTimestamp;
}MsgFollowUp;

/**
 * \brief PDelayReq message fields (Table 29 of the spec)
 */

typedef struct
{
    Timestamp originTimestamp;

}MsgPDelayReq;

/**
 * \brief PDelayResp message fields (Table 30 of the spec)
 */

typedef struct
{
    Timestamp requestReceiptTimestamp;
    PortIdentity requestingPortIdentity;
}MsgPDelayResp;

/**
 * \brief PDelayRespFollowUp message fields (Table 31 of the spec)
 */

typedef struct
{
    Timestamp responseOriginTimestamp;
    PortIdentity requestingPortIdentity;
}MsgPDelayRespFollowUp;

/**
* \brief Signaling message fields (Table 33 of the spec)
 */

typedef struct
{
    PortIdentity targetPortIdentity;
    char* tlv;
}MsgSignaling;


/**
 * \brief Management TLV message fields
 */
/* Management TLV Message */
typedef struct {
	Enumeration16 tlvType;
    UInteger16 lengthField;
    Enumeration16 managementId;
	Octet* dataField;
} ManagementTLV;

/**
 * \brief Management TLV Clock Description fields (Table 41 of the spec)
 */
/* Management TLV Clock Description Message */
typedef struct {
	Octet clockType0;
    Octet clockType1;
    PTPText physicalLayerProtocol;
    PhysicalAddress physicalAddress;
    PortAddress  protocolAddress;
    Octet manufacturerIdentity0;
    Octet manufacturerIdentity1;
    Octet manufacturerIdentity2;
    Octet reserved;
    PTPText productDescription;
    PTPText revisionData;
    PTPText userDescription;
    Octet profileIdentity0;
    Octet profileIdentity1;
    Octet profileIdentity2;
    Octet profileIdentity3;
    Octet profileIdentity4;
    Octet profileIdentity5;
} MMClockDescription;

/**
 * \brief Management TLV User Description fields (Table 43 of the spec)
 */
/* Management TLV User Description Message */
typedef struct {
	PTPText userDescription;
} MMUserDescription;

/**
 * \brief Management TLV Initialize fields (Table 44 of the spec)
 */
/* Management TLV Initialize Message */
typedef struct {
	UInteger16 initializeKey;
} MMInitialize;

/**
 * \brief Management TLV Default Data Set fields (Table 50 of the spec)
 */
/* Management TLV Default Data Set Message */
typedef struct {
	Octet so_tsc;
    Octet reserved0;
    UInteger16 numberPorts;
    UInteger8 priority1;
    ClockQuality clockQuality;
    UInteger8 priority2;
    ClockIdentity clockIdentity;
    UInteger8 domainNumber;
    Octet reserved1;
} MMDefaultDataSet;

/**
 * \brief Management TLV Current Data Set fields (Table 55 of the spec)
 */
/* Management TLV Current Data Set Message */
typedef struct {
	UInteger16 stepsRemoved;
    TimeInterval offsetFromMaster;
    TimeInterval meanPathDelay;
} MMCurrentDataSet;

/**
 * \brief Management TLV Parent Data Set fields (Table 56 of the spec)
 */
/* Management TLV Parent Data Set Message */
typedef struct {
    PortIdentity parentPortIdentity;
    Boolean PS;
    Octet reserved;
    UInteger16 observedParentOffsetScaledLogVariance;
    Integer32 observedParentClockPhaseChangeRate;
    UInteger8 grandmasterPriority1;
    ClockQuality grandmasterClockQuality;
    UInteger8 grandmasterPriority2;
    ClockIdentity grandmasterIdentity;
} MMParentDataSet;

/**
 * \brief Management TLV Time Properties Data Set fields (Table 57 of the spec)
 */
/* Management TLV Time Properties Data Set Message */
typedef struct {
    Integer16 currentUtcOffset;
    Octet ftra_ttra_ptp_utcv_li59_li61;
    Enumeration8 timeSource;
} MMTimePropertiesDataSet;

/**
 * \brief Management TLV Port Data Set fields (Table 61 of the spec)
 */
/* Management TLV Port Data Set Message */
typedef struct {
    PortIdentity portIdentity;
    Enumeration8 portState;
    Integer8 logMinDelayReqInterval;
    TimeInterval peerMeanPathDelay;
    Integer8 logAnnounceInterval;
    UInteger8 announceReceiptTimeout;
    Integer8 logSyncInterval;
    Enumeration8 delayMechanism;
    Integer8 logMinPdelayReqInterval;
    NibbleUpper reserved;
    UInteger4Lower versionNumber;
} MMPortDataSet;

/**
 * \brief Management TLV Priority1 fields (Table 51 of the spec)
 */
/* Management TLV Priority1 Message */
typedef struct {
    UInteger8 priority1;
    Octet reserved;
} MMPriority1;

/**
 * \brief Management TLV Priority2 fields (Table 52 of the spec)
 */
/* Management TLV Priority2 Message */
typedef struct {
    UInteger8 priority2;
    Octet reserved;
} MMPriority2;

/**
 * \brief Management TLV Domain fields (Table 53 of the spec)
 */
/* Management TLV Domain Message */
typedef struct {
    UInteger8 domainNumber;
    Octet reserved;
} MMDomain;

/**
 * \brief Management TLV Slave Only fields (Table 54 of the spec)
 */
/* Management TLV Slave Only Message */
typedef struct {
    Boolean so;
    Octet reserved;
} MMSlaveOnly;

/**
 * \brief Management TLV Log Announce Interval fields (Table 62 of the spec)
 */
/* Management TLV Log Announce Interval Message */
typedef struct {
    Integer8 logAnnounceInterval;
    Octet reserved;
} MMLogAnnounceInterval;

/**
 * \brief Management TLV Announce Receipt Timeout fields (Table 63 of the spec)
 */
/* Management TLV Announce Receipt Timeout Message */
typedef struct {
    UInteger8 announceReceiptTimeout;
    Octet reserved;
} MMAnnounceReceiptTimeout;

/**
 * \brief Management TLV Log Sync Interval fields (Table 64 of the spec)
 */
/* Management TLV Log Sync Interval Message */
typedef struct {
    Integer8 logSyncInterval;
    Octet reserved;
} MMLogSyncInterval;

/**
 * \brief Management TLV Version Number fields (Table 67 of the spec)
 */
/* Management TLV Version Number Message */
typedef struct {
    NibbleUpper reserved0;
    UInteger4Lower versionNumber;
    Octet reserved1;
} MMVersionNumber;

/**
 * \brief Management TLV Time fields (Table 48 of the spec)
 */
/* Management TLV Time Message */
typedef struct {
    Timestamp currentTime;
} MMTime;

/**
 * \brief Management TLV Clock Accuracy fields (Table 49 of the spec)
 */
/* Management TLV Clock Accuracy Message */
typedef struct {
    Enumeration8  clockAccuracy;
    Octet reserved;
} MMClockAccuracy;

/**
 * \brief Management TLV UTC Properties fields (Table 58 of the spec)
 */
/* Management TLV UTC Properties Message */
typedef struct {
    Integer16 currentUtcOffset;
    Octet utcv_li59_li61;
    Octet reserved;
} MMUtcProperties;

/**
 * \brief Management TLV Traceability Properties fields (Table 59 of the spec)
 */
/* Management TLV Traceability Properties Message */
typedef struct {
    Octet ftra_ttra;
    Octet reserved;
} MMTraceabilityProperties;

/**
 * \brief Management TLV Delay Mechanism fields (Table 65 of the spec)
 */
/* Management TLV Delay Mechanism Message */
typedef struct {
    Enumeration8 delayMechanism;
    Octet reserved;
} MMDelayMechanism;

/**
 * \brief Management TLV Log Min Pdelay Req Interval fields (Table 66 of the spec)
 */
/* Management TLV Log Min Pdelay Req Interval Message */
typedef struct {
    Integer8 logMinPdelayReqInterval;
    Octet reserved;
} MMLogMinPdelayReqInterval;

/**
 * \brief Management TLV Error Status fields (Table 71 of the spec)
 */
/* Management TLV Error Status Message */
typedef struct {
    Enumeration16 managementId;
    UInteger32 reserved;
    PTPText displayData;
} MMErrorStatus;

/**
* \brief Management message fields (Table 37 of the spec)
 */
/*management Message*/
typedef struct 

{
	MsgHeader header;
	PortIdentity targetPortIdentity;
  	UInteger8 startingBoundaryHops;
    UInteger8 boundaryHops;
    NibbleUpper reserved0;
	Enumeration4Lower actionField;
	Octet reserved1;
	ManagementTLV* tlv;
}MsgManagement;

/**
* \brief Time structure to handle Linux time information
 */

typedef struct
{
    Integer32 seconds;
    Integer32 nanoseconds;
} TimeInternal;

/**
* \brief Structure used as a timer
 */
typedef struct
{
	Integer32 interval;
	volatile Boolean expire;
} IntervalTimer;

/**
* \brief ForeignMasterRecord is used to manage foreign masters
 */
typedef struct
{
	PortIdentity foreignMasterPortIdentity;
	UInteger16 foreignMasterAnnounceMessages;

	//This one is not in the spec
	MsgAnnounce  announce;
	MsgHeader    header;
} ForeignMasterRecord;


/**
 * \struct PtpdStats
 * \brief Ptpd clock statistics per port
 */
typedef struct
{

    Integer32 ofmMean; /* mean offset from master - accuracy*/
    Integer32 ofmStdDev; /* offset from master standard deviation - precision */
    /* TODO: investigate MAD - Mean Absolute Deviation */
    Integer32 ofmMedian; /* offset from master median - effective accuracy*/
    /* TODO: investigate velocity/acceleration as statistical measures */
    Integer32 driftStdDev; /* observed drift standard deviation */

} PtpdStats;


/**
 * \struct PtpClock
 * \brief Main program data structure
 */
/* main program data structure */
typedef struct {
	/* Default data set */

	/*Static members*/
	Boolean twoStepFlag;
	ClockIdentity clockIdentity;
	UInteger16 numberPorts;

	/*Dynamic members*/
	ClockQuality clockQuality;

	/*Configurable members*/
	UInteger8 priority1;
	UInteger8 priority2;
	UInteger8 domainNumber;
	Boolean slaveOnly;


	/* Current data set */

	/*Dynamic members*/
	UInteger16 stepsRemoved;
	TimeInternal offsetFromMaster;
	TimeInternal meanPathDelay;


	/* Parent data set */

	/*Dynamic members*/
	PortIdentity parentPortIdentity;
	Boolean parentStats;
	UInteger16 observedParentOffsetScaledLogVariance;
	Integer32 observedParentClockPhaseChangeRate;
	ClockIdentity grandmasterIdentity;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority1;
	UInteger8 grandmasterPriority2;

	/* Global time properties data set */

	/*Dynamic members*/
	Integer16 currentUtcOffset;
	Boolean currentUtcOffsetValid:1;
	Boolean leap59:1;
	Boolean leap61:1;
	Boolean timeTraceable:1;
	Boolean frequencyTraceable:1;
	Boolean ptpTimescale:1;
	Boolean leapSecondInProgress:1;
	Boolean leapSecondPending:1;
	Enumeration8 timeSource;

	/* Port configuration data set */

	/*Static members*/
	PortIdentity portIdentity;

	/*Dynamic members*/
	Enumeration8 portState;
	Integer8 logMinDelayReqInterval;
	TimeInternal peerMeanPathDelay;
 
	/*Configurable members*/
	Integer8 logAnnounceInterval;
	UInteger8 announceReceiptTimeout;
	Integer8 logSyncInterval;
	Enumeration8 delayMechanism;
	Integer8 logMinPdelayReqInterval;
	UInteger4 versionNumber;


	/* Foreign master data set */
	ForeignMasterRecord *foreign;

	/* Other things we need for the protocol */
	UInteger16 number_foreign_records;
	Integer16  max_foreign_records;
	Integer16  foreign_record_i;
	Integer16  foreign_record_best;
	UInteger32 random_seed;
	Boolean  record_update;    /* should we run bmc() after receiving an announce message? */


	MsgHeader msgTmpHeader;

	union {
		MsgSync  sync;
		MsgFollowUp  follow;
		MsgDelayReq  req;
		MsgDelayResp resp;
		MsgPDelayReq  preq;
		MsgPDelayResp  presp;
		MsgPDelayRespFollowUp  prespfollow;
		MsgManagement  manage;
		MsgAnnounce  announce;
		MsgSignaling signaling;
	} msgTmp;

	//MsgManagement outgoingManageTmp;

	Octet msgObuf[PACKET_SIZE];
	Octet msgIbuf[PACKET_SIZE];

/*
	20110630: These variables were deprecated in favor of the ones that appear in the stats log (delayMS and delaySM)
	
	TimeInternal  master_to_slave_delay;
	TimeInternal  slave_to_master_delay;

	*/
	Integer32 	observed_drift;

	TimeInternal  pdelay_req_receive_time;
	TimeInternal  pdelay_req_send_time;
	TimeInternal  pdelay_resp_receive_time;
	TimeInternal  pdelay_resp_send_time;
	TimeInternal  sync_send_time;
    TimeInternal  sync_receive_time;
	TimeInternal  delay_req_send_time;
	TimeInternal  delay_req_receive_time;
	MsgHeader		PdelayReqHeader;
	MsgHeader		delayReqHeader;
	TimeInternal	pdelayMS;
	TimeInternal	pdelaySM;
	TimeInternal    delayMS;
	TimeInternal	delaySM;
	TimeInternal  lastSyncCorrectionField;
	TimeInternal  lastPdelayRespCorrectionField;

        TimeInternal correctionFieldSM;
        TimeInternal correctionFieldMS;


	Boolean  sentPDelayReq;
	UInteger16  sentPDelayReqSequenceId;
	UInteger16  sentDelayReqSequenceId;
	UInteger16  sentSyncSequenceId;
	UInteger16  sentAnnounceSequenceId;
	UInteger16  recvPDelayReqSequenceId;
	UInteger16  recvSyncSequenceId;
	UInteger16  recvPDelayRespSequenceId;
	Boolean  waitingForFollow;
	Boolean  waitingForDelayResp;
	
	offset_from_master_filter  ofm_filt;
	one_way_delay_filter  owd_filt;

	Boolean message_activity;

	IntervalTimer  itimer[TIMER_ARRAY_SIZE];

	NetPath netPath;

	/*Usefull to init network stuff*/
	UInteger8 port_communication_technology;
	Octet port_uuid_field[PTP_UUID_LENGTH];

	int reset_count;
	int warned_operator_slow_slewing;
	int warned_operator_fast_slewing;

	char char_last_msg;                             /* representation of last message processed by servo */
	Boolean last_packet_was_sync;                   /* used to limit logging of Sync messages */
	
	int waiting_for_first_sync;                     /* we'll only start the delayReq timer after the first sync */
	int waiting_for_first_delayresp;                /* Just for information purposes */
	Boolean startup_in_progress;

	Integer32 last_saved_drift;                     /* Last observed drift value written to file */
	Boolean drift_saved;                            /* Did we save a drift value already? */

	/* user description is max size + 1 to leave space for a null terminator */
	Octet user_description[USER_DESCRIPTION_MAX + 1];

#ifdef PTPD_EXPERIMENTAL
	Integer32 MasterAddr;                           // used for hybrid mode, when receiving announces
	Integer32 LastSlaveAddr;                        // used for hybrid mode, when receiving delayreqs
#endif

	/*
	 * basic clock statistics information, useful
	 * for monitoring servo performance and estimating
	 * clock stability - should be exposed through
	 * management messages and SNMP eventually
	*/
	PtpdStats stats;

    TimeInternal tickInterval;
} PtpClock;

/**
 * \struct RunTimeOpts
 * \brief Program options set at run-time
 */
/* program options set at run-time */
typedef struct {
	Integer8 announceInterval;
	Integer8 announceReceiptTimeout;
	
	Integer8 syncInterval;
	ClockQuality clockQuality;
	UInteger8 priority1;
	UInteger8 priority2;
	UInteger8 domainNumber;
#ifdef PTPD_EXPERIMENTAL
	UInteger8 mcast_group_Number;
#endif

	Boolean	slaveOnly;
    Boolean currentUtcOffsetValid;
	Integer16 currentUtcOffset;
    Boolean leap59:1;
    Boolean leap61:1;
    Boolean timeTraceable:1;
    Boolean frequencyTraceable:1;
    Boolean ptpTimescale:1;

    Boolean twoStepFlag;

    Enumeration8 timeSource;
	Octet ifaceName[IFACE_NAME_LENGTH];
	Boolean	noResetClock;

	Boolean	noAdjust;
	Boolean	displayStats;

#ifdef PTPD_UNICAST_MODE
	Octet unicastAddress[MAXHOSTNAMELEN];
#endif
	Integer32 ap, ai;
	Integer16 s;
	TimeInternal inboundLatency, outboundLatency;
	Integer16 max_foreign_records;
	Boolean ethernet_mode;
	Enumeration8 delayMechanism;
	Boolean	offset_first_updated;
	int ttl;
	int log_seconds_between_message;

	Boolean do_IGMP_refresh;
	
	int initial_delayreq;
	int subsequent_delayreq;
	Boolean ignore_delayreq_interval_master;//是否设置为主延时间隔

	int drift_recovery_method; /* how the observed drift is managed between restarts */

#ifdef PTPD_EXPERIMENTAL
	Boolean do_hybrid_mode;
#endif
#ifdef PTPD_UNICAST_MODE
	Boolean do_unicast_mode;
#endif

#ifdef RUNTIME_DEBUG
	int debug_level;
#endif

} RunTimeOpts;

#endif /*DATATYPES_H_*/
