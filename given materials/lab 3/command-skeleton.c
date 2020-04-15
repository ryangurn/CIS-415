void lfcat()
{
	// Define your variables here

	// Get the current directory

	// Open the dir using opendir()

	// use a while loop to read the dir

		// Hint: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "a.out", "output.txt"

			// Open the file

			// Read in each line using getline()
				// Write the line to stdout

			// write 80 "-" characters to stdout

			// close the read file and free/null assign your line buffer

	//close the directory you were reading from using closedir()
}
