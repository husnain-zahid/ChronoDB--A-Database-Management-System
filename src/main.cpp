#include <iostream>
#include <string>
#include "../storage/storage.h"
#include "../graph/graph.h"
#include "../query/parser.h"

using namespace ChronoDB;

int main() {
    StorageEngine storage;
    GraphEngine graph;
    Parser parser(storage, graph);

    std::cout << "=== ChronoDB Test CLI ===\n";
    std::cout << "Type 'EXIT' to quit.\n";

    // Sample pre-filled commands for quick testing
    std::vector<std::string> testCommands = {
        "CREATE TABLE students",
        "INSERT INTO students VALUES 1 Alice 3.8",
        "INSERT INTO students VALUES 2 Bob 3.5",
        "SELECT * FROM students",
        "UPDATE students SET GPA 3.9 WHERE id 1",
        "SELECT * FROM students",
        "DELETE FROM students WHERE id 2",
        "SELECT * FROM students",
        "UNDO",
        "SELECT * FROM students",
        "REDO",
        "SELECT * FROM students",
        
        // Graph Tests
        "GRAPH CREATE g1",
        "GRAPH ADDVERTEX g1 A",
        "GRAPH ADDVERTEX g1 B",
        "GRAPH ADDVERTEX g1 C",
        "GRAPH ADDEDGE g1 A B 5",
        "GRAPH ADDEDGE g1 B C 3",
        "GRAPH PRINT g1",
        "GRAPH BFS g1 A",
        "GRAPH DFS g1 A",
        "GRAPH DIJKSTRA g1 A C",
        "GRAPH REMOVEVERTEX g1 B",
        "GRAPH PRINT g1",
        "GRAPH DELETE g1"
    };

    // Run the pre-filled commands
    for (auto& cmd : testCommands) {
        std::cout << "\n> " << cmd << "\n";
        parser.parseAndExecute(cmd);
    }

    // Interactive CLI (optional)
    std::string input;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;
        if (input == "EXIT") break;
        parser.parseAndExecute(input);
    }

    std::cout << "Exiting ChronoDB Test CLI.\n";
    return 0;
}
