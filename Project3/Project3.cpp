// Project3Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>

bool readFile(std::string filename, char*** data, int* rows, int* columns);

int main()
{
    MPI_Init(NULL, NULL);

    // read in both files
    char** input = nullptr;
    char** pattern = nullptr;
    int inputRows, inputColumns, patternRows, patternColumns;

    if (!readFile("input1.txt", &input, &inputRows, &inputColumns) ||
        !readFile("pattern1.txt", &pattern, &patternRows, &patternColumns))
    {
        return 1;
    }

    std::cout << "Input: " << inputRows << " " << inputColumns << std::endl;
    for (int i = 0; i < inputRows; i++)
    {
        for (int j = 0; j < inputColumns; j++)
        {
            std::cout << input[i][j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Pattern: " << patternRows << " " << patternColumns << std::endl;
    for (int i = 0; i < patternRows; i++)
    {
        for (int j = 0; j < patternColumns; j++)
        {
            std::cout << pattern[i][j];
        }
        std::cout << std::endl;
    }

    // create structure that holds coordinate pairs
    // inputRows * inputColumns is the maximum amount of possible patterns
    int** coords = new int* [inputRows * inputColumns];
    int count = 0;

    // loop through every space in input
    for (int k = 0; k < inputRows * inputColumns; k++)
    {
        int i = k / inputColumns;
        int j = k % inputColumns;

        // check if pattern is at this position
        bool patternFound = true;

        // loop through each character in the pattern, stop as soon as pattern is out of bounds or it doesn't match
        for (int currentRow = 0; currentRow < patternRows && patternFound; currentRow++)
        {
            for (int currentColumn = 0; currentColumn < patternColumns && patternFound; currentColumn++)
            {
                if (i + currentRow >= inputRows || j + currentColumn >= inputColumns || // out of bounds
                    input[i + currentRow][j + currentColumn] != pattern[currentRow][currentColumn]) // doesn't match pattern
                {
                    patternFound = false;
                }
            }
        }

        // if the pattern is found, add it to the list
        if (patternFound)
        {
            int* pair = new int[2];
            pair[0] = i + 1;
            pair[1] = j + 1;
            coords[count] = pair;
            count++;
        }
    }

    // Print the pattern locations
    std::cout << std::endl;
    std::cout << "Found patterns: " << count << std::endl;
    for (int i = 0; i < count; i++)
    {
        std::cout << coords[i][0] << ", " << coords[i][1] << std::endl;
    }

    MPI_Finalize();
}

bool readFile(std::string filename, char*** data, int* rows, int* columns)
{
    std::ifstream file(filename);

    *rows = 0;
    *columns = 0;

    //Open the file and read line by line into array
    if (file.is_open())
    {
        // Get the total number of lines in the file by counting the number of newline characters
        int totalLines = std::count(std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(), '\n') + 1;

        // Initiailize the array with the total number of lines
        char** arr = new char* [totalLines];
        *rows = totalLines;

        //Go back to the start of the file
        file.clear();
        file.seekg(0, std::ios::beg);

        for (int i = 0; i < totalLines; i++)
        {
            std::string line;
            if (!std::getline(file, line, '\n'))
            {
                break; // Break out of the loop on error
            }

            *columns = line.length();
            arr[i] = new char[line.length()];

            for (int j = 0; j < line.length(); j++)
            {
                arr[i][j] = line[j];
            }
        }

        *data = arr;

        return true;
    }
    else
    {
        std::cout << filename << " not opened" << std::endl;
        return false;
    }
}
