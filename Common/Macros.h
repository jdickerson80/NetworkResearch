#ifndef MACROS_H
#define MACROS_H

#define INET_ECN_NOT_ECT    ( 0x00 )    /* ECN was not enabled */
#define INET_ECN_ECT_1      ( 0x01 )    /* ECN capable packet */
#define INET_ECN_ECT_0      ( 0x02 )    /* ECN capable packet */
#define INET_ECN_CE         ( 0x03 )    /* ECN congestion */
#define INET_ECN_MASK       ( 0x03 )    /* Mask of ECN bits */

#define WorkConservationFlowDecimalForm ( 56 )
#define WorkConservationFlowHexForm ( 38 )
#define WorkConvervationMask ( 0xFC )

#endif // MACROS_H
