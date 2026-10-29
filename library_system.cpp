/*
 * Library Management System
 * Author: Syed Asfandyar Shah
 *
 * Console application that manages books and members, issues and returns
 * books with a due date, calculates late fines, and saves everything to
 * text files so data persists between runs.
 *
 * Concepts used: classes and encapsulation, vectors, file I/O,
 * string handling, input validation, <ctime> date arithmetic.
 *
 * Compile:  g++ -std=c++11 -o library library_system.cpp
 * Run:      ./library          (library.exe on Windows)
 */

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int LOAN_DAYS = 14;             // how long a book can be borrowed
const int FINE_PER_DAY = 10;          // fine for each day late
const int SECONDS_PER_DAY = 24 * 60 * 60;
const string BOOKS_FILE = "books.txt";
const string MEMBERS_FILE = "members.txt";

// ---------- Helper functions ----------

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

string formatDate(time_t t) {
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", localtime(&t));
    return string(buffer);
}

// '|' separates fields in the save files, so it is not allowed in text.
string cleanText(string s) {
    replace(s.begin(), s.end(), '|', '-');
    return s;
}

vector<string> split(const string &line, char delimiter) {
    vector<string> parts;
    string item;
    stringstream ss(line);
    while (getline(ss, item, delimiter)) parts.push_back(item);
    return parts;
}

int readInt(const string &prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  Please enter a valid number.\n";
    }
}

string readLine(const string &prompt) {
    string text;
    while (true) {
        cout << prompt;
        getline(cin, text);
        if (!text.empty()) return cleanText(text);
        cout << "  This field cannot be empty.\n";
    }
}

// ---------- Book class ----------

class Book {
private:
    int id;
    string title;
    string author;
    bool available;
    int borrowerId;   // 0 when the book is on the shelf
    time_t dueDate;   // 0 when the book is on the shelf

public:
    Book(int id, const string &title, const string &author,
         bool available = true, int borrowerId = 0, time_t dueDate = 0)
        : id(id), title(title), author(author), available(available),
          borrowerId(borrowerId), dueDate(dueDate) {}

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    bool isAvailable() const { return available; }
    int getBorrowerId() const { return borrowerId; }
    time_t getDueDate() const { return dueDate; }

    void issueTo(int memberId) {
        available = false;
        borrowerId = memberId;
        dueDate = time(nullptr) + (time_t)LOAN_DAYS * SECONDS_PER_DAY;
    }

    void markReturned() {
        available = true;
        borrowerId = 0;
        dueDate = 0;
    }

    // Number of days past the due date (0 if not late).
    int daysLate() const {
        if (available) return 0;
        time_t now = time(nullptr);
        if (now <= dueDate) return 0;
        return (int)((now - dueDate) / SECONDS_PER_DAY);
    }

    string serialize() const {
        ostringstream out;
        out << id << '|' << title << '|' << author << '|' << available << '|'
            << borrowerId << '|' << (long long)dueDate;
        return out.str();
    }

    static bool deserialize(const string &line, Book &result) {
        vector<string> f = split(line, '|');
        if (f.size() != 6) return false;
        result = Book(stoi(f[0]), f[1], f[2], f[3] == "1", stoi(f[4]),
                      (time_t)stoll(f[5]));
        return true;
    }
};

// ---------- Member class ----------

class Member {
private:
    int id;
    string name;

public:
    Member(int id, const string &name) : id(id), name(name) {}
    int getId() const { return id; }
    string getName() const { return name; }

    string serialize() const { return to_string(id) + "|" + name; }

    static bool deserialize(const string &line, Member &result) {
        vector<string> f = split(line, '|');
        if (f.size() != 2) return false;
        result = Member(stoi(f[0]), f[1]);
        return true;
    }
};

// ---------- Library class ----------

class Library {
private:
    vector<Book> books;
    vector<Member> members;

    int nextBookId() const {
        int maxId = 0;
        for (const Book &b : books) maxId = max(maxId, b.getId());
        return maxId + 1;
    }

    int nextMemberId() const {
        int maxId = 0;
        for (const Member &m : members) maxId = max(maxId, m.getId());
        return maxId + 1;
    }

    Book *findBook(int id) {
        for (Book &b : books) if (b.getId() == id) return &b;
        return nullptr;
    }

    Member *findMember(int id) {
        for (Member &m : members) if (m.getId() == id) return &m;
        return nullptr;
    }

    string memberName(int id) {
        Member *m = findMember(id);
        return m ? m->getName() : "Unknown";
    }

    void printBookRow(const Book &b) {
        cout << left << setw(5) << b.getId() << setw(30) << b.getTitle().substr(0, 28)
             << setw(22) << b.getAuthor().substr(0, 20);
        if (b.isAvailable()) {
            cout << "Available\n";
        } else {
            cout << "Issued to " << memberName(b.getBorrowerId())
                 << " (due " << formatDate(b.getDueDate()) << ")\n";
        }
    }

    void printBookHeader() {
        cout << left << setw(5) << "ID" << setw(30) << "Title" << setw(22) << "Author"
             << "Status\n" << string(80, '-') << "\n";
    }

public:
    // --- Saving and loading ---
    void load() {
        string line;
        ifstream bookFile(BOOKS_FILE);
        while (getline(bookFile, line)) {
            Book b(0, "", "");
            if (Book::deserialize(line, b)) books.push_back(b);
        }
        ifstream memberFile(MEMBERS_FILE);
        while (getline(memberFile, line)) {
            Member m(0, "");
            if (Member::deserialize(line, m)) members.push_back(m);
        }
    }

    void save() const {
        ofstream bookFile(BOOKS_FILE);
        for (const Book &b : books) bookFile << b.serialize() << "\n";
        ofstream memberFile(MEMBERS_FILE);
        for (const Member &m : members) memberFile << m.serialize() << "\n";
    }

    // --- Features ---
    void addBook() {
        string title = readLine("Title: ");
        string author = readLine("Author: ");
        books.push_back(Book(nextBookId(), title, author));
        save();
        cout << "Book added with ID " << books.back().getId() << ".\n";
    }

    void addMember() {
        string name = readLine("Member name: ");
        members.push_back(Member(nextMemberId(), name));
        save();
        cout << "Member registered with ID " << members.back().getId() << ".\n";
    }

    void listBooks() {
        if (books.empty()) { cout << "No books in the library yet.\n"; return; }
        printBookHeader();
        for (const Book &b : books) printBookRow(b);
    }

    void listMembers() {
        if (members.empty()) { cout << "No members registered yet.\n"; return; }
        cout << left << setw(6) << "ID" << "Name\n" << string(30, '-') << "\n";
        for (const Member &m : members)
            cout << left << setw(6) << m.getId() << m.getName() << "\n";
    }

    void searchBooks() {
        string keyword = toLower(readLine("Search by title or author: "));
        bool found = false;
        for (const Book &b : books) {
            if (toLower(b.getTitle()).find(keyword) != string::npos ||
                toLower(b.getAuthor()).find(keyword) != string::npos) {
                if (!found) printBookHeader();
                printBookRow(b);
                found = true;
            }
        }
        if (!found) cout << "No matching books found.\n";
    }

    void issueBook() {
        int bookId = readInt("Book ID to issue: ");
        Book *book = findBook(bookId);
        if (!book) { cout << "No book with that ID.\n"; return; }
        if (!book->isAvailable()) { cout << "That book is already issued.\n"; return; }

        int memberId = readInt("Member ID: ");
        Member *member = findMember(memberId);
        if (!member) { cout << "No member with that ID.\n"; return; }

        book->issueTo(memberId);
        save();
        cout << "\"" << book->getTitle() << "\" issued to " << member->getName()
             << ". Due date: " << formatDate(book->getDueDate()) << "\n";
    }

    void returnBook() {
        int bookId = readInt("Book ID to return: ");
        Book *book = findBook(bookId);
        if (!book) { cout << "No book with that ID.\n"; return; }
        if (book->isAvailable()) { cout << "That book was not issued.\n"; return; }

        int late = book->daysLate();
        book->markReturned();
        save();
        cout << "Book returned. ";
        if (late > 0)
            cout << "It was " << late << " day(s) late. Fine: Rs " << late * FINE_PER_DAY << "\n";
        else
            cout << "No fine.\n";
    }

    void overdueReport() {
        bool any = false;
        for (const Book &b : books) {
            if (b.daysLate() > 0) {
                if (!any) cout << "Overdue books:\n";
                cout << "  [" << b.getId() << "] " << b.getTitle() << " - "
                     << memberName(b.getBorrowerId()) << ", " << b.daysLate()
                     << " day(s) late, fine so far Rs " << b.daysLate() * FINE_PER_DAY << "\n";
                any = true;
            }
        }
        if (!any) cout << "No overdue books.\n";
    }

    void removeBook() {
        int bookId = readInt("Book ID to remove: ");
        Book *book = findBook(bookId);
        if (!book) { cout << "No book with that ID.\n"; return; }
        if (!book->isAvailable()) { cout << "Cannot remove a book that is currently issued.\n"; return; }
        books.erase(remove_if(books.begin(), books.end(),
                              [bookId](const Book &b) { return b.getId() == bookId; }),
                    books.end());
        save();
        cout << "Book removed.\n";
    }
};

// ---------- Main menu ----------

void printMenu() {
    cout << "\n========== LIBRARY MANAGEMENT SYSTEM ==========\n"
         << " 1. Add book            6. Issue book\n"
         << " 2. Add member          7. Return book\n"
         << " 3. List all books      8. Overdue report\n"
         << " 4. List all members    9. Remove book\n"
         << " 5. Search books        0. Exit\n";
}

int main() {
    Library library;
    library.load();

    while (true) {
        printMenu();
        int choice = readInt("Choose an option: ");
        cout << "\n";
        switch (choice) {
            case 1: library.addBook(); break;
            case 2: library.addMember(); break;
            case 3: library.listBooks(); break;
            case 4: library.listMembers(); break;
            case 5: library.searchBooks(); break;
            case 6: library.issueBook(); break;
            case 7: library.returnBook(); break;
            case 8: library.overdueReport(); break;
            case 9: library.removeBook(); break;
            case 0: cout << "Goodbye!\n"; return 0;
            default: cout << "Invalid option. Try again.\n";
        }
    }
}
