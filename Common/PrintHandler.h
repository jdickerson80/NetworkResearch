#ifndef PRINTHANDLER_H
#define PRINTHANDLER_H

#include <stdio.h>

#define ALLOWPRINTING

#if defined( ALLOWPRINTING )
	#define PRINT( format, args... ) fprintf( stdout, format, ##args );
#else
	#define PRINT( format, args... ) ;
#endif

#endif // PRINTHANDLER_H
