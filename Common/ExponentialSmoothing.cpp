#include "ExponentialSmoothing.h"

//static float Alpha = 1.0;
namespace Common {
namespace Math {

ExponentialSmoothing::ExponentialSmoothing()
	: _rate( 0 )
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
	dataDelta = dataValues.currentValue - dataValues.lastValue;
	timeDelta = timeValues.currentValue - timeValues.lastValue;

	// make sure the time delta is not zero
	timeDelta = timeDelta <= 0 ? 1 : timeDelta;

	// calculate the rate using the last rate
	_rate = (float)( ( dataDelta / timeDelta ) );// * Alpha + ( 1 - Alpha ) * _rate;

	// bound the rate
	_rate = _rate < 0 ? 0 : _rate;

	// and return it
	return _rate;
}

} // namespace Math

} // namespace Common
