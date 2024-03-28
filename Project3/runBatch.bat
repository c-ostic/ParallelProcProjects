@echo off
title Batch File Processing

setlocal EnableDelayedExpansion

rem Get the project executable location
set program=.\x64\Release\Project3.exe

rem Get the MPI executable location
set mpiExe="C:\Program Files\Microsoft MPI\Bin\mpiexec.exe"

rem Set input pattern and thread count data
set inputs=input1.txt input2.txt 
set patterns=pattern1.txt pattern2.txt
set threadCounts=1 2 4

rem Clear the output file
echo/ > batchOutput.txt

rem Create arrays from the input and pattern lists
set i=0
for %%Q in (%inputs%) do (
   set /A i+=1
   set "Q[!i!]=%%Q"
)
set i=0
for %%P in (%patterns%) do (
   set /A i+=1
   set "P[!i!]=%%P"
)

rem Loop through the files
for /L %%A in (1,1,%i%) do (
	echo Processing !Q[%%A]! !P[%%A]!
	echo Processing !Q[%%A]! !P[%%A]! >> batchOutput.txt
	
	rem Run the file with each thread count
	for %%t in (%threadCounts%) do (
		echo Threads: %%t
		echo Threads: %%t >> batchOutput.txt
		%mpiExe% -n %%t %program% !Q[%%A]! !P[%%A]! >> batchOutput.txt
		echo/ >> batchOutput.txt
	)
	
	rem Separator (in both console and output file)
	echo ---------------------------------------------------
	echo --------------------------------------------------- >> batchOutput.txt
)

rem Open the output file
start batchOutput.txt