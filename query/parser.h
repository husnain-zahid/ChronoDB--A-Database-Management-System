#ifndef CHRONODB_PARSER_H
#define CHRONODB_PARSER_H

#include <string>
#include <vector>
#include <stack>
#include <functional>
#include "../storage/storage.h"
#include "lexer.h"
#include "../graph/graph.h"

namespace ChronoDB {

    class Parser {
    public:
        Parser(StorageEngine& storageRef, GraphEngine& graphRef); 
        
        void parseAndExecute(const std::string& command);
        void undo(); 
        void redo();

    private:
        StorageEngine& storage;
        GraphEngine& graph;

        std::stack<std::function<void()>> undoStack; 
        std::stack<std::function<void()>> redoStack; // NEW

        void handleCreate(const std::vector<Token>& tokens);
        void handleInsert(const std::vector<Token>& tokens);
        void handleUpdate(const std::vector<Token>& tokens);
        void handleDelete(const std::vector<Token>& tokens);
        void handleSelect(const std::vector<Token>& tokens);

        void handleGraph(const std::vector<Token>& tokens); // NEW
    };

}

#endif
