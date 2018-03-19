#ifndef CURRENTANDLASTVALUES_H
#define CURRENTANDLASTVALUES_H


/**
 * @brief	The CurrentAndLastValues struct just wraps a
 *			previous and current value.
 *
 */
template< typename DataType >
struct CurrentAndLastValues
{
    DataType currentValue;
    DataType lastValue;

	/**
	 * @brief CurrentAndLastValues default constructor for all zero
	 *		  intialization
	 */
    CurrentAndLastValues()
        : currentValue( 0 )
        , lastValue( 0 ) {}

	/**
	 * @brief CurrentAndLastValues overload of the constructor
	 *		  to allow setting of the values
	 * @param currentValue
	 * @param lastValue
	 */
    CurrentAndLastValues( const DataType& currentValue, const DataType& lastValue )
        : currentValue( currentValue )
        , lastValue( lastValue ) {}

	/**
	 * @brief ~CurrentAndLastValues Do nothing destructor
	 */
    ~CurrentAndLastValues() {}
};
#endif // CURRENTANDLASTVALUES_H
