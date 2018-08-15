#ifndef BANDWIDTHVALUES_H
#define BANDWIDTHVALUES_H

#include <atomic>

namespace WCEnabler {

/**
 * @brief	The BandwidthValues struct contains all of the values
 *			pertinent to bandwidth calcs
 */
struct BandwidthValues
{
	std::atomic_uint bandwidthGuarantee;
	std::atomic_uint bandwidthGuaranteeRate;
	std::atomic_uint ecnValue;
	std::atomic_uint totalRate;
	std::atomic_uint workConservingRate;

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
