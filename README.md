This course will introduce programming and essential concepts of operating systems, compilers, and concurrency, focused around several cross-cutting examples, such as memory management, error handling and threaded programming.

HW1
Serialization and Deserialization    09/2019
•	Used C programming language to implement serialization and deserialization functions to provide a way to compress and decompress files and directories.
•	Read the command line from the user in the shell to provide several functions such that for serialization, it reads the name, size, depth and contents of files and directories recursively from the path given by the user and zip them into a hexadecimal sequence of code which costs less storage on the disk and easy to be transmitted. For deserialization, given a sequence of hexadecimal code from the input, we can unzip the hexadecimal code and recursively reconstruct files and directory system which is exactly the same from the way zip them.

HW3
Dynamic Memory Allocator  10/2019
•	A working replica of explicit free list memory allocator by using immediate coalescing adjacent free blocks and strategies that mimic best fit placement policy through first fit police.
•	Supported boundary tags with footer and header at the same time and the footer will be removed within allocated blocks along with block splitting without creating splinters.
•	Maintain a free double linked list using last in first out discipline.

HW4
Job Spooler             11/2019
•	Used C programming language to get requests from command line to execute, cancel orders from the user, pause and resume jobs using process, show the status of user request jobs and set up pipe between each pipeline to handle input and output among them and to interact between each jobs.
•	Using fork, execution, signal handler to reap child process, pipes, multi-processing, input and output redirection.

HW5
Bourse Exchange Server  12/2019
•	Used C programming language to achieve multi-threads under one server process to respond user request.
•	Implemented locks (POSIX threads), semaphores and multithreading in order to handle different trades at the same time between client and server without conflicts of deadlock and race condition.

