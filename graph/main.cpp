#include "graph.h"
#include <iostream>

int main() {
  GraphEngine engine;

  // Create a graph named "CityMap"
  engine.createGraph("CityMap");
  Graph *cityMap = engine.getGraph("CityMap");

  if (cityMap) {
    std::cout << "Adding edges to CityMap..." << std::endl;
    // Add edges: A->B (5), B->C (10), A->C (20), C->D (5)
    // Using directed edges as implied by the arrow notation
    cityMap->addEdge("A", "B", 5, true);
    cityMap->addEdge("B", "C", 10, true);
    cityMap->addEdge("A", "C", 20, true);
    cityMap->addEdge("C", "D", 5, true);

    // Run BFS starting at A
    std::cout << "\n--- BFS from A ---" << std::endl;
    cityMap->bfs("A");

    // Run DFS starting at A
    std::cout << "\n--- DFS from A ---" << std::endl;
    cityMap->dfs("A");

    // Run Dijkstra from A to D
    std::cout << "\n--- Dijkstra from A to D ---" << std::endl;
    cityMap->dijkstra("A", "D");
  } else {
    std::cerr << "Failed to create or retrieve graph 'CityMap'." << std::endl;
  }

  return 0;
}
