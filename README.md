Library Management System (C++)

A console application for managing a small library: books, members, borrowing, returns, and late fines. Records are saved to text files, so data persists after the program closes.

Features

	•	Add, list, search, and remove books
	•	Register and list members
	•	Issue a book to a member with a 14-day due date
	•	Return a book, with an automatic late fine (Rs 10 per day late)
	•	Overdue report showing who has late books and the fine so far

Design

Object-oriented design with three classes:

	•	Book: details, availability, borrower, and due date
	•	Member: member ID and name
	•	Library: stores both lists, handles all features, and saves/loads the files

Requirements

	•	A C++11 compiler such as g++

How to build and run

g++ -std=c++11 -o library library_system.cpp
./library

On Windows, run library.exe. Data is saved to books.txt and members.txt in the same folder.

What I practised

Classes and encapsulation, vectors, file I/O, string handling, date arithmetic with <ctime>, input validation.

Author

Syed Asfandyar Shah, BSCS student at SZABIST University
