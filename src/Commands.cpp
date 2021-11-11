#include "Commands.h"
#include "GetArgs.h"
#include "stonks.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string> 

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

	stonks::Stonks	*mStonks{nullptr};
	CommandTypeMap mCommands;
};

Commands *Commands::create(void)
{
	auto ret = new CommandsImpl;
	return static_cast< Commands *>(ret);
}


}

