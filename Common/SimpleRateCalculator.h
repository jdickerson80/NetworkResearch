/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef SIMPLERATECALCULATOR_H
#define SIMPLERATECALCULATOR_H

#include <ctime>

#include "CurrentAndLastValues.h"
#include "SimpleRateCalculation.h"

namespace Common {
namespace Math {

/**
 * @brief The SimpleRateCalculator class calculates the rate by:
 *			  current value - last value
 *			------------------------------
 *			current time - last time value
 */
template< typename CalculationDataType, typename RateDataType >
class SimpleRateCalculator
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
	SimpleRateCalculation _rateCalculation;

	// Variables for the data and time
	CalculatorDataValues _dataValues;
	CalculatorTimeValues _timeValues;

public:

	/**
	 * @brief DataRateCalculator
	 * @param statisticPath
	 */
	SimpleRateCalculator()
		: _rate( 0 )
	{
		_dataValues.currentValue = _dataValues.lastValue = 0;

		// get the initial times
		_timeValues.currentValue = _timeValues.lastValue = time( nullptr );
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
#endif // SIMPLERATECALCULATOR_H
