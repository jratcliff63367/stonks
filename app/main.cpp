#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "leveldb/db.h"
#include "Commands.h"
#include "WakeupThread.h"
#include "InputLine.h"
#include "positions.h"

int main(int /* argc */,const char ** /* argv */)
{
#if 1
	positions::Positions *p = positions::Positions::create();
	p->buildPositions();
	p->buildDividends();
	p->saveResults();
	p->release();
#else
	commands::Commands *c = commands::Commands::create();

	wakeupthread::WakeupThread *wt = wakeupthread::WakeupThread::create();
	inputline::InputLine *il = inputline::InputLine::create(wt);


	bool isExit = false;
	while ( !isExit )
	{
		wt->goToSleep(1000);
		const char *data = il->getInputLine();
		if ( data )
		{
			isExit = c->processInput(data);
		}
	}


	c->release();
	il->release();
	wt->release();
#endif
	return 0;
}
