#pragma once
#include "KeyValue.h"

class HashMap
{
private:
	const int INITIAL_CAPACITY = 16;
	const float LOAD_FACTOR = 0.75;

	int size, capacity;
	KeyValue* array;

	void checkResize();
	int hash(std::string);
	int find(std::string);

	std::mutex mutex;

public:
	HashMap();
	~HashMap();

	void put(std::string, int);
	int get(std::string);
	void increment(std::string);
	bool contains(std::string);

	KeyValue* getAll();
};