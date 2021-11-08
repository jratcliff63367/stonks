#pragma once

namespace keyvaluedatabase
{

class KeyValueDatabase
{
public:
	static KeyValueDatabase *create(void);

	virtual void release(void) = 0;
protected:
	virtual ~KeyValueDatabase(void)
	{
	}
};

}
