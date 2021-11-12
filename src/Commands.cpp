#include "Commands.h"
#include "GetArgs.h"
#include "stonks.h"
#include "sutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string> 
#include <assert.h>
#include <math.h>
#include <float.h>

#include <unordered_map>

namespace commands
{

using CommandTypeMap = std::unordered_map< std::string, CommandType >;

class CommandsImpl : public Commands
{
public:
	CommandsImpl(void)
	{
		mCommands["create"] = CommandType::create;
		mCommands["remove"] = CommandType::remove;
		mCommands["delete"] = CommandType::_delete;
		mCommands["show"] = CommandType::show;
		mCommands["account"] = CommandType::account;
		mCommands["help"] = CommandType::help;
		mCommands["bye"] = CommandType::bye;
		mCommands["tickers"] = CommandType::tickers;
		mCommands["backup"] = CommandType::backup;
		mCommands["restore"] = CommandType::restore;

		mCommands["marketcap"] = CommandType::marketcap;
		mCommands["mcap"] = CommandType::marketcap;

		mCommands["pe"] = CommandType::pe;

		mCommands["dividend"] = CommandType::dividend;
		mCommands["div"] = CommandType::dividend;

		mCommands["filter"] = CommandType::filter;
		mCommands["f"] = CommandType::filter;

		mCommands["sectors"] = CommandType::sectors;
		mCommands["industry"] = CommandType::industry;
		mCommands["industries"] = CommandType::industry;

		mCommands["volatility"] = CommandType::volatility;

		mStonks = stonks::Stonks::create();
	}

	virtual ~CommandsImpl(void)
	{
		if ( mStonks )
		{
			mStonks->release();
		}
	}

	virtual bool pump(void) final
	{
		bool ret = false;

		printf("Enter a command. Type 'help' for help. Type 'bye' to exit.\n");

		while ( !ret )
		{
			std::string str;
			std::getline(std::cin, str); 
			GetArgs g;
			uint32_t argc;
			const char **argv = g.getArgs(str.c_str(),argc);
			if ( argv )
			{
				ret = processCommand(argc,argv);
			}

		}

		return ret;
	}

	bool processCommand(uint32_t argc,const char **argv)
	{
		bool ret = false;

		if ( argc >= 1 )
		{
			CommandType c = getCommandType(argv[0]);
			switch ( c )
			{
				case CommandType::bye:
					ret = true;
					break;
				case CommandType::help:
					printf("There is no help for you.\n");
					break;
				case CommandType::last:
					printf("Unknown command: %s\n", argv[0]);
					break;
				case CommandType::backup:
					mStonks->backup();
					break;
				case CommandType::marketcap:
					if ( argc >= 2 )
					{
						mMarketCap = atof(argv[1])*1000000000.0;
						marketCap();
					}
					else
					{
						printf("Usage: marketcap <billions>\n");
					}
					break;
				case CommandType::pe:
					if ( argc >= 2 )
					{
						mPE = atof(argv[1]);
						peRatio();
					}
					else
					{
						printf("Usage: pe <value>\n");
					}
					break;
				case CommandType::dividend:
					if ( argc >= 2 )
					{
						mDividend = atof(argv[1])/100.0;
						dividend();
					}
					else
					{
						printf("Usage: dividend <percent>\n");
					}
					break;
				case CommandType::volatility:
					if ( argc >= 2 )
					{
						for (uint32_t i=1; i<argc; i++)
						{
							volatilityReport(argv[i]);
						}
					}
					else
					{
						printf("Usage: volatility <stock1> ...\n");
					}
					break;
				case CommandType::filter:
					filterStocks();
					break;
				case CommandType::sectors:
					mStonks->showSectors();
					break;
				case CommandType::industry:
					mStonks->showIndustries();
					break;
				default:
					printf("Command: %s not yet implemented.\n", argv[0]);
					break;
			}
		}

		return ret;
	}

	virtual void release(void) final
	{
		delete this;
	}

	virtual CommandType getCommandType(const char *str) const final
	{
		CommandType ret = CommandType::last;

		CommandTypeMap::const_iterator found = mCommands.find(std::string(str));
		if ( found != mCommands.end() )
		{
			ret = (*found).second;
		}

		return ret;
	}

	void marketCap(void)
	{
		uint32_t countTotal=0;
		uint32_t countGreater=0;
		uint32_t countLesser=0;
		uint32_t countUndefined=0;
		uint32_t count = mStonks->begin();
		for (uint32_t i=0; i<count; i++)
		{
			const stonks::Stock *s = mStonks->next();
			assert(s);
			if ( s )
			{
				countTotal++;
				if ( s->mMarketCapitalization == 0 )
				{
					countUndefined++;
				}
				else if ( s->mMarketCapitalization >= mMarketCap )
				{
					countGreater++;
				}
				else if ( s->mMarketCapitalization < mMarketCap )
				{
					countLesser++;
				}
			}
		}
		printf("TotalStockCount: %-20s\n", sutil::formatNumber(countTotal));
		printf("GreaterCount:    %-20s\n", sutil::formatNumber(countGreater));
		printf("LessCount:       %-20s\n", sutil::formatNumber(countLesser));
		printf("UndfinedCount:   %-20s\n", sutil::formatNumber(countUndefined));
	}

	void peRatio(void)
	{
		uint32_t countTotal=0;
		uint32_t countGreater=0;
		uint32_t countLesser=0;
		uint32_t countUndefined=0;
		uint32_t count = mStonks->begin();
		for (uint32_t i=0; i<count; i++)
		{
			const stonks::Stock *s = mStonks->next();
			assert(s);
			if ( s )
			{
				countTotal++;
				if ( s->mPERatio == 0 )
				{
					countUndefined++;
				}
				else if ( s->mPERatio >= mPE )
				{
					countGreater++;
				}
				else if ( s->mPERatio < mPE )
				{
					countLesser++;
				}
			}
		}
		printf("TotalStockCount: %-20s\n", sutil::formatNumber(countTotal));
		printf("GreaterCount:    %-20s\n", sutil::formatNumber(countGreater));
		printf("LessCount:       %-20s\n", sutil::formatNumber(countLesser));
		printf("UndfinedCount:   %-20s\n", sutil::formatNumber(countUndefined));
	}

	void dividend(void)
	{
		uint32_t countTotal=0;
		uint32_t countGreater=0;
		uint32_t countLesser=0;
		uint32_t countUndefined=0;
		uint32_t count = mStonks->begin();
		for (uint32_t i=0; i<count; i++)
		{
			const stonks::Stock *s = mStonks->next();
			assert(s);
			if ( s )
			{
				countTotal++;
				if ( s->mDividendYield == 0 )
				{
					countUndefined++;
				}
				else if ( s->mDividendYield >= mDividend )
				{
					countGreater++;
				}
				else if ( s->mDividendYield < mDividend )
				{
					countLesser++;
				}
			}
		}
		printf("TotalStockCount: %-20s\n", sutil::formatNumber(countTotal));
		printf("GreaterCount:    %-20s\n", sutil::formatNumber(countGreater));
		printf("LessCount:       %-20s\n", sutil::formatNumber(countLesser));
		printf("UndfinedCount:   %-20s\n", sutil::formatNumber(countUndefined));
	}

	void filterStocks(void)
	{
		FILE *fph = fopen("d:\\github\\stonks\\scripts\\filter.csv","wb");
		uint32_t found = 0;
		uint32_t count = mStonks->begin();
		for (uint32_t i=0; i<count; i++)
		{
			const stonks::Stock *s = mStonks->next();
			assert(s);
			if ( s )
			{
				bool ok = true;
				if ( mMarketCap )
				{
					if ( s->mMarketCapitalization < mMarketCap )
					{
						ok = false;
					}
				}
				if ( ok && mPE )
				{
					if ( s->mPERatio == 0 || s->mPERatio > mPE )
					{
						ok = false;
					}
				}
				if ( ok && mDividend )
				{
					if ( s->mDividendYield == 0 || s->mDividendYield < mDividend )
					{
						ok = false;
					}
				}
				if ( ok )
				{
					if ( fph )
					{
						fprintf(fph,"%s\n", s->mSymbol.c_str() );
					}
					printf("%8s : MCAP:%0.2f PE:%0.2f DIV:%0.2f%% : Sector:%s : Name:%s\n", 
						s->mSymbol.c_str(),
						s->mMarketCapitalization/1000000000.0,
						s->mPERatio,
						s->mDividendYield*100,
						s->mSector.c_str(),
						s->mName.c_str());
					found++;
				}
			}
		}
		printf("Found %d matching stocks.\n", found);
		if ( fph )
		{
			fclose(fph);
		}
	}

	void computeVolatility(const char *year,
						   const std::vector<double> &prices,
						   const std::vector<double> &percentChange,
						   const std::vector<double> &absolutePercentChange,
						   double &totalGain,
						   double &totalProfit)
	{
		uint32_t crossCount = 0;
		double lastPrice = prices[0];
		bool searchingForPriceDrop=true;
		size_t count = prices.size();
		#define BUY_PERCENT -3.0
		#define SELL_PERCENT 3.0
		double profit = 0;
		for (size_t i=1; i<count; i++)
		{
			double price = prices[i];
			double diff = price - lastPrice;
			double percent = (diff*100.0) / lastPrice;
			if ( searchingForPriceDrop )
			{
				if ( percent <= BUY_PERCENT )
				{
					searchingForPriceDrop = false;
					//printf("Price drop from %0.2f to %0.2f so this would be a buy.\n", lastPrice, price );
					lastPrice = price;
				}
				else if ( price > lastPrice )
				{
					lastPrice = price;
					//printf("Price increased to:%0.2f\n", price );
				}
			}
			else 
			{
				if ( percent >= SELL_PERCENT )
				{
					//printf("Price raised to %0.2f from our last buy price of %0.2f time to take profit\n", price, lastPrice);
					profit+=diff;
					searchingForPriceDrop = true;
					lastPrice = price;
					crossCount++;
				}
			}
		}
		double priceBegin = prices[0];
		double priceEnd = prices[count-1];
		if ( profit == 0 )
		{
			profit = priceEnd - lastPrice;
		}

		double gain = priceEnd - priceBegin;
		(absolutePercentChange);
		(percentChange);

		totalGain+=gain;
		totalProfit+=profit;

		if ( gain > profit )
		{
			printf("[%s] HOLD WINS:%0.2f to %0.2f : CrossCount:%d\n",year, gain, profit, crossCount);
		}
		else
		{
			printf("[%s] TRADE WINS:%0.2f to %0.2f : CrossCount:%d\n",year, profit, gain, crossCount);
		}
	}

	void volatilityReport(const char *symbol)
	{
		char temp[512];
		strncpy(temp,symbol,sizeof(temp));
		_strupr(temp);
		auto s = mStonks->find(temp);
		if ( s )
		{
			double totalGain=0;
			double totalProfit=0;
			std::vector< double > prices;
			std::vector< double > percentChange;
			std::vector< double> absolutePercentChange;
			std::string str;
			const char *lastYear = nullptr;
			double lastPrice = 0;
			for (auto &i:s->mHistory)
			{
				const char *year = i.mDate.c_str();
				if ( lastYear && strncmp(lastYear,year,4) != 0 )
				{
					computeVolatility(lastYear,prices,percentChange,absolutePercentChange,totalGain,totalProfit);
					lastYear = nullptr;
				}
				if ( lastYear )
				{
					double diff = i.mPrice - lastPrice;
					double adiff = abs(diff);
					double percent = (diff*100.0) / lastPrice;
					double apercent = (adiff*100.0) / lastPrice;
					percentChange.push_back(percent);
					absolutePercentChange.push_back(apercent);
					prices.push_back(i.mPrice);
				}
				else
				{
					percentChange.clear();
					absolutePercentChange.clear();
					prices.clear();
					str = i.mDate;
					lastYear = str.c_str();
					lastPrice = i.mPrice;
				}
			}
			if ( lastYear )
			{
				computeVolatility(lastYear,prices,percentChange,absolutePercentChange,totalGain,totalProfit);
			}
			if ( totalGain >= totalProfit )
			{
				printf("HOLD WINS: %0.2f to %0.2f\n", totalGain, totalProfit);
			}
			else
			{
				printf("TRADE WINS: %0.2f to %0.2f\n", totalProfit, totalGain);
			}
		}
		else
		{
			printf("Stock symbol:%s not found\n", symbol);
		}
	}

	double			mMarketCap{0}; // 
	double			mPE{0};
	double			mDividend{0};
	stonks::Stonks	*mStonks{nullptr};
	CommandTypeMap mCommands;
};

Commands *Commands::create(void)
{
	auto ret = new CommandsImpl;
	return static_cast< Commands *>(ret);
}


}

