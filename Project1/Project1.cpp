// Project1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include "HashMap.h"

using namespace std;

std::string processString(std::string);
void countFrequency(string*, int, int, int);
void selectionSort(KeyValue*);
void swap(KeyValue*, KeyValue*);
void mergeSort(KeyValue*, int, int);
void merge(KeyValue*, int, int, int);

HashMap* hashmap = new HashMap();

int main()
{
    std::string filename;
    std::cout << "Enter the filename: ";
    std::cin >> filename;

    int numThreads;
    std::cout << "Enter the number of threads: ";
    std::cin >> numThreads;
    std::thread* threads = new thread[numThreads];

    std::ifstream file(filename);

    std::ofstream outputFile("output.txt");

    chrono::time_point<chrono::system_clock> startTotal, endTotal, startFile, endFile, startCount, endCount, startSort, endSort;

    startTotal = chrono::system_clock::now();
    startFile = chrono::system_clock::now();

    //Open the file and read line by line into array
    if (file.is_open()) {

        // Get the total number of lines in the file by counting the number of newline characters
        int totalLines = std::count(std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(), '\n') + 1;

        cout << "Total lines in file: " << totalLines << endl;

       //Array to store lines in file
       std::string* linesArray = new std::string[totalLines];

        //Go back to the start of the file
       file.clear();
       file.seekg(0, std::ios::beg);

       for (int i = 0; i < totalLines; i++) {            
           
           if (!std::getline(file, linesArray[i], '\n')) {
               break; // Break out of the loop on error
           }

           // cout << linesArray[i] << endl;

       }
       endFile = chrono::system_clock::now();
       startCount = chrono::system_clock::now();

        // Calculate the number of lines each thread should process
        // Handles uneven division and adds the extra lines evenly 
        int linesPerThread = totalLines / numThreads;
        int extraLines = totalLines % numThreads;
        int currentLine = 0;

        for (int i = 0; i < numThreads; i++)
        {
            int endLine = currentLine + linesPerThread;

            if (i < extraLines)
            {
                endLine++;
            }
            //Spawn threads
            threads[i] = thread(countFrequency, linesArray, currentLine, endLine, i);
            //cout << "Thread " << i << " spawned" << endl;
        
            //std::cout << "Start: " << currentLine << " ";
            //std::cout << "End: " << endLine << endl;

            currentLine = endLine;
        }

        // Join threads
        for (int i = 0; i < numThreads; ++i) {
            threads[i].join();
            //cout << "Thread " << i << " finished" << endl;
        }

        endCount = chrono::system_clock::now();
        startSort = chrono::system_clock::now();
        
        KeyValue* words = hashmap->getAll();
        int start = 0;
        int end = hashmap->getSize() - 1;
        //selectionSort(words);
        mergeSort(words, start, end);

        endSort = chrono::system_clock::now();
        endTotal = chrono::system_clock::now();

        for (int i = 0; i < hashmap->getSize(); i++) {
            outputFile << words[i].getKey() << " " << words[i].getValue() << endl;
        }

        chrono::duration<double> elapsedTotal, elapsedFile, elapsedCount, elapsedSort;
        elapsedTotal = endTotal - startTotal;
        elapsedFile = endFile - startFile;
        elapsedCount = endCount - startCount;
        elapsedSort = endSort - startSort;

        cout << "Elapsed time file reading: " << elapsedFile.count() << endl;
        cout << "Elapsed time word count: " << elapsedCount.count() << endl;
        cout << "Elapsed time sort: " << elapsedSort.count() << endl;
        cout << "Elapsed time total: " << elapsedTotal.count() << endl;
    }
    else {
        cout << "File not opened.";
    }

}

//Selection sort implementation for our HashMap
//https://www.geeksforgeeks.org/selection-sort/
void selectionSort(KeyValue* words) {

    int i, j, max_index;

    for (i = 0; i < hashmap->getSize(); i++) {

        // Find the max element in unsorted array 
        max_index = i;
        for (j = i + 1; j < hashmap->getSize(); j++) {
            if (words[j].getValue() > words[max_index].getValue())
                max_index = j;
        }

        // Swap the found maximum element with the first element 
        if (max_index != i)
            swap(&words[max_index], &words[i]);
    }
}
void swap(KeyValue* pair1, KeyValue* pair2) {
    KeyValue* temp = new KeyValue(pair1->getKey(), pair1->getValue());
    pair1->setKey(pair2->getKey());
    pair1->setValue(pair2->getValue());

    pair2->setKey(temp->getKey());
    pair2-> setValue(temp->getValue());

}

//Start is for left index and end is right index of the sub-array of arr to be sorted
//https://www.geeksforgeeks.org/merge-sort/
void mergeSort(KeyValue* array, int const start, int const end)
{
    if (start >= end)
        return;

    int mid = start + (end - start) / 2;
    mergeSort(array, start, mid);
    mergeSort(array, mid + 1, end);
    merge(array, start, mid, end);
}

// Merges two subarrays of array[].
// First subarray is arr[begin..mid]
// Second subarray is arr[mid+1..end]
static void merge(KeyValue* array, int const left, int const mid, int const right)
{
    int const subArrayOne = mid - left + 1;
    int const subArrayTwo = right - mid;

    // Create temp arrays
    KeyValue* leftArray = new KeyValue[subArrayOne];
    KeyValue* rightArray = new KeyValue[subArrayTwo];

    // Copy data to temp arrays leftArray[] and rightArray[]
    for (int i = 0; i < subArrayOne; i++) {
        leftArray[i].setKey(array[left + i].getKey());
        leftArray[i].setValue(array[left + i].getValue());
    }

    for (int j = 0; j < subArrayTwo; j++) {
        rightArray[j].setKey(array[mid + 1 + j].getKey());
        rightArray[j].setValue(array[mid + 1 + j].getValue());
    }


    int indexOfSubArrayOne = 0;
    int indexOfSubArrayTwo = 0;
    int indexOfMergedArray = left;

    // Merge the temp arrays back into array[left..right]
    while (indexOfSubArrayOne < subArrayOne && indexOfSubArrayTwo < subArrayTwo) {
        if (leftArray[indexOfSubArrayOne].getValue() >= rightArray[indexOfSubArrayTwo].getValue()) {
            array[indexOfMergedArray].setKey(leftArray[indexOfSubArrayOne].getKey());
            array[indexOfMergedArray].setValue(leftArray[indexOfSubArrayOne].getValue());
            indexOfSubArrayOne++;
        }
        else {
            array[indexOfMergedArray].setKey(rightArray[indexOfSubArrayTwo].getKey());
            array[indexOfMergedArray].setValue(rightArray[indexOfSubArrayTwo].getValue());
            indexOfSubArrayTwo++;
        }
        indexOfMergedArray++;
    }

    // Copy the remaining elements of
    // left[], if there are any
    while (indexOfSubArrayOne < subArrayOne) {
        array[indexOfMergedArray].setKey(leftArray[indexOfSubArrayOne].getKey());
        array[indexOfMergedArray].setValue(leftArray[indexOfSubArrayOne].getValue());
        indexOfSubArrayOne++;
        indexOfMergedArray++;
    }

    // Copy the remaining elements of
    // right[], if there are any
    while (indexOfSubArrayTwo < subArrayTwo) {
        array[indexOfMergedArray].setKey(rightArray[indexOfSubArrayTwo].getKey());
        array[indexOfMergedArray].setValue(rightArray[indexOfSubArrayTwo].getValue());
        indexOfSubArrayTwo++;
        indexOfMergedArray++;
    }
    delete[] leftArray;
    delete[] rightArray;
}

void countFrequency(string* linesArr, int start, int end, int threadNum) {

    HashMap* threadmap = new HashMap();

    //Each thread traverse through the array from start to end indicies
    for (int i = start; i < end; i++) {
        // Check if value is in the map
        int position = 0;
        string word;
        while ((position = linesArr[i].find(' ')) != string::npos) {

            word = linesArr[i].substr(0, position);
            word = processString(word);

            if (threadmap->contains(word))
            {
                // found, increase count
                threadmap->increment(word);
            }
            else
            {
                // not found, add to the map
                threadmap->put(word, 1);
            }

            linesArr[i].erase(0, position + 1);
        }
        //The last word in the line doesn't have a space after it
        //And since each word is erased, the element of linesArr will just be the last word
        word = linesArr[i];
        word = processString(word);
        
        if (threadmap->contains(word))
        {
            // found, increase count
            threadmap->increment(word);
        }
        else
        {
            // not found, add to the map
            threadmap->put(word, 1);
        }
    }

    KeyValue* words = threadmap->getAll();
    for (int i = 0; i < threadmap->getSize(); i++)
    {
        if (hashmap->contains(words[i].getKey()))
        {
            // found, add to map value
            hashmap->addTo(words[i].getKey(), words[i].getValue());
        }
        else
        {
            // not found, add to the map
            hashmap->put(words[i].getKey(), words[i].getValue());
        }
    }

    //cout << "Thread " << threadNum << " finished" << endl;

}

// Convert strings to all uppercase and ignore all other characters
int toUpperDiff = 'a' - 'A';
std::string processString(std::string input)
{
    std::string output;

    for (int i = 0; i < input.length(); i++)
    {
        if (input[i] >= 'a' && input[i] <= 'z')
            output += input[i] - toUpperDiff;
        else if ((input[i] >= 'A' && input[i] <= 'Z') || input[i] == '-' || input[i] == '\'')
            output += input[i];
        //ignore everything else
    }

    return output;
}