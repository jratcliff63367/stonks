#pragma once

#include <stdint.h>

namespace commands
{

enum class CommandType : uint32_t
{
	create,
	remove,
	_delete,
	show,
	account,
	help,
	bye,
	tickers,
	last
};

class Commands
{
public:
	static Commands *create(void);

	virtual bool pump(void) = 0;

	virtual CommandType getCommandType(const char *str) const = 0;

	virtual void release(void) = 0;
protected:
	virtual ~Commands(void)
	{
	} 

};

}
