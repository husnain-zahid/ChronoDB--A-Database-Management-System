#include "graph.h"

// --- Graph Class Implementation ---

void Graph::addVertex(const std::string &name) {
  if (adjacencyList.find(name) == adjacencyList.end()) {
    adjacencyList[name] = {};
  } else {
    std::cerr << "Vertex " << name << " already exists." << std::endl;
  }
}

void Graph::addEdge(const std::string &u, const std::string &v, int weight,
                    bool isDirected) {
  // Automatically add vertices if they don't exist
  if (adjacencyList.find(u) == adjacencyList.end()) {
    addVertex(u);
  }
  if (adjacencyList.find(v) == adjacencyList.end()) {
    addVertex(v);
  }

  // Add edge u -> v
  adjacencyList[u].push_back({v, weight});

  // If undirected, add edge v -> u
  if (!isDirected) {
    adjacencyList[v].push_back({u, weight});
  }
}

void Graph::printGraph() const {
  std::cout << "Graph Adjacency List:" << std::endl;
  for (const auto &pair : adjacencyList) {
    std::cout << pair.first << " -> ";
    for (const auto &neighbor : pair.second) {
      std::cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
    }
    std::cout << std::endl;
  }
}

// --- GraphEngine Class Implementation ---

void GraphEngine::createGraph(const std::string &name) {
  if (graphs.find(name) == graphs.end()) {
    graphs[name] = Graph();
    std::cout << "Graph '" << name << "' created successfully." << std::endl;
  } else {
    std::cerr << "Graph '" << name << "' already exists." << std::endl;
  }
}

Graph *GraphEngine::getGraph(const std::string &name) {
  if (graphs.find(name) != graphs.end()) {
    return &graphs[name];
  } else {
    std::cerr << "Graph '" << name << "' not found." << std::endl;
    return nullptr;
  }
}

void GraphEngine::deleteGraph(const std::string &name) {
  if (graphs.erase(name)) {
    std::cout << "Graph '" << name << "' deleted successfully." << std::endl;
  } else {
    std::cerr << "Graph '" << name << "' not found." << std::endl;
  }
}

// --- Graph Algorithms ---

void Graph::bfs(const std::string &startNode) {
  if (adjacencyList.find(startNode) == adjacencyList.end()) {
    std::cerr << "Start node '" << startNode << "' not found in graph."
              << std::endl;
    return;
  }

  std::unordered_map<std::string, bool> visited;
  std::queue<std::string> q;

  visited[startNode] = true;
  q.push(startNode);

  std::cout << "BFS Traversal: ";

  while (!q.empty()) {
    std::string current = q.front();
    q.pop();
    std::cout << current << " -> ";

    for (const auto &neighbor : adjacencyList[current]) {
      if (!visited[neighbor.first]) {
        visited[neighbor.first] = true;
        q.push(neighbor.first);
      }
    }
  }
  std::cout << "End" << std::endl;
}

void Graph::dfs(const std::string &startNode) {
  if (adjacencyList.find(startNode) == adjacencyList.end()) {
    std::cerr << "Start node '" << startNode << "' not found in graph."
              << std::endl;
    return;
  }

  std::unordered_map<std::string, bool> visited;
  std::stack<std::string> s;

  s.push(startNode);

  std::cout << "DFS Traversal: ";

  while (!s.empty()) {
    std::string current = s.top();
    s.pop();

    if (!visited[current]) {
      std::cout << current << " -> ";
      visited[current] = true;

      // Push neighbors to stack
      for (const auto &neighbor : adjacencyList[current]) {
        if (!visited[neighbor.first]) {
          s.push(neighbor.first);
        }
      }
    }
  }
  std::cout << "End" << std::endl;
}

void Graph::dijkstra(const std::string &startNode, const std::string &endNode) {
  if (adjacencyList.find(startNode) == adjacencyList.end()) {
    std::cerr << "Start node '" << startNode << "' not found in graph."
              << std::endl;
    return;
  }
  if (adjacencyList.find(endNode) == adjacencyList.end()) {
    std::cerr << "End node '" << endNode << "' not found in graph."
              << std::endl;
    return;
  }

  std::unordered_map<std::string, int> distances;
  std::unordered_map<std::string, std::string> parent;

  for (const auto &pair : adjacencyList) {
    distances[pair.first] = std::numeric_limits<int>::max();
  }
  distances[startNode] = 0;

  std::priority_queue<std::pair<int, std::string>,
                      std::vector<std::pair<int, std::string>>,
                      std::greater<std::pair<int, std::string>>>
      pq;

  pq.push({0, startNode});

  while (!pq.empty()) {
    int currentDist = pq.top().first;
    std::string current = pq.top().second;
    pq.pop();

    if (current == endNode)
      break;

    if (currentDist > distances[current])
      continue;

    for (const auto &neighbor : adjacencyList[current]) {
      std::string nextNode = neighbor.first;
      int weight = neighbor.second;

      if (distances[current] + weight < distances[nextNode]) {
        distances[nextNode] = distances[current] + weight;
        parent[nextNode] = current;
        pq.push({distances[nextNode], nextNode});
      }
    }
  }

  if (distances[endNode] == std::numeric_limits<int>::max()) {
    std::cout << "No path found from " << startNode << " to " << endNode
              << std::endl;
  } else {
    std::vector<std::string> path;
    std::string curr = endNode;
    while (curr != startNode) {
      path.push_back(curr);
      curr = parent[curr];
    }
    path.push_back(startNode);
    std::reverse(path.begin(), path.end());

    std::cout << "Path: ";
    for (size_t i = 0; i < path.size(); ++i) {
      std::cout << path[i] << (i < path.size() - 1 ? " -> " : "");
    }
    std::cout << "\nTotal Cost: " << distances[endNode] << std::endl;
  }
}