#include "asyncgetline.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

namespace merci::thread {
struct SharedState {
    // Cross-thread-safe boolean to tell the getline thread to stop when
    // AsyncGetline is deconstructed.
    std::atomic<bool> continueGettingInput;
    // Cross-thread-safe boolean to denote when the processing thread is ready
    // for the next input line. This exists to prevent any previous line(s) from
    // being overwritten by new input lines without using a queue by only
    // processing further getline input when the processing thread is ready.
    std::atomic<bool> sendOverNextLine;
    // string utilized safely by each thread due to the inputLock mutex.
    std::string input;
    // Mutex lock to ensure only one thread (processing vs. getline) is
    // accessing the input string at a time.
    std::mutex inputLock;
    //
    SharedState() {
        continueGettingInput = true;
        sendOverNextLine = true;
        input = "";
    }
};

std::shared_ptr<SharedState> state = nullptr;

AsyncGetline::AsyncGetline() {
    state = std::make_shared<SharedState>();

    // Start a new detached thread to call getline over and over again and
    // retrieve new input to be processed.
    std::thread([]() {
        // Non-synchronized string of input for the getline calls.
        std::string synchronousInput;
        char nextCharacter;

        // Get the asynchronous input lines.
        do {
            // Start with an empty line.
            synchronousInput = "";

            // Process input characters one at a time asynchronously, until
            // a new line character is reached.
            while (state->continueGettingInput) {
                // See if there are any input characters available
                // (asynchronously).
                while (std::cin.peek() == EOF) {
                    // Ensure that the other thread is always yielded to
                    // when necessary. Don't sleep here; only yield, in
                    // order to ensure that processing will be as responsive
                    // as possible.
                    std::this_thread::yield();
                }

                // Get the next character that is known to be available.
                nextCharacter = std::cin.get();

                // Check for new line character.
                if (nextCharacter == '\n') {
                    break;
                }

                // Since this character is not a new line character, add it
                // to the synchronousInput string.
                synchronousInput += nextCharacter;
            }

            // Be ready to stop retrieving input at any moment.
            if (!state->continueGettingInput) {
                break;
            }

            // Wait until the processing thread is ready to process the next
            // line.
            while (state->continueGettingInput && !state->sendOverNextLine) {
                // Ensure that the other thread is always yielded to when
                // necessary. Don't sleep here; only yield, in order to
                // ensure that the processing will be as responsive as
                // possible.
                std::this_thread::yield();
            }

            // Be ready to stop retrieving input at any moment.
            if (!state->continueGettingInput) {
                break;
            }

            // Safely send the next line of input over for usage in the
            // processing thread.
            state->inputLock.lock();
            state->input = synchronousInput;
            state->inputLock.unlock();

            // Signal that although this thread will read in the next line,
            // it will not send it over until the processing thread is
            // ready.
            state->sendOverNextLine = false;
        } while (state->continueGettingInput && state->input != "exit");
    }).detach();
}

AsyncGetline::~AsyncGetline() {
    // Stop the getline thread.
    state->continueGettingInput = false;
}

std::string AsyncGetline::GetLine() {
    // See if the next line of input, if any, is ready to be processed.
    if (state->sendOverNextLine) {
        // Don't consume the CPU while waiting for input;
        // this_thread::yield() would still consume a lot of CPU, so sleep
        // must be used.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        return "";
    } else {
        // Retrieve the next line of input from the getline thread and store
        // it for return.
        state->inputLock.lock();
        std::string returnInput = state->input;
        state->inputLock.unlock();

        // Also, signal to the getline thread that it can continue
        // sending over the next line of input, if available.
        state->sendOverNextLine = true;

        return returnInput;
    }
}
}