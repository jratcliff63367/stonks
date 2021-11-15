#include "TradingSimulator.h"
#include "stonks.h"

#include <map>
#include <vector>

#pragma warning(disable:4100)


namespace stonks
{

enum class OperationType
{
	buy,
	sell
};

class Operation
{
public:
	OperationType	mOperation{OperationType::buy};
	std::string	mSymbol;
	double		mShares{0};
	double		mPrice{0};
	uint32_t	mTradingDay{0};
};

enum SymbolState
{
	none,		// do not own this stock and have never owned it
	own,		// currently own some of this stock
	sold		// previously owned it, but sold it
};

class StockSymbol
{
public:
	std::string	mSymbol;
	SymbolState	mState{SymbolState::none};
	double		mLastPrice{0}; // last price we bought or sold at..
	uint32_t	mLastDate{0};	// last trading day
};

using OperationVector = std::vector< Operation >;
using StockSymbolMap = std::_In_place_key_extract_map< std::string, StockSymbol >;

class Account
{
public:

	void init(const StockList &stocks,const Stonks *st)
	{
		for (auto &i:stocks)
		{
			const stonks::Stock *s = st->find(i);
			if ( s )
			{

			}
		}
	}

	double			mSettledCash{0};
	double			mUnsettledCash{0};
	OperationVector	mOperations;
	StockSymbolMap	mStocks;
};

class TradingSimulatorImpl : public TradingSimulator
{
public:

	virtual double runSimulation(const TradingParameters &params,
								const StockList &stocks,
								Stonks *s) final
	{
		double ret = 0;



		return ret;
	}

	virtual void release(void) final
	{
		delete this;
	}
};

TradingSimulator *TradingSimulator::create(void)
{
	auto ret = new TradingSimulatorImpl;
	return static_cast< TradingSimulator *>(ret);
}


}
