### C++/MPI distributed data shuffling function

This code was written as part of the requirement for CSE 570 Introduction to Parallel and Distributed Programming at State University of New York at Buffalo.

The code implements **Shuffling** operation using barebone MPI. Shuffling operation is one of the most fundamental operation in all distributed data processing frameworks like Apache Spark, Apache Hadoop, etc. It involves partitioning of the input data such that data items with the same hash are assigned to the same processor.

This code was tested and benchmarked on UB's HPC center, [Center for Computational Research.](http://www.buffalo.edu/ccr.html)



 