#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "leveldb/db.h"
#include "stonks.h"
#include "KeyValueDatabase.h"
#include "Commands.h"

static const double billion = 1000000000;

int main(int /* argc */,const char ** /* argv */)
{
#if 1
	commands::Commands *c = commands::Commands::create();
	while ( !c->pump() )
	{
	}
	c->release();

#else
	keyvaluedatabase::KeyValueDatabase *kvdb = keyvaluedatabase::KeyValueDatabase::create();

	stonks::Stonks *s = stonks::Stonks::create();

	uint32_t count = s->begin();
	printf("Found %d stocks.\n", count );
	uint32_t matchCount=0;
	for (uint32_t i=0; i<count; i++)
	{
		const stonks::Stock *stock = s->next();
		if ( stock )
		{
			if ( stock->mMarketCap >= 10*billion )
			{
				printf("%s : %s\n", stock->mSymbol.c_str(), stock->mName.c_str());
				matchCount++;
			}
		}
	}
	printf("%d stocks match.\n", matchCount);

	s->release();
	kvdb->release();
#endif

	return 0;
}
