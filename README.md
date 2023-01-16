# Producer-Consumer-Problem
Implementation of the bounded-buffer producer-consumer problem using C++ semaphores and shared memory functions

## Table of Contents
- [Problem Description](#problem-description)
  + [What is the producer-consumer problem?](#what-is-the-producer-consumer-problem)
  + [Description of this implementation](#description-of-this-implementation)
- [Dependencies](#dependencies)
- [Running the program](#running-the-program)

## Problem Description
### What is the producer-consumer problem?

The Producer-Consumer problem is a classic problem this is used for multi-process synchronization.
In the producer-consumer problem, producers produce something and places it on a memory buffer and there is a consumer that is consuming the products from the same buffer. Since this is the __bounded-buffer__ version of the producer-consumer problem, the producers and consumers share the same memory buffer that is of fixed-size.

___The problem arises due to the following constraints:___
- The producer should produce data only when the buffer is not full. If the buffer is full, then the producer shouldn't be allowed to put any data into the buffer.
- The consumer should consume data only when the buffer is not empty. If the buffer is empty, then the consumer shouldn't be allowed to take any data from the buffer.
- The producer and consumer should not access the buffer at the same time.

<br>

### Description of this implementation
#### Producers:
- Each Producer would declare the live price of a commodity (e.g., GOLD, SILVER, CRUDEOIL, COTTON, …).

- Each producer is supposed to continuously declare the price of one commodity. For simplicity, we assume that each commodity price follows a normal distribution with parameters (𝜇, 𝜎^2). Therefore, producers will generate a new random price, share it with the consumer, and then sleep for an interval before producing the next price. All producers are processes running the same codebase. Producers are to be run concurrently, either in separate terminals, or in the background.
  
- Each producer keeps a log of all operations done throughout their lifetime. The log is output to stderr.

- While running a producer, you will specify the following command line arguments:
  1. Commodity name (e.g., GOLD – Assume no more than 10 characters.)
  2. Commodity Price Mean; 𝜇 – a double value.
  3. Commodity Price Standard Deviation; 𝜎 – a double value.
  4. Length of the sleep interval in milliseconds; T – an integer.
  5. Bounded-Buffer Size (number of entries); N – an integer.

  _Therefore, the command “./producer NATURALGAS 7.1 0.5 200 40” would run a producer that declares
  the current price of Natural Gas every 200ms according to a gaussian distribution with parameters
  (mean=7.1 and variance=0.25). The size of the shared bounded buffer is 40 entries_
 
#### Consumers:
- One Consumer would show a dashboard for the prices of all commodities. Producers and Consumer would run indefinitely sharing the prices through shared memory.

- The consumer is to print the current price of each commodity, along the average of the current and past 4 readings. An Up/Down arrow to show whether the current Price (AvgPrice) got increased or decreased from the prior one. Until you receive current prices, use 0.00 for the current price of any commodity. For simplicity, let’s limit the commodities to GOLD, SILVER, CRUDEOIL, NATURALGAS, ALUMINIUM, COPPER, NICKEL, LEAD, ZINC, MENTHAOIL, and COTTON. Commodities in the table will be sorted in alphabetical order.

- While running the consumer, you will specify the following command line argument:
  + Bounded-Buffer Size (number of entries); N – an integer.

## Dependencies
This problem implementation is meant for a UNIX-based kernel

## Running the program
First off, make sure to be in the program directory. 
Build the .o files using the makefile. Call the makefile usikng the command
```
make
```
Next, call the consumer by entering the following command with the buffer size you want as the argument. __ex:__
```
./consumer 200
```

Now, from any number of separate terminals you can run the producer. __ex:__
```
./producer NATURALGAS 7.1 0.5 200
```
