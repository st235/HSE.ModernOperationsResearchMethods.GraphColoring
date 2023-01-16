#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <time.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace {

double RoundTo(double value, double precision = 1.0) {
    return std::round(value / precision) * precision;
}

template<typename T>
std::string ListToString(const std::vector<T>& vector, char delimiter = ' ') {
    std::ostringstream os;
    for (const auto& item: vector) {
        os << item;
        os << delimiter;
    }
    return os.str();
}

//  This is DSatur implementation.
//
//  It uses the same greedy technique to color vertices
//  with the smallest possible color value. If it fails
//  it initialises new color.
//  Heuristic for vertices ordering is a vertex saturation.
//  Saturation is a number characterising amount of different colors
//  in the local neighbourhood, ie within the adjacent vertices.
//
//  A small heads-up: I will use Google's C++ Code Style
//  and will refactor the code according to it:
//  https://google.github.io/styleguide/cppguide.html
class ColoringProblem {
private:
    static constexpr int32_t kColorNoColor = -1;
    static constexpr int32_t kColorInitial = 0;

    struct Node {
    public:
        uint32_t id;
        uint32_t saturation;
        uint32_t uncolored_neighborhood_degree;

        Node(uint32_t id, uint32_t saturation, uint32_t uncolored_neighborhood_degree):
            id(id),
            saturation(saturation),
            uncolored_neighborhood_degree(uncolored_neighborhood_degree) {
            // empty on purpose
        }

        Node(const Node& that):
            id(that.id),
            saturation(that.saturation),
            uncolored_neighborhood_degree(that.uncolored_neighborhood_degree) {
            // empty on purpose
        }

        Node& operator=(const Node& that) {
            if (this != &that) {
                this->id = that.id;
                this->saturation = that.saturation;
                this->uncolored_neighborhood_degree = that.uncolored_neighborhood_degree;
            }

            return *this;
        }
    };

    struct NodeSaturationComparator {
    public:
        bool operator()(const Node& lhs, const Node& rhs) const {
            if (lhs.saturation == rhs.saturation) {
                // should be strict comparison in all occurences,
                // otherwise a collection won't be able to find equal elements
                if (lhs.uncolored_neighborhood_degree == rhs.uncolored_neighborhood_degree) {
                    return lhs.id > rhs.id;
                }

                return lhs.uncolored_neighborhood_degree > rhs.uncolored_neighborhood_degree;
            }

            return lhs.saturation > rhs.saturation;
        }
    };

    struct NodeDegreeComparator {
    public:
        bool operator()(const std::pair<uint32_t, uint32_t>& lhs,
                        const std::pair<uint32_t, uint32_t>& rhs) const {
            if (lhs.second == rhs.second) {
                return lhs.first > rhs.first;
            }

            return lhs.second > rhs.second;
        }
    };

    std::vector<int32_t> colors_;
    uint32_t max_color_ = static_cast<uint32_t>(kColorInitial);
    std::vector<std::unordered_set<uint32_t>> graph_;

public:
    inline uint32_t GetNumberOfColors() {
        // as max_color is the last seen color
        // we need to add + 1 to count them
        // as we start coloring them from 0
        return max_color_ + 1;
    }

    const inline std::vector<int32_t>& GetColors() {
        return colors_;
    }

    void ReadGraphFile(const std::string& filename) {
        std::ifstream fin(filename);
        std::string line;
        int vertices = 0, edges = 0;
        while (std::getline(fin, line)) {
            // 'c' stands for comments,
            // therefore skipping
            if (line[0] == 'c') {
                continue;
            }

            std::stringstream line_input(line);
            char command;
            if (line[0] == 'p') {
                std::string type;
                line_input >> command >> type >> vertices >> edges;
                graph_.resize(vertices);
                colors_.resize(vertices + 1);
            } else {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                graph_[start - 1].insert(finish - 1);
                graph_[finish - 1].insert(start - 1);
            }
        }
    }

    void GreedyGraphColoring() {
        RFL();
    }

    // Recursive-Large-First algorithm
    void RFL() {
        int32_t ongoing_color = kColorInitial;
        std::unordered_map<uint32_t, uint32_t> vertex_to_degree_lookup;
        std::set<std::pair<uint32_t, uint32_t>, NodeDegreeComparator> queue;

        std::vector<uint32_t> vertices_degrees(graph_.size());

        std::vector<std::unordered_set<int32_t>> adjacent_colors(graph_.size());

        for (size_t i = 0; i < graph_.size(); i++) {
            const auto& adjacent_vertices = graph_[i];

            colors_[i] = kColorNoColor;

            uint32_t vertex = static_cast<uint32_t>(i);
            uint32_t degree = adjacent_vertices.size();

            queue.insert(std::make_pair(
                vertex,
                degree
            ));

            vertex_to_degree_lookup[vertex] = degree;
        }

        while (!queue.empty()) {
            // std::cout << "iter: " << ongoing_color + 1 << std::endl;
            const auto& iterator = queue.begin();
            std::pair<uint32_t, uint32_t> node = *iterator;
            queue.erase(node);

            std::unordered_set<uint32_t> s;
            std::unordered_set<uint32_t> adj_to_s;
            std::unordered_set<uint32_t> uncolored_subset;

            s.insert(node.first);
            for (const auto& av: graph_[node.first]) {
                adj_to_s.insert(av);
            }
            colors_[node.first] = ongoing_color;
            // std::cout << "first: " << node.first << std::endl;

            // add all uncolored vertices
            for (uint32_t v = 0; v < graph_.size(); v++) {
                bool is_adj_to_s = adj_to_s.find(v) != adj_to_s.end();
                if (colors_[v] == kColorNoColor && !is_adj_to_s) {
                    uncolored_subset.insert(v);
                }
            }

            while (!uncolored_subset.empty()) {
                int32_t y_vertex = -1;
                int32_t yadj = std::numeric_limits<int32_t>::min();
                int32_t ynadj = std::numeric_limits<int32_t>::max();

                for (const auto& v: uncolored_subset) {
                    int32_t adj_to_s_neighbours_count = 0;
                    int32_t not_adj_to_s_neighbours_count = 0;

                    for (const auto nv: graph_[v]) {
                        if (adj_to_s.find(nv) != adj_to_s.end()) {
                            adj_to_s_neighbours_count += 1;
                        } else {
                            not_adj_to_s_neighbours_count += 1;
                        }
                    }

                    if ((adj_to_s_neighbours_count > yadj) || (adj_to_s_neighbours_count == yadj && not_adj_to_s_neighbours_count < ynadj)) {
                        y_vertex = v;
                        yadj = adj_to_s_neighbours_count;
                        ynadj = not_adj_to_s_neighbours_count;
                    }
                }

                // std::cout << "others: " << y_vertex << std::endl;

                if (y_vertex == -1) {
                    break;
                }

                queue.erase(std::make_pair(
                    y_vertex,
                    vertex_to_degree_lookup[y_vertex]
                ));

                s.insert(y_vertex);
                uncolored_subset.erase(y_vertex);
                for (const auto& av: graph_[y_vertex]) {
                    adj_to_s.insert(av);
                    uncolored_subset.erase(av);
                }
                colors_[y_vertex] = ongoing_color;
            }
        
            ongoing_color += 1;
        }

        max_color_ = ongoing_color - 1;
    }

    // void GreedyGraphColoring() {
    //     std::set<Node, NodeSaturationComparator> queue;

    //     std::vector<uint32_t> vertices_degrees(graph_.size());

    //     std::vector<std::unordered_set<int32_t>> adjacent_colors(graph_.size());

    //     for (size_t i = 0; i < graph_.size(); i++) {
    //         const auto& adjacent_vertices = graph_[i];

    //         // let's reset all colors to kColorNoColor
    //         colors_[i] = kColorNoColor;
    //         vertices_degrees[i] = adjacent_vertices.size();

    //         queue.insert(Node(static_cast<uint32_t>(i) /* id */, 
    //                           static_cast<uint32_t>(adjacent_colors[i].size()) /* saturation */,
    //                           vertices_degrees[i] /* uncolored_neighborhood_degree */ ));
    //     }

    //     while (!queue.empty()) {
    //         const auto queue_iterator = queue.begin();
    //         Node node = *queue_iterator;
    //         queue.erase(queue_iterator);

    //         int32_t current_color = kColorNoColor;
    //         std::vector<bool> available_colors(colors_.size(), true);
    //         for (const auto& neighbour: graph_[node.id]) {
    //             int32_t color = colors_[neighbour];
    //             if (color != kColorNoColor) {
    //                 available_colors[color] = false;
    //             }
    //         }
    //         for (size_t color = 0; color < available_colors.size(); color++) {
    //             if (available_colors[color]) {
    //                 current_color = color;
    //                 break;
    //             }
    //         }

    //         colors_[node.id] = current_color;
    //         max_color_ = std::max(max_color_, static_cast<uint32_t>(current_color));

    //         for (const auto& neighbour: graph_[node.id]) {
    //             if (colors_[neighbour] != kColorNoColor) {
    //                 continue;
    //             }

    //             Node old_neighbour_state(static_cast<uint32_t>(neighbour) /* id */, 
    //                                      static_cast<uint32_t>(adjacent_colors[neighbour].size()) /* saturation */,
    //                                      vertices_degrees[neighbour] /* uncolored_neighborhood_degree */ );

    //             adjacent_colors[neighbour].insert(current_color);
    //             vertices_degrees[neighbour] -= 1;
    //             queue.erase(old_neighbour_state);

    //             Node new_neighbour_state(static_cast<uint32_t>(neighbour) /* id */, 
    //                                      static_cast<uint32_t>(adjacent_colors[neighbour].size()) /* saturation */,
    //                                      vertices_degrees[neighbour] /* uncolored_neighborhood_degree */ );

    //             queue.insert(new_neighbour_state);
    //         }
    //     }
    // }

    bool Check() {
        for (size_t i = 0; i < graph_.size(); ++i) {
            if (colors_[i] == kColorNoColor) {
                std::cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }

            for (int neighbour : graph_[i]) {
                if (colors_[neighbour] == colors_[i]) {
                    std::cout << "Neighbour vertices " << i + 1 << ", " << neighbour + 1 <<  " have the same color\n";
                    return false;
                }
            }
        }

        return true;
    }
};

} // namespace

int main() {
    std::vector<std::string> files = { "myciel3.col", "myciel7.col", "latin_square_10.col", "school1.col", "school1_nsh.col",
        "mulsol.i.1.col", "inithx.i.1.col", "anna.col", "huck.col", "jean.col", "miles1000.col", "miles1500.col",
        "fpsol2.i.1.col", "le450_5a.col", "le450_15b.col", "le450_25a.col", "games120.col",
        "queen11_11.col", "queen5_5.col" };

    std::ofstream output_report("output_report.csv");
    std::ofstream colors_report("colors_report.txt");

    output_report << "Instance; Colors; Time (sec)\n";
    std::cout << std::setfill(' ') << std::setw(20) << "Instance" 
              << std::setfill(' ') << std::setw(10) << "Colors"
              << std::setfill(' ') << std::setw(15) << "Time, sec"
              << std::endl;

    for (const auto& file: files) {
        ColoringProblem problem;
        problem.ReadGraphFile("data/" + file);
        clock_t start = clock();
        problem.GreedyGraphColoring();
        if (!problem.Check()) {
            output_report << "*** WARNING: incorrect coloring: ***" << std::endl;
            std::cout << "*** WARNING: incorrect coloring: ***" << std::endl;
        }
        clock_t end = clock();
        clock_t ticks_diff = end - start;
        double seconds_diff = RoundTo(double(ticks_diff) / CLOCKS_PER_SEC, 0.001);

        colors_report << file << std::endl
                      << ListToString(problem.GetColors()) << std::endl
                      << std::endl;

        output_report << file << ";"
                      << problem.GetNumberOfColors() << ";"
                      << seconds_diff << std::endl;

        std::cout << std::setfill(' ') << std::setw(20) << file
                << std::setfill(' ') << std::setw(10) << problem.GetNumberOfColors()
                << std::setfill(' ') << std::setw(15) << seconds_diff
                << std::endl;
    }

    colors_report.close();
    output_report.close();
    return 0;
}
