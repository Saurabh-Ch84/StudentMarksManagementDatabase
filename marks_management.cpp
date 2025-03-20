#include <iostream>
#include "sqlite3.h"
#include <string>
#include <iomanip>

using namespace std;

// Function to execute SQL commands
int executeSQL(sqlite3* db, const string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return rc;
    }
    return SQLITE_OK;
}

// Function to initialize the database schema
void initializeDatabase(sqlite3* db) {
    string createStudentsTable = R"(
        CREATE TABLE IF NOT EXISTS Students (
            StudentID INTEGER PRIMARY KEY AUTOINCREMENT,
            FirstName TEXT NOT NULL,
            LastName TEXT NOT NULL,
            RollNumber INTEGER UNIQUE NOT NULL
        );
    )";

    string createSubjectsTable = R"(
        CREATE TABLE IF NOT EXISTS Subjects (
            SubjectID INTEGER PRIMARY KEY AUTOINCREMENT,
            SubjectName TEXT NOT NULL UNIQUE
        );
    )";

    string createTeachersTable = R"(
        CREATE TABLE IF NOT EXISTS Teachers (
            TeacherID INTEGER PRIMARY KEY AUTOINCREMENT,
            FirstName TEXT NOT NULL,
            LastName TEXT NOT NULL,
            SubjectID INTEGER,
            FOREIGN KEY (SubjectID) REFERENCES Subjects(SubjectID)
        );
    )";

    string createMarksTable = R"(
        CREATE TABLE IF NOT EXISTS Marks (
            MarkID INTEGER PRIMARY KEY AUTOINCREMENT,
            StudentID INTEGER,
            SubjectID INTEGER,
            MarksObtained REAL CHECK(MarksObtained BETWEEN 0 AND 100),
            FOREIGN KEY (StudentID) REFERENCES Students(StudentID),
            FOREIGN KEY (SubjectID) REFERENCES Subjects(SubjectID)
        );
    )";

    executeSQL(db, createStudentsTable);
    executeSQL(db, createSubjectsTable);
    executeSQL(db, createTeachersTable);
    executeSQL(db, createMarksTable);

    cout << "Database initialized successfully!" << endl;
}

// Function for teachers to update marks
void updateMarks(sqlite3* db, int teacherId) {
    sqlite3_stmt* stmt;

    // Get subject ID for the teacher
    string subjectQuery = "SELECT SubjectID FROM Teachers WHERE TeacherID=" + to_string(teacherId);
    sqlite3_prepare_v2(db, subjectQuery.c_str(), -1, &stmt, nullptr);

    int subjectId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        subjectId = sqlite3_column_int(stmt, 0);
    } else {
        cout << "Invalid Teacher ID." << endl;
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    // Display students and prompt marks update
    string studentQuery = "SELECT StudentID, FirstName, LastName, RollNumber FROM Students";
    sqlite3_prepare_v2(db, studentQuery.c_str(), -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int studentId = sqlite3_column_int(stmt, 0);
        string firstName = (const char*)sqlite3_column_text(stmt, 1);
        string lastName = (const char*)sqlite3_column_text(stmt, 2);
        int rollNumber = sqlite3_column_int(stmt, 3);

        cout << "Roll: " << rollNumber << ", Name: " << firstName << " " << lastName << endl;
        cout << "Enter marks for this student: ";
        float marks;
        cin >> marks;

        string insertMarks = "INSERT OR REPLACE INTO Marks(StudentID, SubjectID, MarksObtained) VALUES (" +
                             to_string(studentId) + ", " + to_string(subjectId) + ", " + to_string(marks) + ")";
        executeSQL(db, insertMarks);
    }

    sqlite3_finalize(stmt);
}

// Function to view sorted results
void viewSortedResults(sqlite3* db) {
    string query =
        "SELECT Students.RollNumber, Students.FirstName || ' ' || Students.LastName AS Name,"
        " SUM(Marks.MarksObtained) AS TotalMarks"
        " FROM Students"
        " JOIN Marks ON Students.StudentID = Marks.StudentID"
        " GROUP BY Students.StudentID"
        " ORDER BY TotalMarks DESC";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    cout << "\nFinal Sorted Results:\n";
    cout << left << setw(10) << "Roll" << setw(20) << "Name" << setw(10) << "Total Marks\n";
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int roll = sqlite3_column_int(stmt, 0);
        string name = (const char*)sqlite3_column_text(stmt, 1);
        float totalMarks = sqlite3_column_double(stmt, 2);

        cout << left << setw(10) << roll << setw(20) << name << setw(10) << totalMarks << endl;
    }

    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;

    // Open or create the database
    if (sqlite3_open("marks_management.db", &db)) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return -1;
    }

    initializeDatabase(db);

    int choice;

    do {
        cout << "\nMarks Management System\n";
        cout << "1. Update Marks (Teacher)\n";
        cout << "2. View Final Sorted Results\n";
        cout << "0. Exit\n";

        cin >> choice;

        switch(choice) {
            case 1: {
                int teacherId;
                cout << "Enter Teacher ID: ";
                cin >> teacherId;
                updateMarks(db, teacherId);
                break;
            }
            case 2:
                viewSortedResults(db);
                break;
            case 0:
                break;
            default:
                cout << "Invalid option.\n";
                break;
        }

    } while(choice != 0);

    sqlite3_close(db);

    return 0;
}
