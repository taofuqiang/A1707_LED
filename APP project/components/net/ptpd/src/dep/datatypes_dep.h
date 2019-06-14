#ifndef DATATYPES_DEP_H_
#define DATATYPES_DEP_H_

#include "rtthread.h"
/**
*\file
* \brief Implementation specific datatype

 */
/* FIXME: shouldn't uint32_t and friends be used here? */
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
typedef bool Boolean;
typedef char Octet;
typedef int8_t Integer8;
typedef int16_t Integer16;
typedef int32_t Integer32;
typedef uint8_t UInteger8;
typedef uint16_t UInteger16;
typedef uint32_t UInteger32;
typedef uint_least16_t Enumeration16;
typedef uint_least8_t Enumeration8;
typedef uint_least8_t Enumeration4;
typedef Enumeration4 Enumeration4Upper;
typedef Enumeration4 Enumeration4Lower;
typedef uint8_t UInteger4;
typedef UInteger4 UInteger4Upper;
typedef UInteger4 UInteger4Lower;
typedef uint8_t Nibble;
typedef Nibble NibbleUpper;
typedef Nibble NibbleLower;

/**
* \brief Implementation specific of UInteger48 type
 */
typedef struct {
	unsigned int lsb;     /* FIXME: shouldn't uint32_t and uint16_t be used here? */
	unsigned short msb;
} UInteger48;

/**
* \brief Implementation specific of Integer64 type
 */
typedef struct {
	unsigned int lsb;     /* FIXME: shouldn't uint32_t and int32_t be used here? */
	int msb;
} Integer64;

/**
* \brief Struct used to average the offset from master
*
* The FIR filtering of the offset from master input is a simple, two-sample average
 */
typedef struct {
    Integer32  nsec_prev, y;
} offset_from_master_filter;

/**
* \brief Struct used to average the one way delay
*
* It is a variable cutoff/delay low-pass, infinite impulse response (IIR) filter.
*
*  The one-way delay filter has the difference equation: s*y[n] - (s-1)*y[n-1] = x[n]/2 + x[n-1]/2, where increasing the stiffness (s) lowers the cutoff and increases the delay.
 */
typedef struct {
    Integer32  nsec_prev, y;
    Integer32  s_exp;
} one_way_delay_filter;

/**
* \brief Struct used to store network datas
 */
typedef struct {
    struct netconn * eventConn;
    struct netconn * generalConn;

#define eventSock       eventConn    
#define generalSock       generalConn
    
    struct netbuf * sendBuf;

    Integer32 multicastAddr, peerMulticastAddr;
#ifdef PTPD_UNICAST_MODE
    Integer32 unicastAddr;
#endif

    /* used by IGMP refresh */
    struct in_addr interfaceAddr;

#ifdef PTPD_EXPERIMENTAL
    /* used for Hybrid mode */
    Integer32 lastRecvAddr;
#endif

    uint64_t sentPackets;
    uint64_t receivedPackets;

} NetPath;

#endif /*DATATYPES_DEP_H_*/
