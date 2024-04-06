# Producer_Consumer_with_docs

Example:

g++ producer.cpp -pthread -o producer
g++ consumer.cpp -pthread -o consumer
./producer & ./consumer

The producer program follows these steps:

1. Initialize semaphores

2. Setup shared memory, which consists of 4 subtasks:
  a. Create shared memory object
  b. Set the size of the shared memory
  c. Map the shared memory
  d. Initialize the buffer

3. Execution of the producer function
  a. Check if space is available
  b. Ensure mutual exclusion
  c. Produce
  d. Signal to the consumer

4. Clean

After the producer has finished execution, the result is a shared memory buffer containing the values 1 and 2.

The consumer follows similar steps, except instead of initializing semaphores or creating shared memory, it opens the existing semaphores and shared memory from the producer.

The consumer program follows these steps:

1. Open existing semaphores

2. Access shared memory
  a. Open shared memory
  b. Map shared memory

3. Execution of the consumer function
  a. Wait until there is at least one item in the buffer
  b. Ensure mutual exclusion
  c. Consume
  d. Signal to the producer

Throughout the course of both processes' execution, the values 1 and 2 are added to the shared memory buffer by the producer program, then removed (consumed) by the consumer program.








