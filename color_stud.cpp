#include <algorithm>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <unordered_set>
#include <time.h>

namespace {

class ColoringProblem {
private:
    static constexpr uint32_t INITIAL_COLOR = 1U;

    std::vector<uint32_t> _colors;
    uint32_t _maxcolor = INITIAL_COLOR;
    std::vector<std::unordered_set<int>> _neighbour_sets;

public:
    int GetRandom(int a, int b) {
        static std::mt19937 generator;
        std::uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(const std::string& filename) {
        std::ifstream fin(filename);
        std::string line;
        int vertices = 0, edges = 0;
        while (std::getline(fin, line))
        {
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
                _neighbour_sets.resize(vertices);
                _colors.resize(vertices + 1);
            } else {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                _neighbour_sets[start - 1].insert(finish - 1);
                _neighbour_sets[finish - 1].insert(start - 1);
            }
        }
    }

    void GreedyGraphColoring() {
        std::vector<int> uncolored_vertices(_neighbour_sets.size());
        for (size_t i = 0; i < uncolored_vertices.size(); ++i) {
            uncolored_vertices[i] = i;
        }

        while (! uncolored_vertices.empty()) {
            int index = GetRandom(0, uncolored_vertices.size() - 1);
            int vertex = uncolored_vertices[index];
            int color = GetRandom(1, _maxcolor);
            for (int neighbour : _neighbour_sets[vertex]) {
                if (color == _colors[neighbour]) {
                    color = ++_maxcolor;
                    break;
                }
            }
            _colors[vertex] = color;
            // Move the colored vertex to the end and pop it
            std::swap(uncolored_vertices[uncolored_vertices.size() - 1], uncolored_vertices[index]);
            uncolored_vertices.pop_back();
        }
    }

    bool Check() {
        for (size_t i = 0; i < _neighbour_sets.size(); ++i) {
            if (_colors[i] == 0) {
                std::cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }

            for (int neighbour : _neighbour_sets[i]) {
                if (_colors[neighbour] == _colors[i]) {
                    std::cout << "Neighbour vertices " << i + 1 << ", " << neighbour + 1 <<  " have the same color\n";
                    return false;
                }
            }
        }

        return true;
    }

    int GetNumberOfColors() {
        return _maxcolor;
    }

    const std::vector<uint32_t>& GetColors() {
        return _colors;
    }
};

} // namespace

int main() {
    std::vector<std::string> files = { "myciel3.col", "myciel7.col", "latin_square_10.col", "school1.col", "school1_nsh.col",
        "mulsol.i.1.col", "inithx.i.1.col", "anna.col", "huck.col", "jean.col", "miles1000.col", "miles1500.col",
        "fpsol2.i.1.col", "le450_5a.col", "le450_15b.col", "le450_25a.col", "games120.col",
        "queen11_11.col", "queen5_5.col" };

    std::ofstream fout("color.csv");
    fout << "Instance; Colors; Time (sec)\n";
    std::cout << "Instance; Colors; Time (sec)\n";

    for (const auto& file: files) {
        ColoringProblem problem;
        problem.ReadGraphFile("data/" + file);
        clock_t start = clock();
        problem.GreedyGraphColoring();
        if (!problem.Check()) {
            fout << "*** WARNING: incorrect coloring: ***\n";
            std::cout << "*** WARNING: incorrect coloring: ***\n";
        }
        fout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << '\n';
        std::cout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << '\n';
    }

    fout.close();
    return 0;
}
