#include "BandwidthGuaranteeHost.h"

namespace BGAdaptor {

BandwidthGuaranteeHost::BandwidthGuaranteeHost()
	: _demand( 0 )
	, _guarantee( 0 )
	, _lastDemand( 0 )
	, _lastGuarantee( 0 )
{

}

const sockaddr_in& BandwidthGuaranteeHost::address() const
{
	return _address;
}

void BandwidthGuaranteeHost::setAddress( const sockaddr_in address )
{
	_address = address;
}

const std::atomic_int& BandwidthGuaranteeHost::delta() const
{
	return _delta;
}

//void BandwidthGuaranteeHost::setDelta( int delta )
//{
//	_delta = delta;
//}

const std::atomic_uint& BandwidthGuaranteeHost::demand() const
{
	return _demand;
}

void BandwidthGuaranteeHost::setDemand( uint demand )
{
	_demand = demand;
	calculateDelta();
}

const std::atomic_uint& BandwidthGuaranteeHost::guarantee() const
{
	return _guarantee;
}

void BandwidthGuaranteeHost::setGuarantee( uint guarantee )
{
	_guarantee = guarantee;
	calculateDelta();
}

void BandwidthGuaranteeHost::calculateDelta()
{
	_delta = static_cast< int >( _guarantee - _demand );
}
} // namespace BGAdaptor
