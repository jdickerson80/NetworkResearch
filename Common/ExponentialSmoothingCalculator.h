#ifndef EXPONENTIALSMOOTHINGCALCULATOR_H
#define EXPONENTIALSMOOTHINGCALCULATOR_H

#include <ctime>

#include "CurrentAndLastValues.h"
#include "ExponentialSmoothing.h"

namespace Common {
namespace Math {

/**
 * @brief	The ExponentialSmoothingCalculator class wraps the exponential smoothing
 *			calculation to make it easier to use.
 */
template< typename CalculationDataType, typename RateDataType >
class ExponentialSmoothingCalculator
{
public:

	// Typedef for the data's current and last values
	typedef CurrentAndLastValues< CalculationDataType > CalculatorDataValues;

private:

	// Typedef for the time's current and last values
	typedef CurrentAndLastValues< time_t > CalculatorTimeValues;

	// Hold the rate to get it without calculation
	RateDataType _rate;

	// Rate calculation
	ExponentialSmoothing _rateCalculation;

	// Variables for the data and time
	CalculatorDataValues _dataValues;
	CalculatorTimeValues _timeValues;

public:

	/**
	 * @brief ExponentialSmoothingCalculator
	 */
	ExponentialSmoothingCalculator( float alpha )
		: _rate( 0 )
		, _rateCalculation( alpha )
	{
		_dataValues.currentValue = _dataValues.lastValue = 0;

		// get the initial times
		_timeValues.currentValue = _timeValues.lastValue = time( NULL );
	}

	/**
	 * @brief calculateRate calculates the rate and returns it
	 * @return new rate
	 */
	RateDataType calculateRate( CalculationDataType currentValue )
	{
		RateDataType tempRate;
		_dataValues.currentValue = currentValue;

		// get the current time
		std::time( &_timeValues.currentValue );

		// calculate the rate
		tempRate = _rateCalculation.calculate( _dataValues, _timeValues );

		// update the last value variables
		_dataValues.lastValue = _dataValues.currentValue;
		_timeValues.lastValue = _timeValues.currentValue;

		_rate = tempRate;
		return _rate;
	}

	/**
	 * @brief rate getter
	 * @return rate
	 */
	RateDataType rate() const
	{
		return _rate;
	}
};

} // namespace Math
} // namespace Common

#endif // EXPONENTIALSMOOTHINGCALCULATOR_H
