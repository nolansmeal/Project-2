# Design Log — Project 2

## Growth factor and amortized cost
For the Conversation class, I used a dynamically allocated array of Message objects. The class keeps track of the array using data_, while size_ stores the number of messages currently in the conversation and capacity_ stores the number of messages that can fit in the current allocation. An empty conversation starts with a size and capacity of zero and data_ set to nullptr.

When append() is called and the array is full, the capacity is increased. The first allocation changes the capacity from 0 to 1. After that, the capacity doubles each time more space is required, resulting in capacities of 1, 2, 4, 8, 16, and so on. A new array is allocated, the existing messages are copied into it, and the old array is deleted.

Although resizing requires copying the existing messages, it does not happen on every append. Most calls to append() simply place the new message into an available position. Because the capacity doubles, the average or amortized cost of appending a message is O(1).

## Rule of Five evidence

The Conversation class manages dynamically allocated memory, so I implemented the Rule of Five. This includes a destructor, copy constructor, copy assignment operator, move constructor, and move assignment operator.

The destructor uses delete[] to release the dynamically allocated Message array. The copy constructor performs a deep copy by allocating a new array and copying each message from the original conversation. This means that the copied conversation and original conversation contain the same messages but do not share the same dynamically allocated array. My test verifies this by checking that their begin() pointers have different addresses.

The copy assignment operator also creates a separate copy of the other conversation's array. It first checks for self-assignment and then allocates the required memory, copies the messages, deletes the old array, and updates the object's data.

The move constructor and move assignment operator work differently. Instead of copying every message, they transfer the data_ pointer, size, and capacity from the original object. The original object's pointer is then changed to nullptr, and its size and capacity are set to zero. My move test verifies that the moved conversation receives the same array address and that the original conversation becomes empty.

## Sentinel scanner: bounded pending_ proof
The SentinelScanner searches streamed text for the sentinel <|end_conversation|>. A challenge is that the sentinel may be divided between multiple chunks. For example, one chunk could end with part of the sentinel while the next chunk contains the rest.

To handle this, the scanner uses pending_ to temporarily store characters that could be the beginning of a sentinel. When feed() receives another chunk, it combines the pending characters with the new chunk and searches the combined string for the sentinel. If the sentinel is found, only the text before it is returned as safe text.

If the sentinel is not found, the scanner does not need to keep the entire chunk. It only keeps up to sentinel_.size() - 1 characters at the end of the text. Anything before those characters cannot become part of a complete sentinel in a future chunk, so it can safely be returned. Therefore, pending_ stays bounded by the length of the sentinel rather than growing with the total amount of streamed text.

I tested the scanner with normal text, false sentinel patterns, a large input stream, and the sentinel divided at every possible split position. These tests verify that the scanner can detect the sentinel across chunk boundaries without incorrectly stopping on similar text.

## What I would change differently
If I were to improve this project, I would focus on making the implementation easier to debug and test while keeping the same behavior. One improvement would be adding more debugging information during development so that I could see why the harness stopped, such as whether it reached the sentinel, the turn limit, or the end of the input. This was useful while testing the harness.

I would also spend more time designing the tests before implementing the classes. Writing the tests earlier would make the expected behavior clearer before writing the implementation. Overall, this project helped me better understand dynamic memory, pointers, copy and move operations, growable arrays, and processing data that arrives in separate chunks.