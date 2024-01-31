#pragma once
#include <string>
#include <atomic>

class KeyValue
{
private:
	std::string key;
	std::atomic<int> value;

public:
	KeyValue();
	KeyValue(std::string, int);

	std::string getKey();
	int getValue();

	void setKey(std::string);
	void setValue(int);

	void increment();
};