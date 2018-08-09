/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef EXPONENTIALSMOOTHING_H
#define EXPONENTIALSMOOTHING_H

#include <time.h>

#include "CurrentAndLastValues.h"

namespace Common {
namespace Math {

/**
 * @brief The ExponentialSmoothing class calculates the rate by the
 *        simplest form of exponential smoothing formula found
 *        at the address in the note.
 *
 * @note https://en.wikipedia.org/wiki/Exponential_smoothing
 */
class ExponentialSmoothing
{
public:

	// Typedef of the data values
	typedef CurrentAndLastValues< float > DataValues;

private:

	// Typedef of the time values
	typedef CurrentAndLastValues< time_t > TimeValues;

	// Alpha value
	float _alpha;

	// Rate variable
	float _rate;

public:

	/**
	 * @brief ExponentialSmoothing default constructor
	 * @param alpha
	 */
	ExponentialSmoothing( float alpha = 1.0 );

	/**
	 * @brief ExponentialSmoothing do nothing destructor
	 */
	~ExponentialSmoothing();

	/**
	 * @brief calculate method calculates the rate
	 * @param dataValues current data value
	 * @param timeValues current time value
	 * @return new rate
	 */
	float calculate( const DataValues& dataValues, const TimeValues& timeValues );

	/**
	 * @brief alpha getter
	 * @return alpha
	 */
	float alpha() const;

	/**
	 * @brief setAlpha
	 * @param alpha
	 */
	void setAlpha( float alpha );
};

} // namespace Math
} // namespace Common
#endif // EXPONENTIALSMOOTHING_H
