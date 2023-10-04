# Parallel-implementation-of-Submatrix-Search
Parallel implementation of Submatrix Search

This is a parallel implementation of submatrix search, where the program searches for an object inside a picture using MPI, openMP and CUDA

 Picture(N) and Object(N) – are square matrices of integers with N rows and N columns. Each member of the matrix represents a “color”. The range of possible colors is [1, 100].
Position(I, J) defines a coordinates of the upper left corner of the Object into Picture. 
For each pair of overlapping members p and o of the Picture and Object we will calculate a relative difference
				diff =  abs((p – o)/p)
The total difference is defined as a sum of all relative differences for all overlapping members for given Position(I, J) of the Object into Picture. We will call it Matching(I, J).

For example, for the Picture and Object from the Fig.1 the matching at Position(0,0) is equal
Matching(0,0) = abs((10-5)/10) + abs((5-14)/5) + abs((67-9)/67) + abs((23-20)/23) + abs((6-56)/6) +
abs((5-2)/5) + abs((12-6)/12) + abs((10-10)/10) + abs((20-3)/20)


| 23 | 6 | 5 | 14 | 9  |  5 |                                 
| -- | -- | -- | -- | -- | -- |                                         
| 12 | 10 | 20 | 56 | 2 | 3 |                                 
| 1 | 2 | 6 | 10 | 3 | 2 |                                   
| 45 | 3 | 7 | 5 | 5 | 2 |
| 11 | 43 | 2 | 54 | 1 | 12 |

| 5  | 14 | 9  |
| -- | -- | -- |
| 20 | 56 | 2  |
| 6  | 10  |  3 |

Fig 1. Picture(6) and Object(3)


Ideal Matching of the Object into the Picture at Position (1, 2)
Matching(1,2) = abs((5-5)/5) + abs((14-14)/14) + abs((9-9)/9) + abs((20-20)/20) + abs((56-56)/56) +
abs((2-2)/2) + abs((6-6)/6) + abs((10-10)/10) + abs((3-3)/3) = 0

This project deals with a “recognition” if there is a Position(I,J) of the Object into Picture with a Matchin(I,J) less than the given value.

The program runs with three process using MPI the main one sends half of the pictures and all the objects to the other two process to search
Then we use openMP threads each thread will hand one object and search for it in each picture, by sending the object and picture to the gpu
Then the gpu will do the search computation using CUDA

In the end the program will print each object if they were found or not

