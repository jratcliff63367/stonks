#include "Commands.h"
#include "GetArgs.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string> 

namespace commands
{

class CommandsImpl : public Commands
{
public:
	CommandsImpl(void)
	{
	}

	virtual ~CommandsImpl(void)
	{
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
			const char *cmd = argv[0];
			if ( strcmp(cmd,"bye") == 0 )
			{
				ret = true;
			}
			else if ( strcmp(cmd,"help") == 0 )
			{
				printf("There is no help for you.\n");
			}
			else
			{
				printf("Unknown command: %s\n", cmd );
			}
		}

		return ret;
	}

	virtual void release(void) final
	{
		delete this;
	}


};

Commands *Commands::create(void)
{
	auto ret = new CommandsImpl;
	return static_cast< Commands *>(ret);
}


}

