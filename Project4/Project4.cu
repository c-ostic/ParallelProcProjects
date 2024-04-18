
#include <iostream>
#include <fstream>
#include <string>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"

using namespace std;

bool readFile(std::string filename, char** data, int* rows, int* columns);
__device__ bool checkForPattern(char* input, char* pattern, int inputRows, int inputColumns, int patternRows, int patternColumns, int i, int j);

// CUDA kernel for pattern matching
__global__ void patternMatchingKernel(char* input, int inputRows, int inputColumns, char* pattern, int patternRows, int patternColumns, int* resultCoords, int maxCoordsSize) {
    int k = blockIdx.x * blockDim.x + threadIdx.x;

    int i = k / inputColumns;
    int j = k % inputColumns;

    // Perform pattern matching using the checkForPattern function
    if (i < inputRows && j < inputColumns)
    {
        bool patternFound = checkForPattern(input, pattern, inputRows, inputColumns, patternRows, patternColumns, i, j);

        // Calculate index for resultCoords array
        int index = i * inputColumns + j;
        if (patternFound)
        {
            resultCoords[index * 2] = i;
            resultCoords[index * 2 + 1] = j;
        }
        else
        {
            resultCoords[index * 2] = -1;
            resultCoords[index * 2 + 1] = -1;
        }
    }
}

int main(int argc, char* argv[]) {

    std::string inputFilename, patternFilename;

    // C++ makes the first argument the name of the program, so two additional arguments make 3
    if (argc != 3)
    {
        cout << "Program accepts two arguments: <input filename> and <pattern filename>" << endl;
        return 1;
    }

    inputFilename = argv[1];
    patternFilename = argv[2];

    // read in both files
    char* input = nullptr;
    char* pattern = nullptr;
    int inputRows, inputColumns, patternRows, patternColumns;

    if (!readFile(inputFilename, &input, &inputRows, &inputColumns) ||
        !readFile(patternFilename, &pattern, &patternRows, &patternColumns))
    {
        return 1;
    }

    for (int i = 0; i < inputColumns * inputRows; i++)
    {
        cout << input[i];
    }

    int maxCoordsSize = inputRows * inputColumns * 2;

    // Allocate device memory for input, pattern, and result coordinates
    char* d_input;
    char* d_pattern;
    int* d_resultCoords;

    cudaMalloc(&d_input, inputRows * inputColumns * sizeof(char));
    cudaMalloc(&d_pattern, patternRows * patternColumns * sizeof(char));
    cudaMalloc(&d_resultCoords, maxCoordsSize * sizeof(int));

    // Copy input and pattern data from host to device
    cudaMemcpy(d_input, input, inputRows * inputColumns * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(d_pattern, pattern, patternRows * patternColumns * sizeof(char), cudaMemcpyHostToDevice);

    // Launch CUDA kernel for pattern matching
    patternMatchingKernel << < 1, 32 >> > (d_input, inputRows, inputColumns, d_pattern, patternRows, patternColumns, d_resultCoords, maxCoordsSize);

    // Copy result coordinates from device to host
    int* resultCoords = new int[maxCoordsSize];
    cudaMemcpy(resultCoords, d_resultCoords, maxCoordsSize * sizeof(int), cudaMemcpyDeviceToHost);

    // Cleanup
    cudaFree(d_input);
    cudaFree(d_pattern);
    cudaFree(d_resultCoords);

    for (int i = 0; i < maxCoordsSize; i += 2)
    {
        //if (resultCoords[i] >= 0)
        cout << resultCoords[i] << ", " << resultCoords[i + 1] << endl;
    }

    delete[] resultCoords;

    return 0;
}

bool readFile(std::string filename, char** data, int* rows, int* columns)
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

        char* flat = new char[(*rows) * (*columns)];
        for (int i = 0; i < *rows; i++)
        {
            for (int j = 0; j < *columns; j++)
            {
                flat[i * (*columns) + j] = arr[i][j];
            }
        }

        *data = flat;

        return true;
    }
    else
    {
        std::cout << filename << " not opened" << std::endl;
        return false;
    }
}

/*
Checks for the pattern within the input file
Ignores wildcard character '*' and returns true or false if pattern is found or not
*/
__device__ bool checkForPattern(char* input, char* pattern, int inputRows, int inputColumns, int patternRows, int patternColumns, int i, int j)
{
    bool patternFound = true;

    // loop through each character in the pattern, stop as soon as pattern is out of bounds or it doesn't match
    for (int currentRow = 0; currentRow < patternRows && patternFound; currentRow++)
    {
        for (int currentColumn = 0; currentColumn < patternColumns && patternFound; currentColumn++)
        {
            if (pattern[currentRow * patternColumns + currentColumn] != '*' &&
                (i + currentRow < 0 || i + currentRow >= inputRows || j + currentColumn < 0 || j + currentColumn >= inputColumns || // out of bounds
                    input[(i + currentRow) * inputColumns + (j + currentColumn)] != pattern[currentRow * patternColumns + currentColumn])) // doesn't match pattern
            {
                patternFound = false;
            }
        }
    }

    return patternFound;
}