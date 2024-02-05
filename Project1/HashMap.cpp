#include <iostream>
#include "HashMap.h"

HashMap::HashMap()
{
	this->size = 0;
	this->capacity = this->INITIAL_CAPACITY;

	array = new KeyValue[capacity];
}

HashMap::~HashMap()
{
	delete[] array;
}

int HashMap::getSize()
{
	return size;
}

/*
* Generate a hash for a given key
*/
int HashMap::hash(std::string key)
{
	// Can't hash a key with no characters
	if (key.empty())
	{
		return 0;
	}

	int fullHash = key[0];
	int p = 31;
	int pow = 1;

	for (int i = 1; i < key.length(); i++)
	{
		// Add to the hash
		fullHash += key[i] * pow;
		pow = pow * p % INT16_MAX;
	}

	// Reduce full hash to fit within the current capacity
	return fullHash % capacity;
}

/*
* Checks to see if the array needs to be resized
* If it does, doubles the size of the array and moves every value over
*/
void HashMap::checkResize()
{
	// Only resize if the load factor has been reached or exceeded
	if ((float)size / capacity > LOAD_FACTOR)
	{
		mutex.lock();

		int oldCapacity = capacity;

		// Double capacity
		capacity *= 2;

		KeyValue* oldArray = array;
		array = new KeyValue[capacity];

		// Iterate through all KeyValues and put any that aren't empty into the newly created array
		for (int i = 0; i < oldCapacity; i++)
		{
			if (!oldArray[i].getKey().empty())
			{
				int index = hash(oldArray[i].getKey());

				// Search until the key or an empty space is found in the array
				std::string currentKey = array[index].getKey();
				while (!currentKey.empty() && oldArray[i].getKey().compare(currentKey) != 0)
				{
					index = (index + 1) % capacity;
					currentKey = array[index].getKey();
				}

				array[index].setKey(oldArray[i].getKey());
				array[index].setValue(oldArray[i].getValue());
			}
		}

		delete[] oldArray;

		mutex.unlock();
	}
}

/*
* Tries to find the location of key in the array
* If found, returns the index
* If not found, returns the index of the first available empty space as a negative number - 1
*/
int HashMap::find(std::string key)
{
	int hashCode = hash(key);

	// Search until the key or an empty space is found in the array
	std::string currentKey = array[hashCode].getKey();
	while (!currentKey.empty() && key.compare(currentKey) != 0)
	{
		hashCode = (hashCode + 1) % capacity;
		currentKey = array[hashCode].getKey();
	}

	// If an empty space was found, return a negative number of that index
	if (currentKey.empty())
	{
		return hashCode * -1 - 1;
	}
	// Otherwise return the index where the key was found
	else
	{
		return hashCode;
	}
}

/*
* Attempts to put a new key into the array
* If the key already exists, just increment the counter in KeyValue
* Otherwise, add the key to the map at the first empty space (found by find() method)
*/
void HashMap::put(std::string key, int value)
{
	if (key.empty()) 
	{
		return;
	}

	mutex.lock();
	int index = find(key);

	// If the index is positive, the key is already in the array, so just increment
	if (index >= 0)
	{
		array[index].increment();
	}
	else
	{
		// This is a new key, so it must be added to the array
		// Return index to a positive number
		
		index = (index + 1) * -1;
		array[index].setKey(key);
		array[index].setValue(value);
		size++;

	}
	mutex.unlock();

	checkResize();
}

/*
* Returns the value associated with the key
* If the key doesnt exist in the array, return 0
*/
int HashMap::get(std::string key)
{
	mutex.lock_shared();
	int index = find(key);
	mutex.unlock_shared();

	if (index >= 0)
	{
		mutex.lock_shared();
		int val = array[index].getValue();
		mutex.unlock_shared();

		return val;
	}
	else
	{
		return 0;
	}
}

/*
* Tries to increment the current key
* If the key doesn't exist, call put instead
*/
void HashMap::increment(std::string key)
{
	mutex.lock_shared();
	int index = find(key);

	if (index >= 0)
	{
		array[index].increment();
		mutex.unlock_shared();
	}
	else
	{
		mutex.unlock_shared();
		put(key, 1);
	}
}

/*
* Returns true if the key is found within the array
*/
bool HashMap::contains(std::string key)
{
	mutex.lock_shared();
	int index = find(key);
	mutex.unlock_shared();

	return index >= 0;
}

/*
* Returns an array of all the key/value pairs in the hashmap array
*/
KeyValue* HashMap::getAll()
{
	KeyValue* allKeyValues = new KeyValue[size];
	int allIndex = 0;

	// Iterate through all KeyValues and put any that aren't empty into the newly created array
	for (int mapIndex = 0; mapIndex < capacity; mapIndex++)
	{
		if (!array[mapIndex].getKey().empty())
		{
			allKeyValues[allIndex].setKey(array[mapIndex].getKey());
			allKeyValues[allIndex].setValue(array[mapIndex].getValue());
			allIndex++;
		}
	}

	return allKeyValues;
}