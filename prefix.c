//10.2022
// Parallel Prefix Scan using MPI 
// Code is done by: Damir Kadyrzhan, Student id: 20494312

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


// Nearest Power of 2 Function 
unsigned int nextPowerOf2(unsigned int n)
{
unsigned count = 0;
// if n is 0 then return without changing 
if (n && !(n & (n - 1))) // if n AND NOT(n AND (n - 1))
    return n;

// if n is not equal to 0 then proceed with while loop 
// given input is turned into binary data 
// then this data is shifted until binary data is 0 
// every iteration is counted. Amount of counts represent in which position to shift binary number 
// then this number is shifted to the left by 1 iteration, giving the closest power of 2 
    while( n != 0)
{
    n = n >> 1; // shift bits to the right 
    count = count + 1; // count + 1 
}
return 1 << count;
}


// Main Function
int main(int argc, char* argv[])
{

// MPI Initialise 
    MPI_Init(&argc, &argv); // MPI Initialise 
    int rank,size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // process rank (number), locate process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // size of cluster, locate process size


// Input variables 
    int input; // User input, initial number of elements
    int N; // Number of elements, after padding
    int process_amount; // amount of elements per process
    int i;

// Array variables 
    int array[N]; // Input array, after padding 
    int my_array[process_amount]; // Variable that is distributed to each process from array[N]

    int my_output[process_amount]; // Produced value from each core, also acts as a buffer in processes
    int output[N]; // Collected values from processes is stored in this array, which is then displayed in output

    int zeros_array[N]; // Specifies the size of array with zeros

// Level variables 
  // Each stage of calculation, data is stored in its own level variable, which then is passed to the next level on each stage
    int level_2_up[process_amount]; 
    int level_4_up[process_amount];
    int level_8_up[process_amount];
    int level_16_up[process_amount];
    int level_8_down[process_amount];
    int level_4_down[process_amount];
    int level_2_down[process_amount];

// User Input 
if(rank == 0){
    scanf("%d", &input);
    }
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);


// Randomize Array 
if(rank == 0){
    srand(time(NULL)); // Seeding the array, with time. To prevent array to generate the same values 
    for(i = 0; i < input; i++){
        array[i] = (rand()%10 + 1); // Generate number between 1 to 10
    }
    fflush(stdout);
}
MPI_Barrier(MPI_COMM_WORLD);


// Padding 
  // Any number that user enters, it will be rounded up to the next power of 2
  // The array with zeros with the size of next power of 2 is created 
if(rank == 0){ 
    N = input; // Assign number of elements with user input
    N = nextPowerOf2(N); // Round up to the next power of 2 using pre-defined function 

    for(i = 0; i < N; i++){
        zeros_array[i] = 0; // For loop to fill up the array with zeros
    }
}
MPI_Barrier(MPI_COMM_WORLD);

// Randomly generated sequence with the size of user input replaces the array with zeros,
if(rank == 0){
    for(i = 0; i < input; i++){ // This for loop replaces zero array with user input array
        zeros_array[i] = array[i];
    }
}
MPI_Barrier(MPI_COMM_WORLD);

// Filled up array with random generated sequence and added zeros to the next power of 2 is updated to the main array variable 
if(rank == 0){
    for(i = 0; i < N; i++){ 
        array[i] = zeros_array[i];
        printf("Input [%d]: %d \n", i, array[i]); // Print full array input with consideration of padding
    }
}
MPI_Barrier(MPI_COMM_WORLD);


// Broadcast data 
  // Sends the size of array to all of the processes, to allocate the amount of processors 
  // Then, code determines how many levels it needs to calculate the output
    int send_count = 1; // amount of data sent
    MPI_Bcast(&N, send_count, MPI_INT, 0, MPI_COMM_WORLD);


// MPI_Scatter - Variables 
    int send_rank = 0; // root processor sending the data

    process_amount = N/8; // amount of elements per process, used when elements are larger than processes
    int send_amount = 1; // amount of elements per process, used when elements are less or equal to processes

// Scatter data - one root processor sends out equal amount of data from an array to each processor
    if(size >= N){
    MPI_Scatter(&array, send_amount, MPI_INT, &my_array, send_amount, MPI_INT, send_rank, MPI_COMM_WORLD);  // Scatter when elements <= processes
    }
    else if(size < N){
    MPI_Scatter(&array, process_amount, MPI_INT, &my_array, process_amount, MPI_INT, send_rank, MPI_COMM_WORLD); // Scatter when elements > processes
    }


// If amount of elements less or equal than amount of processes then proceed
if(size >= N){
// This code block will execute with 1 or less elements per process
// If elements are less than processes, amount of processes will adjust to be 
// equal to amount of elements 


// Level 2 Up Phase 
// Identify values in position on multiples of 2 and update those values from position 1 before it 

// Process 0, 2, 4, 6 - send values
// Process 1, 3, 5, 7 - receive and update values
// Update values from (my_array --> level_2_up)
if(rank == 0){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){         // this if statement checks how many processes are needed for the amount of elements 
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];// once my_array saved its value in the buffer, it assigns value to level_2_up to proceed to the next level
        }
        MPI_Send(&my_array, 1, MPI_INT, 1, 1, MPI_COMM_WORLD); // CPU 0 sends value to CPU 1
    }
}
else if(rank == 1){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){
        MPI_Recv(&level_2_up, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 0 and assign to level_2_up)
        for(i = 0; i < 1; i++){
            level_2_up[i] = level_2_up[i] + my_array[i]; // Sum of current value in process 1 with received value from process 0
            my_output[i] = level_2_up[i]; // Save sum value to buffer 
        }
    }
        else{ // If elements > processes is false then the code will proceed without addition
              // It is required to run the code with less elements than processes
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];
        }
        }    
}
else if(rank == 2){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){         
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];// once my_array saved its value in the buffer, it assigns value to level_2_up to proceed to the next level
        }
        MPI_Send(&my_array, 1, MPI_INT, 3, 1, MPI_COMM_WORLD); // CPU 2 sends value to CPU 3
    }
}
else if(rank == 3){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){
        MPI_Recv(&level_2_up, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 2 and assign to level_2_up)
        for(i = 0; i < 1; i++){
            level_2_up[i] = level_2_up[i] + my_array[i];
            my_output[i] = level_2_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];
        }
        }
        
}
else if(rank == 4){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){          
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];// once my_array saved its value in the buffer, it assigns value to level_2_up to proceed to the next level
        }
        MPI_Send(&my_array, 1, MPI_INT, 5, 1, MPI_COMM_WORLD); // CPU 4 sends value to CPU 5
    }
}
else if(rank == 5){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){
        MPI_Recv(&level_2_up, 1, MPI_INT, 4, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 4 and assign to level_2_up)
        for(i = 0; i < 1; i++){
            level_2_up[i] = level_2_up[i] + my_array[i];
            my_output[i] = level_2_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];
        }
        }
        
}
else if(rank == 6){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){         
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];// once my_array saved its value in the buffer, it assigns value to level_2_up to proceed to the next level
        }
        MPI_Send(&my_array, 1, MPI_INT, 7, 1, MPI_COMM_WORLD); // CPU 6 sends value to CPU 7
    }
}
else if(rank == 7){
        for(i = 0; i < 1; i++){
            my_output[i] = my_array[i];
        }
    if(N > rank){
        MPI_Recv(&level_2_up, 1, MPI_INT, 6, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 6 and assign to level_2_up)
        for(i = 0; i < 1; i++){
            level_2_up[i] = level_2_up[i] + my_array[i];
            my_output[i] = level_2_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_up[i] = my_array[i];
        }
        }
        
}
else{ // If any process is doing nothing, it will proceed with updating variables for the next level
    for(i = 0; i < 1; i++){
        level_2_up[i] = my_array[i];
    }
}   
MPI_Barrier(MPI_COMM_WORLD); 



// Level 4 Up Phase 
// Identify values in position on multiples of 4 and update those values from position 2 before it

// Process 1, 5 - send values
// Process 3, 7 - receive and update values
// Process 0, 2, 4, 6 - do nothing, update to further variables (level_2_up --> level_4_up)
if(rank == 1){
        for(i = 0; i < 1; i++){
            my_output[i] = level_2_up[i];
        }
    if(N > rank){        
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_2_up[i];// once level_2_up saved its value in the buffer, it assigns value to level_4_up to proceed to the next level
        }
        MPI_Send(&level_2_up, 1, MPI_INT, 3, 1, MPI_COMM_WORLD); // CPU 1 sends value to CPU 3
    }
}
else if(rank == 3){
    if(N > rank){
        MPI_Recv(&level_4_up, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 1 and assign to level_4_up)
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_4_up[i] + level_2_up[i];
            my_output[i] = level_4_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_2_up[i];
        }
        }    
}
else if(rank == 5){
        for(i = 0; i < 1; i++){
            my_output[i] = level_2_up[i];
        }
    if(N > rank){        
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_2_up[i];// once level_2_up saved its value in the buffer, it assigns value to level_4_up to proceed to the next level
        }
        MPI_Send(&level_2_up, 1, MPI_INT, 7, 1, MPI_COMM_WORLD); // CPU 5 sends value to CPU 7
    }
}
else if(rank == 7){
    if(N > rank){
        MPI_Recv(&level_4_up, 1, MPI_INT, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 5 and assign to level_4_up)
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_4_up[i] + level_2_up[i];
            my_output[i] = level_4_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_4_up[i] = level_2_up[i];
        }
        }    
}
else{
    for(i = 0; i < 1; i++){
        level_4_up[i] = level_2_up[i];
    }
}   
MPI_Barrier(MPI_COMM_WORLD);


// Level 8 Up Phase 
// Identify values in position on multiples of 8 and update those values from position 4 before it

// Process 3 - send values
// Process 7 - receive and update values
// Process 0, 1, 2, 4, 5, 6 - do nothing, update to further variables (level_4_up --> level_8_up)
if(rank == 3){
        for(i = 0; i < 1; i++){
            my_output[i] = level_4_up[i];
        }
    if(N > rank){         // this if statement checks how many processes are needed for the amount of elements 
        for(i = 0; i < 1; i++){
            level_8_up[i] = level_4_up[i];// once level_4_up saved its value in the buffer, it assigns value to level_8_up to proceed to the next level
        }
        MPI_Send(&level_4_up, 1, MPI_INT, 7, 1, MPI_COMM_WORLD); // CPU 3 sends value to CPU 7
    }
}
else if(rank == 7){
    if(N > rank){
        MPI_Recv(&level_8_up, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 3 and assign to level_8_up)
        for(i = 0; i < 1; i++){
            level_8_up[i] = level_8_up[i] + level_4_up[i];
            my_output[i] = level_8_up[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_8_up[i] = level_4_up[i];
        }
        }    
}
else{
    for(i = 0; i < 1; i++){
        level_8_up[i] = level_4_up[i];
    }
}   
MPI_Barrier(MPI_COMM_WORLD);


// Level 4 Down Phase 
// Identify values in position on multiples of 4 and update values to position 2 after it

// Process 3 - send values
// Process 5 - receive and update values
// Process 0, 1, 2, 4, 6, 7 - do nothing, update to further variables (level_8_up --> level_4_down)

if(rank == 3){
        for(i = 0; i < 1; i++){
            my_output[i] = level_8_up[i];
        }
    if(N > rank){         
        for(i = 0; i < 1; i++){
            level_4_down[i] = level_8_up[i];// once level_8_up saved its value in the buffer, it assigns value to level_4_down to proceed to the next level
        }
        MPI_Send(&level_8_up, 1, MPI_INT, 5, 1, MPI_COMM_WORLD); // CPU 3 sends value to CPU 5
    }
}
else if(rank == 5){
    if(N > rank){
        MPI_Recv(&level_4_down, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 5 and assign to level_4_down)
        for(i = 0; i < 1; i++){
            level_4_down[i] = level_4_down[i] + level_8_up[i];
            my_output[i] = level_4_down[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_4_down[i] = level_8_up[i];
        }
        }    
}
else{
    for(i = 0; i < 1; i++){
        level_4_down[i] = level_8_up[i];
    }
}   
MPI_Barrier(MPI_COMM_WORLD);


// Level 2 Down Phase 
// Identify values in position on multiples of 2 and update values to position 1 after it

// Process 1, 3, 5 - send values
// Process 2, 4, 6 - receive and update values
// Process 0, 7 - do nothing, update to further variables (level_4_down --> level_2_down)
if(rank == 1){
        for(i = 0; i < 1; i++){
            my_output[i] = level_4_down[i];
        }
    if(N > rank){          
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];// once level_4_down saved its value in the buffer, it assigns value to level_2_down to proceed to the next level
        }
        MPI_Send(&level_4_down, 1, MPI_INT, 2, 1, MPI_COMM_WORLD); // CPU 1 sends value to CPU 2
    }
}
else if(rank == 2){
    if(N > rank){
        MPI_Recv(&level_2_down, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 1 and assign to level_2_down)
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_2_down[i] + level_4_down[i];
            my_output[i] = level_2_down[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];
        }
        }    
}
else if(rank == 3){
        for(i = 0; i < 1; i++){
            my_output[i] = level_4_down[i];
        }
    if(N > rank){        
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];// once level_4_down saved its value in the buffer, it assigns value to level_2_down to proceed to the next level
        }
        MPI_Send(&level_4_down, 1, MPI_INT, 4, 1, MPI_COMM_WORLD); // CPU 3 sends value to CPU 4
    }
}
else if(rank == 4){
    if(N > rank){
        MPI_Recv(&level_2_down, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 3 and assign to level_2_down)
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_2_down[i] + level_4_down[i];
            my_output[i] = level_2_down[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];
        }
        }    
}
else if(rank == 5){
        for(i = 0; i < 1; i++){
            my_output[i] = level_4_down[i];
        }
    if(N > rank){         
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];// once level_4_down saved its value in the buffer, it assigns value to level_2_down to proceed to the next level
        }
        MPI_Send(&level_4_down, 1, MPI_INT, 6, 1, MPI_COMM_WORLD); // CPU 5 sends value to CPU 6
    }
}
else if(rank == 6){
    if(N > rank){
        MPI_Recv(&level_2_down, 1, MPI_INT, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //  MPI_Recv(receive value from process 5 and assign to level_2_down)
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_2_down[i] + level_4_down[i];
            my_output[i] = level_2_down[i];
        }
    }
        else{
        for(i = 0; i < 1; i++){
            level_2_down[i] = level_4_down[i];
        }
        }    
}
else{
    for(i = 0; i < 1; i++){
        level_2_down[i] = level_4_down[i];
    }
}   
MPI_Barrier(MPI_COMM_WORLD);

}


// If amount of elements bigger than amount of processes
else if(size < N){

// If the number of elements is 16, then each process receives 2 elements
// These elements are stored as an array

// Level 2 Up Phase
// Each process deals with calculations within the process 
// Each elements in multiples of 2 are added with the value before it, meaning 
// the previous value is added with the value in front 
    if(rank == 0){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i]; // save elements to buffer 
        }

        my_array[1] = my_array[0] + my_array[1]; // sum of first and second element, saved as a second element 

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];  // proceed with the level, my_array --> level_2_up 
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i]; // save sum 
        }
    }
    // each process has the same iteration as above for level 2 up phase 
    // the calculations are done within processes 
    else if(rank == 1){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 2){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 3){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 4){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 6){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else if(rank == 7){
        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }

        my_array[1] = my_array[0] + my_array[1];

        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        }

        for(i = 0; i < process_amount; i++){
            my_output[i] = my_array[i];
        }
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_2_up[i] = my_array[i];
        } 
    }


// Level 4 Up Phase 
// Each element in multiples of 4 are updated with the value 2 before it 
// Meaning, the second element from processes 0, 2, 4, 6 are sent to the second element of processes 1, 3, 5, 7
// The received values on processes 1, 3, 5, 7 are added with initial values and updated
    if(rank == 0){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_2_up[i];
        }
        for(i = 0; i < process_amount; i++){ // save values to level_4_up
            level_4_up[i] = level_2_up[i];
        }
        MPI_Send(&level_2_up[1], 1, MPI_INT, 1, 1, MPI_COMM_WORLD); // send second element of level_2_up on process 0
    }
    else if(rank == 1){
        for(i = 0; i < process_amount; i++){ // save values to level_4_up
            level_4_up[i] = level_2_up[i];
        }
 
        MPI_Recv(&level_4_up[1], 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive value from process 0, save it as second element level_4_up

        level_4_up[1] = level_4_up[1] + level_2_up[1]; // do the sum 
        my_output[1] = level_4_up[1]; // save sum value to output

    }
    // every iteration will do the same calculation as the one above 
    else if(rank == 2){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_2_up[i];
        }
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
        MPI_Send(&level_2_up[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD);
    }
    else if(rank == 3){
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
 
        MPI_Recv(&level_4_up[1], 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_up[1] = level_4_up[1] + level_2_up[1];
        my_output[1] = level_4_up[1];
    }
    else if(rank == 4){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_2_up[i];
        }
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
        MPI_Send(&level_2_up[1], 1, MPI_INT, 5, 1, MPI_COMM_WORLD);
    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
 
        MPI_Recv(&level_4_up[1], 1, MPI_INT, 4, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_up[1] = level_4_up[1] + level_2_up[1];
        my_output[1] = level_4_up[1];
    }
    else if(rank == 6){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_2_up[i];
        }
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
        MPI_Send(&level_2_up[1], 1, MPI_INT, 7, 1, MPI_COMM_WORLD);
    }
    else if(rank == 7){
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
 
        MPI_Recv(&level_4_up[1], 1, MPI_INT, 6, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_up[1] = level_4_up[1] + level_2_up[1];
        my_output[1] = level_4_up[1];
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_4_up[i] = level_2_up[i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);



// Level 8 Up Phase 
// Each element in multiples of 8 are updated with the value 4 before it 
// Second element in processes 1, 5 sending the values to second element in processes 3, 7, where it will do the sum 
    if(rank == 1){
        for(i = 0; i < process_amount; i++){
            level_8_up[i] = level_4_up[i];
        }
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_4_up[i];
        }

        MPI_Send(&level_4_up[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD);
    }
    else if(rank == 3){
        for(i = 0; i < process_amount; i++){
            level_8_up[i] = level_4_up[i];
        }
 
        MPI_Recv(&level_8_up[1], 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_8_up[1] = level_8_up[1] + level_4_up[1];
        my_output[1] = level_8_up[1];

    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            level_8_up[i] = level_4_up[i];
        }
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_4_up[i];
        }

        MPI_Send(&level_4_up[1], 1, MPI_INT, 7, 1, MPI_COMM_WORLD);
    }
    else if(rank == 7){
        for(i = 0; i < process_amount; i++){
            level_8_up[i] = level_4_up[i];
        }
 
        MPI_Recv(&level_8_up[1], 1, MPI_INT, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_8_up[1] = level_8_up[1] + level_4_up[1];
        my_output[1] = level_8_up[1];
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_8_up[i] = level_4_up[i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);


// Level 16 Up Phase 
// Each element in multiples of 16 are updated with the value 8 before it 
// Second element in process 3 is sent to process 7, where it will do the sum
    if(rank == 3){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_8_up[i];
        }

        for(i = 0; i < process_amount; i++){
            level_16_up[i] = level_8_up[i];
        }
        MPI_Send(&level_8_up[1], 1, MPI_INT, 7, 1, MPI_COMM_WORLD);
    }
    else if(rank == 7){
        for(i = 0; i < process_amount; i++){
            level_16_up[i] = level_8_up[i];
    }
 
        MPI_Recv(&level_16_up[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_16_up[1] = level_16_up[1] + level_8_up[1]; // Second element of rank 3 is added with second element of rank 7
        my_output[1] = level_16_up[1];
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_16_up[i] = level_8_up[i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);



// Level 8 Down Phase 
// Each element in multiples of 8 update the value after 4  
// Second element in process 3 is sent to process 5, where it will do the sum with the second element of process 5
    if(rank == 3){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_16_up[i];
        }

        for(i = 0; i < process_amount; i++){
            level_8_down[i] = level_16_up[i];
        }
        MPI_Send(&level_16_up[1], 1, MPI_INT, 5, 1, MPI_COMM_WORLD);
    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            level_8_down[i] = level_16_up[i];
        }
 
        MPI_Recv(&level_8_down[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_8_down[1] = level_8_down[1] + level_16_up[1];
        my_output[1] = level_8_down[1];
    }
    else{
        for(i = 0; i < process_amount; i++){
        level_8_down[i] = level_16_up[i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);




// Level 4 Down Phase 
// Each element in multiples of 4 update the value after 2  
// Second element in processes 1, 3, 5 are sent to processes 2, 4, 6, where it will do the sum with the second element
// The rest of the processes will do nothing + update the variable 
    if(rank == 1){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_8_down[i];
        }

        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }
        MPI_Send(&level_8_down[1], 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
    }
    else if(rank == 2){
        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }

        MPI_Recv(&level_4_down[1], 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_down[1] = level_4_down[1] + level_8_down[1];
        my_output[1] = level_4_down[1];
    }
    else if(rank == 3){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_8_down[i];
        }

        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }
        MPI_Send(&level_8_down[1], 1, MPI_INT, 4, 1, MPI_COMM_WORLD);
    }
    else if(rank == 4){
        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }

        MPI_Recv(&level_4_down[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_down[1] = level_4_down[1] + level_8_down[1];
        my_output[1] = level_4_down[1];
    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_8_down[i];
        }

        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }
        MPI_Send(&level_8_down[1], 1, MPI_INT, 6, 1, MPI_COMM_WORLD);
    }
    else if(rank == 6){
        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        }

        MPI_Recv(&level_4_down[1], 1, MPI_INT, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        level_4_down[1] = level_4_down[1] + level_8_down[1];
        my_output[1] = level_4_down[1];
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_4_down[i] = level_8_down[i];
        } 
    }
    MPI_Barrier(MPI_COMM_WORLD);



// Level 2 Down Phase 
// Update elements after 1 of multiples of 2 
// Meaning processes 1 to 6 will send and receive elements 
    if(rank == 0){
        for(i = 0; i < process_amount; i++){
            my_output[i] = level_4_down[i];
        }

        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }
        MPI_Send(&level_4_down[1], 1, MPI_INT, 1, 1, MPI_COMM_WORLD); // Process 0, send the second element to Process 1
    }
    else if(rank == 1){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        // Process 1, receives the second element from Process 0 and saves it as first element of level_2_down 
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];


        MPI_Send(&level_4_down[1], 1, MPI_INT, 2, 1, MPI_COMM_WORLD); // Process 1, send the second element to Process 2
    }
    else if(rank == 2){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 2, receives the second element from Process 1 and saves it as first element of level_2_down 
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];

        MPI_Send(&level_4_down[1], 1, MPI_INT, 3, 1, MPI_COMM_WORLD); // Process 2, send the second element to Process 3
    }
    else if(rank == 3){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 3, receives the second element from Process 2 and saves it as first element of level_2_down 
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];

        MPI_Send(&level_4_down[1], 1, MPI_INT, 4, 1, MPI_COMM_WORLD);
        // Process 3, send the second element to Process 4

        
    }
    else if(rank == 4){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 4, receives the second element from Process 3 and saves it as first element of level_2_down
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];

        MPI_Send(&level_4_down[1], 1, MPI_INT, 5, 1, MPI_COMM_WORLD);
        // Process 4, send the second element to Process 5
        
    }
    else if(rank == 5){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 4, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 5, receives the second element from Process 4 and saves it as first element of level_2_down
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];

        MPI_Send(&level_4_down[1], 1, MPI_INT, 6, 1, MPI_COMM_WORLD);
        // Process 5, send the second element to Process 6
        
    }
    else if(rank == 6){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 6, receives the second element from Process 5 and saves it as first element of level_2_down
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];

        MPI_Send(&level_4_down[1], 1, MPI_INT, 7, 1, MPI_COMM_WORLD);
        // Process 5, send the second element to Process 7
        
    }
    else if(rank == 7){
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }

        MPI_Recv(&level_2_down[0], 1, MPI_INT, 6, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Process 7, receives the second element from Process 6 and saves it as first element of level_2_down
        level_2_down[0] = level_2_down[0] + level_4_down[0];
        my_output[0] = level_2_down[0];
    }
    else{
        for(i = 0; i < process_amount; i++){
            level_2_down[i] = level_4_down[i];
        }  
    }
    MPI_Barrier(MPI_COMM_WORLD);
}
MPI_Barrier(MPI_COMM_WORLD);



// MPI_Gather - variables 
int receive_amount = 1; // Amount to receive
int receive_rank = 0; // Root process receiving outputs

// Gather Output
// Root processor receives and collects values from each processor
if(size >= N){
        // Gather output if amount of elements less or equal to amount of processes
        MPI_Gather(&my_output, receive_amount, MPI_INT, &output, receive_amount, MPI_INT, 0, MPI_COMM_WORLD);
    }
else if(size < N){
        // Gather output if amount of elements more than the amount of processes
        MPI_Gather(&my_output, process_amount, MPI_INT, &output, process_amount, MPI_INT, 0, MPI_COMM_WORLD);
    }

// Print output 
    if(rank == 0){
        for(i = 0; i < N; i++){
        printf("\n Output [%d]: %d", i, output[i]);
    }
    }

    MPI_Finalize(); // Finalize operation
    return 0;
}

