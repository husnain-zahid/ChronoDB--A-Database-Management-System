#include "parser.h"
#include <iostream>
#include "../utils/types.h"
#include "../utils/helpers.h"

namespace ChronoDB {

    Parser::Parser(StorageEngine& s, GraphEngine& g) : storage(s), graph(g) {}

    void Parser::undo() {
        if (undoStack.empty()) {
            Helper::printError("Nothing to Undo!");
            return;
        }
        auto reverseAction = undoStack.top();
        undoStack.pop();

        // Push the undone action into redoStack
        redoStack.push(reverseAction);

        reverseAction();
        Helper::printSuccess("Last action undone successfully.");
    }

    void Parser::redo() {
        if (redoStack.empty()) {
            Helper::printError("Nothing to Redo!");
            return;
        }
        auto redoAction = redoStack.top();
        redoStack.pop();

        // Execute redo and push back to undo stack
        redoAction();
        undoStack.push(redoAction);

        Helper::printSuccess("Redo executed successfully.");
    }

    void Parser::parseAndExecute(const std::string& commandLine) {
        std::string cmdUpper = Helper::toUpper(commandLine);

        if (cmdUpper == "UNDO") { undo(); return; }
        if (cmdUpper == "REDO") { redo(); return; }

        // Any new action clears the redo stack
        while(!redoStack.empty()) redoStack.pop();

        Lexer lexer(commandLine);
        std::vector<Token> tokens = lexer.tokenize();
        if (tokens.empty()) return;

        std::string cmd = Helper::toUpper(tokens[0].value);

        if (cmd == "CREATE") handleCreate(tokens);
        else if (cmd == "INSERT") handleInsert(tokens);
        else if (cmd == "SELECT") handleSelect(tokens);
        else if (cmd == "UPDATE") handleUpdate(tokens);
        else if (cmd == "DELETE") handleDelete(tokens);
        else if( cmd == "GRAPH") handleGraph(tokens); // NEW
        else Helper::printError("Unknown command: " + cmd);
    }

    // ---- HANDLE METHODS ----
    void Parser::handleCreate(const std::vector<Token>& tokens) {
        if (tokens.size() < 3 || Helper::toUpper(tokens[1].value) != "TABLE") {
            Helper::printError("Syntax Error. Expected: CREATE TABLE <name>");
            return;
        }
        std::string tableName = tokens[2].value;
        if (storage.createTable(tableName)) {
            Helper::printSuccess("Table '" + tableName + "' created.");
            
            // UNDO: Drop the table
            undoStack.push([this, tableName]() {
                std::cout << "[UNDO] Dropping table " << tableName << " (Simulated)" << std::endl;
            });
        } else {
            Helper::printError("Table already exists.");
        }
    }

    void Parser::handleInsert(const std::vector<Token>& tokens) {
        if (tokens.size() < 7 || Helper::toUpper(tokens[1].value) != "INTO" || Helper::toUpper(tokens[3].value) != "VALUES") {
            Helper::printError("Syntax Error: INSERT INTO <table> VALUES <id> <name> <gpa>");
            return;
        }
        if (!Helper::isNumber(tokens[4].value)) { Helper::printError("ID must be a number"); return; }

        std::string tableName = tokens[2].value;
        try {
            int id = std::stoi(tokens[4].value);
            std::string name = tokens[5].value;
            float gpa = std::stof(tokens[6].value);

            Record r;
            r.fields.emplace_back(id);
            r.fields.emplace_back(name);
            r.fields.emplace_back(gpa);

            if (storage.insertRecord(tableName, r)) {
                Helper::printSuccess("Record inserted.");

                undoStack.push([this, tableName, r]() {
                    storage.deleteRecord(tableName, std::get<int>(r.fields[0]));
                    std::cout << "[UNDO] Deleted record ID " << std::get<int>(r.fields[0]) << std::endl;
                });
            } else {
                Helper::printError("Insert failed.");
            }
        } catch (const std::exception& e) {
            Helper::printError("Error parsing values: " + std::string(e.what()));
        }
    }

    void Parser::handleSelect(const std::vector<Token>& tokens) {
        if (tokens.size() < 4) return;
        std::string tableName = tokens[3].value;
        auto rows = storage.selectAll(tableName);

        Helper::printLine('-', 40);
        std::cout << "Displaying " << rows.size() << " rows from " << tableName << ":" << std::endl;
        for(const auto& row : rows){
            for(const auto& field : row.fields)
                visit([](auto&& arg){ std::cout << arg << " | "; }, field);
            std::cout << std::endl;
        }
    }

    void Parser::handleUpdate(const std::vector<Token>& tokens) {
        if (tokens.size() != 8 || Helper::toUpper(tokens[2].value) != "SET" || 
            Helper::toUpper(tokens[5].value) != "WHERE" || Helper::toUpper(tokens[6].value) != "ID") {
            Helper::printError("Syntax Error: UPDATE <table> SET <field> <value> WHERE ID <id>");
            return;
        }

        std::string tableName = tokens[1].value;
        std::string field = Helper::toUpper(tokens[3].value);
        std::string value = tokens[4].value;
        int id = std::stoi(tokens[7].value);

        auto records = storage.selectAll(tableName);
        for (auto& rec : records) {
            if (std::get<int>(rec.fields[0]) == id) {
                Record oldRec = rec; // store for Undo

                if (field == "NAME") rec.fields[1] = value;
                else if (field == "GPA") rec.fields[2] = std::stof(value);
                else {
                    Helper::printError("Unknown field: " + field);
                    return;
                }

                storage.updateRecord(tableName, id, rec);

                undoStack.push([this, tableName, oldRec]() {
                    storage.updateRecord(tableName, std::get<int>(oldRec.fields[0]), oldRec);
                    std::cout << "[UNDO] Reverted update for ID " << std::get<int>(oldRec.fields[0]) << std::endl;
                });

                Helper::printSuccess("Record updated.");
                return;
            }
        }

        Helper::printError("Record not found.");
    }


    void Parser::handleDelete(const std::vector<Token>& tokens) {
        if (tokens.size() != 6 || Helper::toUpper(tokens[1].value) != "FROM" || 
            Helper::toUpper(tokens[3].value) != "WHERE" || Helper::toUpper(tokens[4].value) != "ID") {
            Helper::printError("Syntax Error: DELETE FROM <table> WHERE ID <id>");
            return;
        }

        std::string tableName = tokens[2].value;
        int id = std::stoi(tokens[5].value);

        auto records = storage.selectAll(tableName);
        Record deletedRecord;
        bool found = false;

        for (auto& r : records) {
            if (std::get<int>(r.fields[0]) == id) { deletedRecord = r; found = true; break; }
        }

        if (!found) { Helper::printError("Delete failed - record not found."); return; }

        if (storage.deleteRecord(tableName, id)) {
            Helper::printSuccess("Record deleted.");

            undoStack.push([this, tableName, deletedRecord]() {
                storage.insertRecord(tableName, deletedRecord);
                std::cout << "[UNDO] Restored record ID " << std::get<int>(deletedRecord.fields[0]) << std::endl;
            });
        }
    }

    void Parser::handleGraph(const std::vector<Token>& tokens) {
        if (tokens.size() < 2) {
            Helper::printError("GRAPH command requires an action.");
            return;
        }

        std::string action = Helper::toUpper(tokens[1].value);

        if (action == "CREATE" && tokens.size() == 3) {
            std::string name = tokens[2].value;
            graph.createGraph(name);
        } 
        else if (action == "DELETE" && tokens.size() == 3) {
            std::string name = tokens[2].value;
            graph.deleteGraph(name);
        } 
        else if (action == "ADDVERTEX" && tokens.size() == 4) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->addVertex(tokens[3].value);
        } 
        else if (action == "REMOVEVERTEX" && tokens.size() == 4) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->removeVertex(tokens[3].value);
        }
        else if (action == "ADDEDGE" && tokens.size() == 6) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) {
                std::string u = tokens[3].value;
                std::string v = tokens[4].value;
                int weight = std::stoi(tokens[5].value);
                g->addEdge(u, v, weight, false); // default undirected
            }
        }
        else if (action == "PRINT" && tokens.size() == 3) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->printGraph();
        }
        else if (action == "BFS" && tokens.size() == 4) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->bfs(tokens[3].value);
        }
        else if (action == "DFS" && tokens.size() == 4) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->dfs(tokens[3].value);
        }
        else if (action == "DIJKSTRA" && tokens.size() == 5) {
            Graph* g = graph.getGraph(tokens[2].value);
            if (g) g->dijkstra(tokens[3].value, tokens[4].value);
        }
        else {
            Helper::printError("Unknown GRAPH action or wrong number of arguments.");
        }
    }


}
