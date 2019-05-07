#ifndef WCPRINTHANDLER_H
#define WCPRINTHANDLER_H

#include <stdio.h>

//#define ALLOWPRINTING

#if defined( ALLOWPRINTING )
	#define PRINT( format, args... ) fprintf( stdout, format, ##args );
#else
	#define PRINT( format, args... );
#endif

#endif // WCPRINTHANDLER_H
