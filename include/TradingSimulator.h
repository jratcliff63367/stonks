#pragma once

// simulates trading using the algorithm I have developed
#include <stdint.h>
#include <vector>
#include <string>

namespace stonks
{

using StockList = std::vector< std::string>;

class TradingParameters
{
public:
	uint32_t	mStartTradingDay{0};
	uint32_t	mEndTradingDay{0};
	double		mStartingCash{500000};
	double		mMaxBuy{15000}; // don't spend more than 5k on any one stock
	double		mFirstBuy{5000}; // on the first buy, only 2k
	double		mRebuy{5000};		// on additoinal buys, 1k
	double		mPercentFirstBuy{-5}; // initial buy, must be a 5% drop
	double		mPercentRebuy{-3};			// rebuy on an additional 3% drop
	double		mTakeProfit{3};		// take profit on 3% gains
};

class Stonks;

class TradingSimulator
{
public:
	static TradingSimulator *create(void);

	virtual double runSimulation(const TradingParameters &params,
								 const StockList &stocks,
								 Stonks *s) = 0;

	virtual void release(void) =0;
protected:
	virtual ~TradingSimulator(void)
	{
	}
};

}
