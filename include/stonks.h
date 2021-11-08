#pragma once

#include <string>
#include <stdint.h>

namespace stonks
{

class Stock
{
public:
	std::string	mSymbol;
	std::string mName;
	double 	mMarketCap{0};
	std::string	mCountry;
	uint32_t	mIPOYear{0};
	uint32_t	mVolume{0};
	std::string	mSector;
	std::string	mIndustry;
};

class Stonks
{
public:
	static Stonks *create(void);

	virtual const Stock *getStock(const std::string &symbol) const = 0;

	virtual uint32_t begin(void) = 0; // begin iterating stock symbols, returns the number available.
	virtual const Stock *next(void) = 0; // goes to the next

	virtual void release(void) = 0;
protected:
	virtual ~Stonks(void)
	{
	}
};

}