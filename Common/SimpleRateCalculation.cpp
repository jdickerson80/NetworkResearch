#include "SimpleRateCalculation.h"

namespace Common {
namespace Math {

SimpleRateCalculation::SimpleRateCalculation()
	: _rate( 0 )
{

}

SimpleRateCalculation::~SimpleRateCalculation()
{

}

float SimpleRateCalculation::calculate( const DataValues& dataValues, const TimeValues& timeValues )
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
	_rate = (float)( ( dataDelta / timeDelta ) );

	// bound the rate
	_rate = _rate < 0 ? 0 : _rate;

	// and return it
	return _rate;
}

} // namespace Math
} // namespace Common
