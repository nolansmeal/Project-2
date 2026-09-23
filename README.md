# ECE 309 — Project 2

## Overview

This project implements the core memory and streaming components of a simple conversation harness in C++. The program maintains a conversation between a user and a scripted model while keeping track of System, User, and Assistant messages.

The main components implemented for this project are the `Message`, `Conversation`, and `SentinelScanner` classes.

The `Conversation` class uses a custom dynamically allocated array to store messages. It supports dynamic growth and implements the Rule of Five for safe memory management.

The `SentinelScanner` processes model output as it is streamed in chunks. It detects the `<|end_conversation|>` sentinel even when the sentinel is divided between multiple chunks. The sentinel stops the conversation without being printed to the terminal.

## Building the Project

The project uses CMake. From the root directory of the project, run:

    cmake -S . -B build
    cmake --build build

This builds two executables:

- `./build/miniharness` — runs the conversation harness
- `./build/test_p2` — runs the Project 2 test suite

## Running the Program

To run the program using the provided greeting script:

    ./build/miniharness --script scripts/greeting.script

The program will prompt the user for input and generate responses from the scripted model.

To save the conversation to a transcript:

    ./build/miniharness --script scripts/greeting.script --save transcript.txt

The conversation ends when the model produces the `<|end_conversation|>` sentinel, the maximum number of turns is reached, or the user reaches EOF.

## Running the Tests

After building the project, run:

    ./build/test_p2

The test suite contains 12 assert-based tests covering:

- Empty conversation bounds
- System message ordering
- Deep-copy behavior
- Move behavior
- Dynamic array growth
- Sentinel scanner with normal text
- Sentinel detection across every split position
- False sentinel detection
- Large streamed input
- Harness turn limit
- Harness sentinel stopping
- Transcript replay

When all tests are successful, the program prints:

    All 12 tests passed!

## Implementation Details

The `Conversation` class stores messages in a dynamically allocated array. The array begins with a capacity of zero. When additional space is required, the capacity grows from 0 to 1 and then doubles as more messages are added.

The class implements the Rule of Five. Copy operations create independent copies of the dynamically allocated message array, while move operations transfer ownership of the existing array and leave the original object empty.

The `SentinelScanner` maintains a small pending buffer while processing streamed model output. It holds back at most the characters that could still become part of the sentinel. This allows the scanner to detect a sentinel that crosses chunk boundaries without storing the entire output stream.

## Project Structure

    include/core/             Core class headers
    include/harness/          Provided harness interface
    include/model/            Provided model interfaces
    src/                      C++ source files
    tests/p2/                 Project 2 tests
    docs/design-log-p2.md     Project design documentation
    scripts/greeting.script   Example scripted conversation