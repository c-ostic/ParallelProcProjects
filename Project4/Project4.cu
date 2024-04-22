
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
    int stride = blockDim.x * gridDim.x;
    int k = blockIdx.x * blockDim.x + threadIdx.x;

    while (k < inputRows * inputColumns)
    {
        int i = k / inputColumns;
        int j = k % inputColumns;

        // Perform pattern matching using the checkForPattern function
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

        k += stride;
    }
}

int main(int argc, char* argv[]) {

    std::string inputFilename, patternFilename;
    int numBlocks = 1, numThreads = 1;

    // C++ makes the first argument the name of the program, so 4 additional arguments make 5
    if (argc != 5)
    {
        cout << "Program accepts four arguments: <input filename>, <pattern filename>, <num blocks>, <num threads>" << endl;
        return 1;
    }

    inputFilename = argv[1];
    patternFilename = argv[2];

    try
    {
        numBlocks = stoi(argv[3]);
        numThreads = stoi(argv[4]);
    }
    catch (...)
    {
        cout << "Must provide integers for the number of blocks and threads" << endl;
        return 1;
    }

    // read in both files
    char* input = nullptr;
    char* pattern = nullptr;
    int inputRows, inputColumns, patternRows, patternColumns;

    if (!readFile(inputFilename, &input, &inputRows, &inputColumns) ||
        !readFile(patternFilename, &pattern, &patternRows, &patternColumns))
    {
        cout << "Failed to read files" << endl;
        return 1;
    }

    // Print arguments
    cout << "Input File: " << inputFilename << endl;
    cout << "Patter File: " << patternFilename << endl;
    cout << "Number of Blocks: " << numBlocks << endl;
    cout << "Number of Threads: " << numThreads << endl;


    // Allocate device memory for input, pattern, and result coordinates
    int maxCoordsSize = inputRows * inputColumns * 2;
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
    patternMatchingKernel << < numBlocks, numThreads >> > (d_input, inputRows, inputColumns, d_pattern, patternRows, patternColumns, d_resultCoords, maxCoordsSize);

    // Copy result coordinates from device to host
    int* resultCoords = new int[maxCoordsSize];
    cudaMemcpy(resultCoords, d_resultCoords, maxCoordsSize * sizeof(int), cudaMemcpyDeviceToHost);

    // Cleanup
    cudaFree(d_input);
    cudaFree(d_pattern);
    cudaFree(d_resultCoords);

    // Print results (to console for now)
    for (int i = 0; i < maxCoordsSize; i += 2)
    {
        if (resultCoords[i] >= 0)
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