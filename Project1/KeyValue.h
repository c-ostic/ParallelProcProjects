#pragma once
#include <string>
#include <mutex>

class KeyValue
{
private:
	std::string key;
	int value;

	std::mutex mutex;

public:
	KeyValue(std::string, int);

	std::string getKey();
	int getValue();

	void increment();
};