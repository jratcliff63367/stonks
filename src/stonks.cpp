#include "stonks.h"
#include "InParser.h"
#include "rapidjson/rapidjson.h"
#include <map>

namespace stonks
{

using StocksMap = std::map< std::string, Stock >;

class StonksImpl : public Stonks, public IN_PARSER::InPlaceParserInterface
{
public:
	StonksImpl(void)
	{
		IN_PARSER::InPlaceParser ipp;
		ipp.SetHard(',');
		ipp.SetFile("d:\\tickers.csv");
		ipp.ClearHardSeparator(32);
		ipp.Parse(this);
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

	virtual uint32_t ParseLine(uint32_t lineno, uint32_t argc, const char **argv)   // return TRUE to continue parsing, return FALSE to abort parsing process
	{
		uint32_t ret = 0;

		if ( lineno == 1 )
		{
		}
		else
		{
			if ( argc == 21 )
			{
				const char *symbol = argv[0];
				const char *name = argv[2];
				const char *marketCap = argv[10];
				const char *country = argv[12];
				const char *ipoYear = argv[14];
				const char *volume = argv[16];
				const char *sector = argv[18];
				const char *industry = argv[20];

				Stock s;
				s.mSymbol = std::string(symbol);
				s.mName = std::string(name);
				s.mMarketCap = atof(marketCap);
				s.mCountry = std::string(country);
				s.mIPOYear = atoi(ipoYear);
				s.mVolume = atoi(volume);
				s.mSector = std::string(sector);
				s.mIndustry = std::string(industry);

				mStocks[s.mSymbol] = s;

			}
		}

		return ret;
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


	StocksMap::iterator mIterator;
	StocksMap	mStocks;

};

Stonks *Stonks::create(void)
{
	auto ret = new StonksImpl;
	return static_cast< Stonks *>(ret);
}


}
