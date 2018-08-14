#ifndef BANDWIDTHVALUES_H
#define BANDWIDTHVALUES_H

#include <stdint.h>

namespace WCEnabler {

/**
 * @brief	The BandwidthValues struct contains all of the values
 *			pertinent to bandwidth calcs
 */
struct BandwidthValues
{
	unsigned int bandwidthGuarantee;
	unsigned int bandwidthGuaranteeRate;
	unsigned int ecnValue;
	unsigned int totalRate;
	unsigned int workConservingRate;

	BandwidthValues()
		: bandwidthGuarantee( UINT32_MAX ) // use a large value, so nothing happens until we get a value from BGAdaptor
		, bandwidthGuaranteeRate( 0 )
		, ecnValue( 0 )
		, totalRate( 0 )
		, workConservingRate( 0 )
	{}
};

}

#endif // BANDWIDTHVALUES_H
