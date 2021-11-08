#include "KeyValueDatabase.h"
#include "leveldb/db.h"

#include <assert.h>
#include <stdio.h>


namespace keyvaluedatabase
{

class KeyValueDatabaseImpl : public KeyValueDatabase
{
public:
	KeyValueDatabaseImpl(void)
	{
		leveldb::DB* db;
		leveldb::Options options;
		options.create_if_missing = true;
		leveldb::Status status = leveldb::DB::Open(options, "C:\\Users\\jratc\\PycharmProjects\\TestProject\\stock-market", &db);
		assert(status.ok());

		leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
		for (it->SeekToFirst(); it->Valid(); it->Next()) 
		{
			printf("%s : %s\n", it->key().ToString().c_str(), 
				                it->value().ToString().c_str());
		}
		assert(it->status().ok());  // Check for any errors found during the scan
		delete it;

		delete db;
	}

	virtual ~KeyValueDatabaseImpl(void)
	{
	}

	virtual void release(void) final
	{
		delete this;
	}
};

KeyValueDatabase *KeyValueDatabase::create(void)
{
	auto ret = new KeyValueDatabaseImpl;
	return static_cast< KeyValueDatabase *>(ret);
}


}
