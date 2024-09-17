#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <queue>
#include <vector>

struct Link {
  std::size_t src;        // not required
  std::size_t dest;       // not required
  float propogationDelay; // in ms
  float capacity;         // in Mbps
};

struct Connection {
  std::size_t src;
  std::size_t dest;
  float minCapacity;
  float averageCapacity;
  float maxCapacity;
};

struct EnrichedLink : Link {
  std::vector<Connection> connections;

  EnrichedLink(Link link) : Link(link) {}
  EnrichedLink() {
    // Enriched Link should never be default constructed
    assert(false);
  }

  void add_connection(const Connection &connection) {
    connections.push_back(connection);
  }

  void rollback_last_connection() { connections.pop_back(); }
};

struct EnrichedConnection : Connection {
  std::vector<Link> path;
  std::size_t vcId;
  EnrichedConnection(const Connection &connection, std::vector<Link> &&path,
                     std::size_t vcId)
      : Connection(std::move(connection)), path(std::move(path)), vcId(vcId) {}
};

using LinkDistanceCalculator = std::function<float(const Link &link)>;
using ConnectionCondition = std::function<bool(
    const Connection &incomingConnection, const EnrichedLink &link)>;

template <typename T>
using min_heap = std::priority_queue<T, std::vector<T>, std::greater<T>>;

namespace util {

static inline std::string build_path_sring(const std::vector<Link> &path) {
  std::string pathString;
  for (std::size_t i = 0; i < path.size(); i++) {
    pathString += std::to_string(path[i].src) + " -> ";
  }
  pathString += std::to_string(path[path.size() - 1].dest);
  return pathString;
}

template <typename Container>
static inline std::string build_vc_ids(const Container &vcIds) {
  std::string vcIdString{};
  for (int vcId : vcIds)
    vcIdString += std::to_string(vcId) + ", ";
  vcIdString.pop_back();
  vcIdString.pop_back();
  return vcIdString;
}

// Function to find the maximum width of elements in a column
static inline std::size_t
find_max_row_width(const std::vector<std::vector<std::string>> &table,
                   std::size_t col) {
  std::size_t maxWidth = 0;
  for (const auto &row : table)
    if (col < row.size())
      maxWidth = std::max(maxWidth, row[col].length());

  return maxWidth;
}

// Function to print the table
static inline void print_table(std::vector<std::vector<std::string>> &table,
                               std::ostream &os) {
  if (table.empty())
    return;

  // Find the maximum width for each column
  std::vector<std::size_t> colWidths;
  std::size_t numCols = table[0].size();
  std::for_each(table.begin(), table.end(), [&](const auto &row) {
    numCols = std::max(numCols, row.size());
  });

  for (std::size_t col = 0; col < numCols; ++col) {
    colWidths.push_back(find_max_row_width(table, col));
  }

  // Print the table with borders
  for (const auto &row : table) {
    os << "+";
    for (std::size_t col = 0; col < row.size(); ++col) {
      os << std::string(colWidths[col] + 2, '-'); // Adjust for padding
      os << "+";
    }
    os << '\n';

    os << "|";
    for (std::size_t col = 0; col < row.size(); ++col) {
      os << " " << std::setw(colWidths[col]) << std::left << row[col] << " |";
    }
    os << '\n';
  }

  // Print the bottom border
  os << "+";
  for (std::size_t col = 0; col < numCols; ++col) {
    os << std::string(colWidths[col] + 2, '-');
    os << "+";
  }
  os << '\n';
  os << std::flush;
}
} // namespace util
