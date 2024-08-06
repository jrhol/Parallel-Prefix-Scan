# Parallel Prefix Scan in OpenMP

- This code takes a user input, n, of the number of elements in a sequence
- Process 0 then generates n random integers and broadcasts these to other processes as required.
- The program then, in parallell, sums the elements from left to right.
<p align=center> $$b_{i} = b_{i-1} + a_{i}$$ </p>

- Process 0 prints the input and output sequence once complete.

## Getting Started
- I used MSYS2 on Windows to compile this program but in theory any compiler with required OpenMPI libraries should Work
1. Install MSYS2 from [Here](https://www.msys2.org/)
2. Run ``` pacman -Syu ```
3. Relaunch and then run ``` pacman -S make```
4. Run ``` pacman -S mingw-w64-x86_64-gcc```

Now We have a compiler installed, its time to install OpenMPI

5. Run ```pacman -S mingw-w64-x86_64-msmpi```
6. If On Windows Download [This](https://www.microsoft.com/en-us/download/details.aspx?id=57467) and install.
7. Now Clone this Repo ```git clone https://github.com/redfire1015/Parallel_Prefix_Scan```
8. Compile using ```mpicc```
9. Run Using ```mpiexec```
