@echo off
title Batch File Processing

setlocal EnableDelayedExpansion

rem Get the project executable location
set program=.\x64\Release\Project4.exe

rem Set input pattern and thread count data
set inputs=input1.txt input2.txt input3.txt input4.txt input5.txt input6.txt input7.txt
set patterns=pattern1.txt pattern2.txt pattern3.txt pattern4.txt pattern5.txt pattern6.txt pattern7.txt
set blockCounts=1 2 4
set threadCounts=32 64 128

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
	
	rem Run the file with each block and thread count
	for %%b in (%blockCounts%) do (
		for %%t in (%threadCounts%) do (
			echo Blocks: %%b
			echo Threads: %%t
			echo Blocks: %%b >> batchOutput.txt
			echo Threads: %%t >> batchOutput.txt
			%program% !Q[%%A]! !P[%%A]! %%b %%t>> batchOutput.txt
			echo/ >> batchOutput.txt
		)
	)
	
	rem Separator (in both console and output file)
	echo ---------------------------------------------------
	echo --------------------------------------------------- >> batchOutput.txt
)

rem Open the output file
start batchOutput.txt