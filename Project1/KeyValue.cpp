#include "KeyValue.h"

KeyValue::KeyValue(std::string key, int value)
{
	this->key = key;
	this->value = value;
}

std::string KeyValue::getKey()
{
	return this->key;
}

int KeyValue::getValue()
{
	return this->value;
}

void KeyValue::increment()
{
	mutex.lock();
	this->value++;
	mutex.unlock();
}