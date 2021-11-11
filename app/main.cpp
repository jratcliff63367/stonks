#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "leveldb/db.h"
#include "Commands.h"

int main(int /* argc */,const char ** /* argv */)
{
	commands::Commands *c = commands::Commands::create();
	while ( !c->pump() )
	{
	}
	c->release();
	return 0;
}
