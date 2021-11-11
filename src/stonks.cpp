#include "stonks.h"
#include "KeyValueDatabase.h"
#include "rapidjson/rapidjson.h"

#include <map>
#include <vector>
#include <assert.h>

namespace stonks
{


using StocksMap = std::map< std::string, Stock >;
using DateToIndex = std::map< std::string, uint32_t >;
using IndexToDate = std::map< uint32_t, std::string>;
using PriceHistory = std::vector< Price >;
using PriceHistoryMap = std::map< std::string, PriceHistory >;

class StonksImpl : public Stonks
{
public:
	StonksImpl(void)
	{
		auto database = keyvaluedatabase::KeyValueDatabase::create("d:\\github\\stonks\\stock-market");
		assert(database);
		if ( database )
		{
			printf("Scanning for all valid market dates using AAPL as a baseline refrerence.\n");
			bool ok = database->begin("price.AAPL.");
			assert(ok);
			while ( ok )
			{
				std::string key;
				std::string value;
				ok = database->next(key,value);
				if ( ok )
				{
					const char *date = getDate(key.c_str());
					assert(date);
					if ( date )
					{
						uint32_t index = uint32_t(mDateToIndex.size())+1;
						std::string sdate(date);
						mDateToIndex[sdate] = index;
						mIndexToDate[index] = sdate;
					}
				}
			}
			printf("Processed %d unique market dates.\n", uint32_t(mDateToIndex.size()));
		}
		if ( database )
		{
			printf("Reading price data for all tickers.\n");
			bool ok = database->begin("ticker.");
			assert(ok);
			std::vector< std::string > tickers;
			while ( ok )
			{
				std::string key;
				std::string value;
				ok = database->next(key,value);
				if ( ok )
				{
					const char *ticker = strchr(key.c_str(),'.');
					assert(ticker);
					if ( ticker )
					{
						ticker++;
						if ( strcmp(value.c_str(),"no_price") == 0 )
						{
							printf("Skipping ticker(%s) with no valid price data.\n", ticker );
						}
						else
						{
							tickers.push_back(std::string(ticker));
						}
					}
				}
			}
			for (auto &i:tickers)
			{
				readPriceHistory(i.c_str(),database);
			}
			database->release();
		}
	}

	virtual ~StonksImpl(void)
	{
	}

	virtual const Stock *getStock(const std::string &symbol) const final
	{
		const Stock *ret = nullptr;

		StocksMap::const_iterator found = mStocks.find(symbol);
		if ( found != mStocks.end() )
		{
			ret = &(*found).second;
		}
		return ret;
	}

	virtual void release(void) final
	{
		delete this;
	}

	virtual uint32_t begin(void) final // begin iterating stock symbols, returns the number available.
	{
		mIterator = mStocks.begin();
		return uint32_t(mStocks.size());
	}

	virtual const Stock *next(void) final // goes to the next
	{
		const Stock *ret = nullptr;
		if ( mIterator != mStocks.end() )
		{
			ret = &(*mIterator).second;
			mIterator++;
		}
		return ret;
	}

	const char *getDate(const char *scan) const
	{
		const char *ret = nullptr;

		scan = strchr(scan,'.');
		assert(scan);
		if ( scan )
		{
			scan = strchr(scan+1,'.');
			assert(scan);
			if ( scan )
			{
				ret = scan+1;
			}
		}


		return ret;
	}

	virtual uint32_t dateToIndex(const char *date) const final
	{
		uint32_t ret = 0;

		if ( date )
		{
			DateToIndex::const_iterator found = mDateToIndex.find(std::string(date));
			if ( found != mDateToIndex.end() )
			{
				ret = (*found).second;
			}
		}

		return ret;
	}

	virtual const char *indexToDate(uint32_t index) const final
	{
		const char *ret = nullptr;

		IndexToDate::const_iterator found = mIndexToDate.find(index);
		if ( found != mIndexToDate.end() )
		{
			ret = (*found).second.c_str();
		}

		return ret;
	}

	uint32_t readPriceHistory(const char *ticker,keyvaluedatabase::KeyValueDatabase *database)
	{
		uint32_t ret = 0;

		std::string skipKey = "price." + std::string(ticker);
		std::string prefix = skipKey + std::string(".");
		bool ok = database->begin(prefix.c_str());
		assert(ok);
		if ( ok )
		{
			PriceHistory phistory;
			while ( ok )
			{
				std::string key;
				std::string value;
				ok = database->next(key,value);
				if ( ok )
				{
					Price p;
					p.mPrice = atof(value.c_str());
					if ( key == skipKey )
					{
					}
					else
					{
						const char *date = getDate(key.c_str());
						if ( date )
						{
							p.mDate = std::string(date);
							p.mDateIndex = dateToIndex(date);
							assert(p.mDateIndex);
							phistory.push_back(p);
							ret++;
						}
					}
				}
			}
			mPriceHistoryMap[std::string(ticker)] = phistory;
		}
		printf("Found %d price dates for ticker %s.\n", ret, ticker);

		return ret;
	}

	StocksMap::iterator mIterator;
	StocksMap	mStocks;
	DateToIndex	mDateToIndex;
	IndexToDate mIndexToDate;
	PriceHistoryMap	mPriceHistoryMap;

};

Stonks *Stonks::create(void)
{
	auto ret = new StonksImpl;
	return static_cast< Stonks *>(ret);
}


}
