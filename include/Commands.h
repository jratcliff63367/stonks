#pragma once

namespace commands
{

class Commands
{
public:
	static Commands *create(void);

	virtual bool pump(void) = 0;

	virtual void release(void) = 0;
protected:
	virtual ~Commands(void)
	{
	}

};

}
