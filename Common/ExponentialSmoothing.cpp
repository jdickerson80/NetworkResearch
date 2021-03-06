#include "ExponentialSmoothing.h"

#include <cmath>

namespace Common {
namespace Math {

ExponentialSmoothing::ExponentialSmoothing( float alpha )
	: _alpha( alpha )
	, _rate( 0 )
{

}

ExponentialSmoothing::~ExponentialSmoothing()
{

}

float ExponentialSmoothing::calculate( const DataValues& dataValues, const TimeValues& timeValues )
{
	// local variable
	unsigned int dataDelta;
	unsigned int timeDelta;

	// calculation of the delta
	dataDelta = std::abs( dataValues.currentValue - dataValues.lastValue );
	timeDelta = std::abs( timeValues.currentValue - timeValues.lastValue );

	// make sure the time delta is not zero
	timeDelta = timeDelta == 0 ? 1 : timeDelta;

	// calculate the rate using the last rate
	_rate = (float)( ( dataDelta / timeDelta ) ) * _alpha + ( 1 - _alpha ) * _rate;

	// bound the rate
	_rate = _rate < 0 ? 0 : _rate;

//	PRINT( "dd %u td %u\n"
//			, dataDelta
//			, timeDelta );

	// and return it
	return _rate;
}

float ExponentialSmoothing::alpha() const
{
	return _alpha;
}

void ExponentialSmoothing::setAlpha( float alpha )
{
	_alpha = alpha;
}

} // namespace Math
} // namespace Common
