// Project1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <map>

using namespace std;

std::string processString(std::string);
void countFrequency(string[], int, int);

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
               std::cerr << "Error reading line from file." << endl;
               break; // Break out of the loop on error
           }

           cout << linesArray[i] << endl;

       }

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
            threads[i] = thread(countFrequency, linesArray, currentLine, endLine);
            cout << "Thread " << i << " spawned" << endl;
        
            std::cout << "Start: " << currentLine << " ";
            std::cout << "End: " << endLine << endl;

            currentLine = endLine;
        }

        // Join threads
        for (int i = 0; i < numThreads; ++i) {
            threads[i].join();
            cout << "Thread " << i << " finished" << endl;
        }

    }
    else {
        cout << "File not opened.";
    }

}

//For now, uses c++ map and multimap to add each line into map and count frequency for output
void countFrequency(string linesArr[], int start, int end) {

    std::map<std::string, int> frequencyMap;
    //Each thread traverse through the array from start to end indicies
    for (int i = start; i < end; i++) {
        // Check if value is in the map
        if (frequencyMap.contains(linesArr[i]))
        {
            // found, increase count
            frequencyMap[linesArr[i]]++;
        }
        else
        {
            // not found, add to the map
            frequencyMap[linesArr[i]] = 1;
        }
    }

    // Put into multimap to sort
    std::multimap<int, std::string> sortedMap;
    std::map<std::string, int>::iterator iter = frequencyMap.begin();
    while (iter != frequencyMap.end())
    {
        sortedMap.insert({ iter->second, iter->first });
        iter++;
    }

    // Print multimap in reverse order
    std::multimap<int, std::string>::iterator sortedIter = sortedMap.end();
    while (sortedIter != sortedMap.begin())
    {
        sortedIter--;
        std::cout << sortedIter->second << " " << sortedIter->first << std::endl;
    }

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
        else if (input[i] >= 'A' && input[i] <= 'Z')
            output += input[i];
        //ignore everything else
    }

    return output;
}