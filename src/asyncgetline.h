#include <string>

/*
===============================================================================
Credit: https://stackoverflow.com/a/42264216
===============================================================================
*/

namespace merci::thread {

// This code works perfectly well on Windows 10 in Visual Studio 2015 c++ Win32
// Console Debug and Release mode. If it doesn't work in your OS or environment,
// that's too bad; guess you'll have to fix it. :( You are free to use this code
// however you please, with one exception: no plagiarism! (You can include this
// in a much bigger project without giving any credit.)
class AsyncGetline {
   public:
    // AsyncGetline is a class that allows for asynchronous CLI getline-style
    // input (with 0% CPU usage!), which normal iostream usage does not easily
    // allow.
    AsyncGetline();
    // Stop getting asynchronous CLI input.
    ~AsyncGetline();
    // Get the next line of input if there is any; if not, sleep for a
    // millisecond and return an empty string.
    std::string GetLine();
};
}