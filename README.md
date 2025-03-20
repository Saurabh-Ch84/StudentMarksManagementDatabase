gcc -c sqlite3.c -o sqlite3.o
  // To compile sqlite3.c using c compiler not c++ (amalgamation)
g++ marks_management.cpp sqlite3.o -o marks_management
  // To compile .cpp file using previously compiled files
./marks_management
  // To execute the file

// For Testing
.\sqlite3 marks_management.db

// To see the logical schema
  .tables

// To view the schema
.schema <TableName>

// Atlast all queries of sql can be used to insert values, perform joins etc.
