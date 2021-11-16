#include "TradingSimulator.h"
#include "stonks.h"
#include "sutil.h"

#include <map>
#include <vector>
#include <queue>

#pragma warning(disable:4100)


namespace stonks
{

enum SymbolState
{
	none,		// do not own this stock and have never owned it
	own		// currently own some of this stock
};

class PendingCash
{
public:
	double	mAmount{0};
	uint32_t mTradingDay{0};
};

using PendingCashQueue = std::queue< PendingCash >;

class StockSymbol
{
public:
	std::string		mSymbol;
	stonks::Price	mStartingPrice;
	stonks::Price	mEndingPrice;
	double			mSharesOwned{0};	// number of shares owned
	double			mSharesPrice{0};	// price paid for them
	SymbolState	mState{SymbolState::none};
	double		mLastPrice{0}; // last price we bought or sold at..
	uint32_t	mLastDate{0};	// last trading day
	double		mCurrentPrice{0};
	bool		mVeryFirstTime{true};
};

using StockSymbolMap = std::map< std::string, StockSymbol >;

class Account
{
public:

	Account(const TradingParameters &params,
			const StockList &stocks,
			Stonks *st)
	{
		mParams = params;
		mStonks = st;
		mSettledCash = mParams.mStartingCash;
		for (auto &i:stocks)
		{
			const stonks::Stock *s = st->find(i);
			if ( s )
			{
				StockSymbol ss;
				if( s->getPrice(params.mStartTradingDay,ss.mStartingPrice) && s->getPrice(params.mEndTradingDay,ss.mEndingPrice))
				{
					ss.mSymbol = s->mSymbol;
					ss.mLastDate = params.mStartTradingDay;
					ss.mLastPrice = ss.mStartingPrice.mPrice;
					mStocks[ss.mSymbol] = ss;
				}
			}
		}
		mCurrentDay = mParams.mStartTradingDay;
		double fraction = 1.0 / double(mStocks.size());
		double total = 0;
		for (auto &i:mStocks)
		{
			double budget = params.mStartingCash * fraction;
			double shares = budget / i.second.mStartingPrice.mPrice;
			double currentValue = shares * i.second.mEndingPrice.mPrice;
			total+=currentValue;
		}
		mHoldResults = total;
		printf("Portfolio Return: $%s\n", sutil::formatNumber(int32_t(total)) );
	}

	bool simulateDay(void)
	{
		bool ret = false;

		if ( mCurrentDay > mParams.mEndTradingDay )
		{
			double currentValue = mUnsettledCash + mSettledCash;
			for (auto &i:mStocks)
			{
				double position = i.second.mSharesOwned*i.second.mCurrentPrice;
				currentValue+=position;
			}
			printf("Simulation complete: Portfolio Value: $%s vs. Holding:$%s\n", 
				sutil::formatNumber(int32_t(currentValue)),
				sutil::formatNumber(int32_t(mHoldResults)));

			double percentTrade = ((currentValue-mParams.mStartingCash)*100) / mParams.mStartingCash;
			double percentHold = ((mHoldResults-mParams.mStartingCash)*100) / mParams.mStartingCash;
			printf("HOLD:%%%0.2f TRADE:%%%0.2f\n", percentHold, percentTrade);

			ret = true;
		}
		else
		{
#if 0
			printf("=======================================================\n");
			printf("Simulate Day:%d\n", mCurrentDay);
			printf("=======================================================\n");

			printf("Settled Cash:   $%s\n", sutil::formatNumber(mSettledCash));
			printf("Unsettled Cash: $%s\n", sutil::formatNumber(mUnsettledCash));
			double stockPosistions=0;
			for (auto &i:mStocks)
			{
				StockSymbol &ss = i.second;
				if ( ss.mState == SymbolState::own )
				{
					double value = ss.mSharesOwned*ss.mCurrentPrice;
					stockPosistions+=value;
				}
			}
			printf("Stock Value: $%s\n", sutil::formatNumber(stockPosistions));
			double totalValue = mSettledCash+mUnsettledCash+stockPosistions;
			printf("Total portfolio value: $%s\n", sutil::formatNumber(totalValue));
			printf("=======================================================\n");
#endif

			class PriceChange
			{
			public:
				bool operator<(const PriceChange &a) const
				{
					return a.mPercent < mPercent;
				}
				double	mPercent{0};
				std::string	mSymbol;
			};

			// Sort by maximum percentage price drop
			std::priority_queue< PriceChange > pchange;
			for (auto &i:mStocks)
			{
				StockSymbol &ss = i.second;
				const stonks::Stock *s = mStonks->find(ss.mSymbol);
				if ( s )
				{
					Price p;
					if( s->getPrice(mCurrentDay,p))
					{
						ss.mCurrentPrice = p.mPrice; // what the current price is
						double diff = p.mPrice - ss.mLastPrice; // what is the difference to the reference price...
						double percentDifference = (diff*100) / ss.mLastPrice; // percentage change
						PriceChange pc;
						pc.mPercent = percentDifference;
						pc.mSymbol = ss.mSymbol;
						pchange.push(pc);
					}
				}
			}
			while ( !pchange.empty() )
			{
				PriceChange pcg = pchange.top();
				pchange.pop();
				StockSymbolMap::iterator found = mStocks.find(pcg.mSymbol);
				if ( found != mStocks.end() )
				{
					StockSymbol &ss = (*found).second;
					const stonks::Stock *s = mStonks->find(ss.mSymbol);
					if ( s )
					{
						Price p;
						if( s->getPrice(mCurrentDay,p))
						{
							ss.mCurrentPrice = p.mPrice; // what the current price is
							double diff = p.mPrice - ss.mLastPrice; // what is the difference to the reference price...
							double percentDifference = (diff*100) / ss.mLastPrice; // percentage change
							// If we do not currently own this stock, either update the last price of it's higher than what we are currently tracking, or
							// decide this is a buying opportunity
							if ( ss.mState == SymbolState::none ) 
							{
								if ( percentDifference <= mParams.mPercentFirstBuy || ss.mVeryFirstTime)
								{
									ss.mVeryFirstTime = false;
									int32_t shares =(int32_t) (mParams.mFirstBuy / p.mPrice)+1;  //how many shares to buy...
									double cost = double(shares)*p.mPrice;
									if ( cost <= mSettledCash )
									{
										ss.mState = SymbolState::own;
										ss.mSharesOwned+=double(shares);
										ss.mSharesPrice+=cost;
										ss.mLastPrice = p.mPrice;
										mSettledCash-=cost;
										printf("Bought %d shares of %s for $%s\n", shares, ss.mSymbol.c_str(), sutil::formatNumber(int32_t(cost)));
									}
								}
								if ( p.mPrice > ss.mLastPrice )
								{
									ss.mLastPrice = p.mPrice;
								}
							}
							else if ( ss.mState == SymbolState::own )
							{
								if ( percentDifference <= mParams.mPercentRebuy )
								{
									// We cannot rebuy if that would cause us to go over our maximum buy limit
									if ( ss.mSharesPrice < (mParams.mMaxBuy-mParams.mRebuy) )
									{
										int32_t shares =(int32_t) (mParams.mRebuy / p.mPrice)+1;  //how many shares to buy...
										double cost = double(shares)*p.mPrice;
										if ( cost <= mSettledCash )
										{
											ss.mState = SymbolState::own;
											ss.mLastPrice = p.mPrice;
											ss.mSharesOwned+=double(shares);
											ss.mSharesPrice+=cost;
											mSettledCash-=cost;
											printf("Bought more %d shares of %s for $%s\n", shares, ss.mSymbol.c_str(), sutil::formatNumber(int32_t(cost)));
										}
									}
								}
								else
								{
									double currentValue = ss.mSharesOwned*p.mPrice;
									double diffValue = currentValue - ss.mSharesPrice;
									double percentIncrease = (diffValue*100) / ss.mSharesPrice;
									if ( percentIncrease >= mParams.mTakeProfit)
									{
										mUnsettledCash+=currentValue;
										PendingCash pc;
										pc.mTradingDay = mCurrentDay;
										pc.mAmount = currentValue;
										mPendingCash.push(pc);

										double profit = currentValue - ss.mSharesPrice;
										printf("Sold %d shares of %s for $%s banking profit of:$%s\n", 
											int32_t(ss.mSharesOwned), 
											ss.mSymbol.c_str(), 
											sutil::formatNumber(int32_t(currentValue)),
											sutil::formatNumber(int32_t(profit)));

										ss.mState = SymbolState::none;
										ss.mLastPrice = p.mPrice;
										ss.mSharesOwned = 0;
										ss.mSharesPrice = 0;

									}
								}
							}
						}
					}
				}
			}
			processPendingCash();
			mCurrentDay++;
		}

		return ret;
	}

	void processPendingCash(void)
	{
		while ( !mPendingCash.empty() )
		{
			PendingCash pc = mPendingCash.front();
			uint32_t diff = mCurrentDay - pc.mTradingDay;
			if ( diff >= 3 )
			{
				mSettledCash+=pc.mAmount;
				mUnsettledCash-=pc.mAmount;

				printf("Settled:$%s : Unsettled:$%s\n",
					sutil::formatNumber(int32_t(mSettledCash)),
					sutil::formatNumber(int32_t(mUnsettledCash)));

				mPendingCash.pop();
			}
			else
			{
				break;
			}
		}
	}

	TradingParameters	mParams;
	uint32_t		mCurrentDay{0};
	double			mHoldResults{0}; // how much money we would have if we just bought and held over this period of time
	double			mSettledCash{0};
	double			mUnsettledCash{0};
	StockSymbolMap	mStocks;
	PendingCashQueue mPendingCash;
	Stonks			*mStonks{nullptr};
};

class TradingSimulatorImpl : public TradingSimulator
{
public:

	virtual double runSimulation(const TradingParameters &params,
								const StockList &stocks,
								Stonks *s) final
	{
		double ret = 0;

		// Step #1 : Build a list of stocks which match this date
		Account a(params,stocks,s);

		while ( !a.simulateDay() )
		{
			// run simulation for each trading day
		}



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
