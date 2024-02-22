#pragma once
#include <string>

class KeyValue
{
private:
	std::string key;
	int value;

public:
	KeyValue();
	KeyValue(std::string, int);

	std::string getKey();
	int getValue();

	void setKey(std::string);
	void setValue(int);

	void increment();
};