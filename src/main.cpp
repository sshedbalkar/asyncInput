#include "asyncgetline.h"
#include <iostream>

int main() {
    merci::thread::AsyncGetline ag;
    std::string input;

    while (true) {
        // Asynchronously get the next line of input, if any. This function
        // automagically sleeps a millisecond if there is no getline input.
        input = ag.GetLine();

        // Check to see if there was any input.
        if (!input.empty()) {
            // Print out the user's input to demonstrate it being processed.
            std::cout << "{" << input << "}\n";

            // Check for the exit condition.
            if (input == "exit") {
                break;
            }
        }

        // Print out a space character every so often to demonstrate
        // asynchronicity. cout << " ";
        // this_thread::sleep_for(chrono::milliseconds(100));
    }

    std::cout << "\n\n";
    return 0;
}