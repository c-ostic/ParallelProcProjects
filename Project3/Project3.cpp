// Project3Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <mpi.h>

using namespace std;

bool readFile(std::string filename, char*** data, int* rows, int* columns);
char** rotate(char** arr, int* rows, int* columns);
char** mirror(char** arr, int* rows, int* columns);
bool checkForPattern(char** input, char** pattern, int inputRows, int inputColumns, int patternRows, int patternColumns, int i, int j);

int main(int argc, char* argv[])
{
    MPI_Init(NULL, NULL);

    std::string inputFilename, patternFilename;

    // C++ makes the first argument the name of the program, so two additional arguments make 3
    if (argc != 3)
    {
        cout << "Program accepts two arguments: <input filename> and <pattern filename>" << endl;
        MPI_Finalize();
        return 1;
    }

    chrono::time_point<chrono::system_clock> startTotal, endTotal, startFile, endFile, startCount, endCount, startWrite, endWrite;

    startTotal = chrono::system_clock::now();

    inputFilename = argv[1];
    patternFilename = argv[2];

    // read in both files
    char** input = nullptr;
    char** pattern = nullptr;
    int inputRows, inputColumns, patternRows, patternColumns;

    startFile = chrono::system_clock::now();

    if (!readFile(inputFilename, &input, &inputRows, &inputColumns) ||
        !readFile(patternFilename, &pattern, &patternRows, &patternColumns))
    {
        MPI_Finalize();
        return 1;
    }

    endFile = chrono::system_clock::now();

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

    startCount = chrono::system_clock::now();

    //Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /*
    Distribute workload among MPI processes 
    Each rank has access to the whole input file, but may only check a portion of it
    Each portion is calculated based on the total number of characters
    Equally distributes leftover characters if needed
    */
    int start;
    int end;
    int patternMaxDimension = patternRows > patternColumns ? patternRows : patternColumns;
    int total_chars = (inputRows + patternMaxDimension - 1) * (inputColumns + patternMaxDimension - 1);
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

    //Since our valid coordinates can be negative now, switch from -1 to MAX integer
    //Uses bitwise manipulation to calculate max integer, without using limits.h
    int max_int = ~(1 << 31);
    for (int i = 0; i < coords_size; i++) {
        coords[i] = max_int;
    }
    int count = 0;

    // Rank 0 needs to calculate how much will be returned from each other rank
    int* rankCoords = new int[world_size];
    int* displacements = new int[world_size];
    rankCoords[0] = coords_size;
    displacements[0] = 0;
    if (world_rank == 0) {
        for (int rank = 1; rank < world_size; rank++) {
            int rankStart, rankEnd;
            if (rank < leftover_chars) {
                rankStart = rank * (per_rank_chars + 1);
                rankEnd = rankStart + per_rank_chars;
            }
            else {
                rankStart = rank * per_rank_chars + leftover_chars;
                rankEnd = rankStart + per_rank_chars - 1;
            }
            rankCoords[rank] = (rankEnd - rankStart + 1) * 2;
            displacements[rank] = displacements[rank - 1] + rankCoords[rank - 1];
        }
    }

    // loop through every space in input
    for (int k = start; k <= end; k++)
    {
        int i = k / (inputColumns + patternMaxDimension - 1) - (patternMaxDimension - 1);
        int j = k % (inputColumns + patternMaxDimension - 1) - (patternMaxDimension - 1);

        // check if pattern is at this position
        bool patternFound = false;

        /*
        Loops four times because:
        1. First rotate is 90 degrees
        2. Second rotate is 180 degrees
        3. Third rotate is 90 degrees counter-clockwise
        4. Fourth rotate is back to original pattern
        */
        for (int r = 0; r < 4 && !patternFound; r++)
        {
            patternFound = checkForPattern(input, pattern, inputRows, inputColumns, patternRows, patternColumns, i, j);
            pattern = rotate(pattern, &patternRows, &patternColumns);
        }
        
        pattern = mirror(pattern, &patternRows, &patternColumns);
        
        // Similar to above, runs four times to get all versions of the pattern mirrored
        for (int r = 0; r < 4 && !patternFound; r++)
        {
            patternFound = checkForPattern(input, pattern, inputRows, inputColumns, patternRows, patternColumns, i, j);
            pattern = rotate(pattern, &patternRows, &patternColumns);
        }

        // if the pattern is found, add it to the list
        if (patternFound)
        {
            coords[count] = i;
            coords[count + 1] = j;
            count += 2;
        }
    }

    endCount = chrono::system_clock::now();
    startWrite = chrono::system_clock::now();

    int* all_coords = NULL;

    if (world_rank == 0) {
        all_coords = (int*) malloc(sizeof(int) * total_chars * 2);
    }

    //Uses MPI_Gatherv rather than MPI_Gather because each rank can send a different amount of data
    MPI_Gatherv(coords, coords_size, MPI_INT, all_coords, rankCoords, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    //After gathering, rank 0 writes all coords to output file
    if (world_rank == 0) {
        std::ofstream outputFile(inputFilename.substr(0, inputFilename.length() - 4) +  "Output.txt");
        for (int i = 0; i < total_chars * 2; i += 2)
        {
            //Check that the coords are less than the MAX integer
            if (all_coords[i] < max_int) {
                if (all_coords[i] < 0) all_coords[i] = 0;
                if (all_coords[i + 1] < 0) all_coords[i + 1] = 0;

                outputFile << all_coords[i + 1] + 1 << ", " << all_coords[i] + 1 << std::endl;
            }
        }
    }

    endWrite = chrono::system_clock::now();
    endTotal = chrono::system_clock::now();

    chrono::duration<double> elapsedTotal, elapsedFile, elapsedCount, elapsedWrite;
    elapsedTotal = endTotal - startTotal;
    elapsedFile = endFile - startFile;
    elapsedCount = endCount - startCount;
    elapsedWrite = endWrite - startWrite;

    // Each MPI thread will have separate elapsedTime counts
    // Use MPI_Reduce to calculate the max all of the totals
    double allThreadsFile, allThreadsCount, allThreadsWrite, allThreadsTotal;

    //Uses MPI_MAX because each thread runs simultaneously, so we want the max rather than the sum 
    MPI_Reduce(&elapsedFile, &allThreadsFile, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&elapsedCount, &allThreadsCount, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&elapsedWrite, &allThreadsWrite, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&elapsedTotal, &allThreadsTotal, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        // Print total runtimes from root process
        std::cout << "Elapsed time file reading: " << allThreadsFile << std::endl;
        std::cout << "Elapsed time pattern count: " << allThreadsCount << std::endl;
        std::cout << "Elapsed time gather and write to file: " << allThreadsWrite << std::endl;
        std::cout << "Elapsed time total: " << allThreadsTotal << std::endl;
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

/*
Rotates matrix 90 degrees clock-wise
A new rotated array is constructed by traversing the original array by columns first, then by rows going backwards. 
Then, the pointers to the number of rows and columns are used to switch their values
Rotating an X by Y matrix results in a Y by X matrix
*/
char** rotate(char** arr, int* rows, int* columns)
{
    char** rotated = new char*[*columns];

    for (int i = 0, x = 0; i < *columns; i++, x++)
    {
        rotated[x] = new char[*rows];
        for (int j = *rows - 1, y = 0; j >= 0; j--, y++)
        {
            rotated[x][y] = arr[j][i];
        }
    }

    int temp = *rows;
    *rows = *columns;
    *columns = temp;

    return rotated;
}

/*
Mirrors matrix along the y-axis
The new mirrored array is constructed by traversing the original array normally by rows first,
then backwards through the columns
*/
char** mirror(char** arr, int* rows, int* columns)
{
    char** mirrored = new char* [*rows];

    for (int i = 0, x = 0; i < *rows; i++, x++)
    {
        mirrored[x] = new char[*columns];
        for (int j = *columns - 1, y = 0; j >= 0; j--, y++)
        {
            mirrored[x][y] = arr[i][j];
        }
    }

    return mirrored;
}

/*
Checks for the pattern within the input file
Ignores wildcard character '*' and returns true or false if pattern is found or not
*/
bool checkForPattern(char** input, char** pattern, int inputRows, int inputColumns, int patternRows, int patternColumns, int i, int j)
{
    bool patternFound = true;

    // loop through each character in the pattern, stop as soon as pattern is out of bounds or it doesn't match
    for (int currentRow = 0; currentRow < patternRows && patternFound; currentRow++)
    {
        for (int currentColumn = 0; currentColumn < patternColumns && patternFound; currentColumn++)
        {
            if (pattern[currentRow][currentColumn] != '*' && 
                (i + currentRow < 0 || i + currentRow >= inputRows || j + currentColumn < 0 || j + currentColumn >= inputColumns || // out of bounds
                input[i + currentRow][j + currentColumn] != pattern[currentRow][currentColumn])) // doesn't match pattern
            {
                patternFound = false;
            }
        }
    }

    return patternFound;
}
