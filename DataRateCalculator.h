#ifndef DATARATECALCULATOR_H
#define DATARATECALCULATOR_H

#include <string>
#include <fstream>
#include <ctime>
#include "CurrentAndLastValues.h"

/**
 * @brief	The DataRateCalculator calculates the rate with the given template arguments.
 * @param 	CalculationDataType is the data type of the data that will be calculated.
 * @param	RateDataType is the data type of the rate. Could be float for better
 *			precision.
 * @param	RateCalculation is the class that will calculate the rate.
 */
template< typename CalculationDataType, typename RateDataType, typename RateCalculation >
class DataRateCalculator
{
public:

	// Typedef for the data's current and last values
	typedef CurrentAndLastValues< CalculationDataType > CalculatorDataValues;

private:

	// Typedef for the time's current and last values
	typedef CurrentAndLastValues< time_t > CalculatorTimeValues;

	// Path to the data to calculate a rate
	const std::string _statisticPath;

	// Hold the rate to get it without calculation
	RateDataType _rate;

	// Rate calculation
	RateCalculation _rateCalculation;

	// Variables for the data and time
	CalculatorDataValues _dataValues;
	CalculatorTimeValues _timeValues;

public:

	/**
	 * @brief DataRateCalculator
	 * @param statisticPath
	 */
	DataRateCalculator( const std::string& statisticPath )
		: _statisticPath( statisticPath )
		, _rate( 0 )
	{
		// get the initial values
		std::ifstream inputStream;
		std::string input;
		inputStream.open( _statisticPath.c_str() );
		std::getline( inputStream, input );
		_dataValues.currentValue = _dataValues.lastValue
				= static_cast< CalculationDataType >( std::atof( input.c_str() ) );

		// get the initial times
		_timeValues.currentValue = _timeValues.lastValue = time( NULL );
	}

	/**
	 * @brief calculateRate calculates the rate and returns it
	 * @return new rate
	 */
	RateDataType calculateRate()
	{
		// local variable init
		std::ifstream inputStream;
		std::string input;

		// get the current value from the path
		inputStream.open( _statisticPath.c_str() );
		getline( inputStream, input );
		inputStream.close();
		_dataValues.currentValue = static_cast< CalculationDataType >( std::atof( input.c_str() ) );

		// get the current time
		std::time( &_timeValues.currentValue );

		// calculate the rate
		_rate = _rateCalculation.calculate( _dataValues, _timeValues );

		// update the last value variables
		_dataValues.lastValue = _dataValues.currentValue;
		_timeValues.lastValue = _timeValues.currentValue;

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

#endif // DATARATECALCULATOR_H
