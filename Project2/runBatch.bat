@echo off
title Batch File Processing

rem Get the porject executable location
set program=.\x64\Release\Project2.exe

set files=constitution.txt bible.txt big.txt verybig.txt oneWord.txt
set threadCounts=1 2 4 8 12 16 24

rem Clear the output file
echo/ > batchOutput.txt

rem Loop through the files
for %%f in (%files%) do (
	echo Processing %%f
	
	rem Run the file with each thread count
	for %%t in (%threadCounts%) do (
		echo Threads: %%t
		%program% %%f %%t >> batchOutput.txt
		echo/ >> batchOutput.txt
	)
	
	rem Separator (in both console and output file)
	echo ---------------------------------------------------
	echo --------------------------------------------------- >> batchOutput.txt
)

rem Open the output file
start batchOutput.txt