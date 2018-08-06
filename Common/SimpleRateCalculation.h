/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef SIMPLERATECALCULATION_H
#define SIMPLERATECALCULATION_H

#include "CurrentAndLastValues.h"
#include <time.h>

namespace Common {
namespace Math {

/**
 * @brief The ExponentialSmoothing class calculates the rate by the
 *        simplest form of exponential smoothing formula found
 *        at the address in the note.
 *
 * @note https://en.wikipedia.org/wiki/Exponential_smoothing
 */
class SimpleRateCalculation
{
public:

	// Typedef of the data values
	typedef CurrentAndLastValues< unsigned int > DataValues;

private:

	// Typedef of the time values
	typedef CurrentAndLastValues< time_t > TimeValues;

	// Rate variable
	float _rate;

public:

	/**
	 * @brief SimpleRateCalculation default constructor
	 */
	SimpleRateCalculation();

	/**
	 * @brief SimpleRateCalculation do nothing destructor
	 */
	~SimpleRateCalculation();

	/**
	 * @brief calculate method calculates the rate
	 * @param dataValues current data value
	 * @param timeValues current time value
	 * @return new rate
	 */
	float calculate( const DataValues& dataValues, const TimeValues& timeValues );
};

} // namespace Math
} // namespace Common

#endif // SIMPLERATECALCULATION_H
