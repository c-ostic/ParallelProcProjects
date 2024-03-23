// Project3Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>

using namespace std;

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

    //std::cout << "Input: " << inputRows << " " << inputColumns << std::endl;
    //for (int i = 0; i < inputRows; i++)
    //{
    //    for (int j = 0; j < inputColumns; j++)
    //    {
    //        std::cout << input[i][j];
    //    }
    //    std::cout << std::endl;
    //}
    //std::cout << std::endl;

    //std::cout << "Pattern: " << patternRows << " " << patternColumns << std::endl;
    //for (int i = 0; i < patternRows; i++)
    //{
    //    for (int j = 0; j < patternColumns; j++)
    //    {
    //        std::cout << pattern[i][j];
    //    }
    //    std::cout << std::endl;
    //}

    //Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int start;
    int end;
    int total_chars = inputRows * inputColumns;
    int per_rank_chars = total_chars / world_size;
    int leftover_chars = total_chars % world_size;

    if (world_rank < leftover_chars) {
        start = world_rank * (per_rank_chars + 1);
        end = start + per_rank_chars;
    }
    else {
        start = world_rank * per_rank_chars + leftover_chars;
        end = start + per_rank_chars - 1;
    }

    // create structure that holds coordinate pairs
    // inputRows * inputColumns is the maximum amount of possible patterns
    int coords_size = (end - start + 1) * 2;
    int* coords = new int [coords_size];

    for (int i = 0; i < coords_size; i++) {
        coords[i] = -1;
    }
    int count = 0;

    // loop through every space in input
    for (int k = start; k <= end; k++)
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
            coords[count] = i;
            coords[count + 1] = j;
            count += 2;
        }
    }

    int* all_coords = NULL;

    if (world_rank == 0) {
        all_coords = (int*) malloc(sizeof(int) * total_chars * 2);
    }

    MPI_Gather(coords, coords_size, MPI_INT, all_coords, coords_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        std::ofstream outputFile(/* filename.substr(0, filename.length() - 4) + */  "Output.txt");
        for (int i = 0; i < total_chars * 2; i += 2)
        {
            if (all_coords[i] != -1) {
                outputFile << all_coords[i] << ", " << all_coords[i + 1] << std::endl;
            }
        }
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
