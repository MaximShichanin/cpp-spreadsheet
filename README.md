# cpp-spreadsheet

About Spreadsheet
----------------

This is emulation of spreadsheet

How to build
------------

To build this app you need:
cmake version 3.8 or higher (https://cmake.org/)
ANTLR version 4.xx (https://www.antlr.org)

To build the app follow steps:

0. mkdir ./build
1. cmake ../spreadsheet -DCMAKE_BUILD_TYPE=Release
2. cmake --build ./
If you need Debug version, use -DCMAKE_BUILD_TYPE=Debug flag.
Make sure that you have permissions to create files in your working
directory.
Program has been built successfully on Ubuntu/Linux 22.04 with
gcc version 11.2.0, but other gcc versions, that are compatible with C++17
standard should work properly.

How t use
---------

There is a piece of test case:

    auto sheet = CreateSheet();           <----- Create a new sheet
    sheet->SetCell("A2"_pos, "meow");   |
    sheet->SetCell("B2"_pos, "=1+2");   | <----- Add formulas to cells
    sheet->SetCell("A1"_pos, "=1/0");   |

    std::ostringstream texts;
    sheet->PrintTexts(texts);             <----- Print text-mode table to ostream "text"
    std::string str = texts.str();

    std::ostringstream values;
    sheet->PrintValues(values);           <----- Print value-move table to ostream "values"

    sheet->ClearCell("B2"_pos);           <----- Erase cell "B2". Printable area is changed

Values: text or numbers. Empty cell will be used like cells with zero value inside.

There are posible to use next operators: "+", "-", "*", "/" and references to
other cells.

What to improve?
---------------

Add more functional.
