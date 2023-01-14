#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <time.h>

using namespace std;

class ColoringProblem {
public:
    int GetRandom(int a, int b) {
        static mt19937 generator;
        uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(string filename) {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }

            stringstream line_input(line);
            char command;
            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
                colors.resize(vertices + 1);
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
    }

    void GreedyGraphColoring()
    {
        vector<int> uncolored_vertices(neighbour_sets.size());
        for (size_t i = 0; i < uncolored_vertices.size(); ++i)
            uncolored_vertices[i] = i;

        while (! uncolored_vertices.empty())
        {
            int index = GetRandom(0, uncolored_vertices.size() - 1);
            int vertex = uncolored_vertices[index];
            int color = GetRandom(1, maxcolor);
            for (int neighbour : neighbour_sets[vertex])
            {
                if (color == colors[neighbour])
                {
                    color = ++maxcolor;
                    break;
                }
            }
            colors[vertex] = color;
            // Move the colored vertex to the end and pop it
            swap(uncolored_vertices[uncolored_vertices.size() - 1], uncolored_vertices[index]);
            uncolored_vertices.pop_back();
        }
    }

    bool Check()
    {
        for (size_t i = 0; i < neighbour_sets.size(); ++i)
        {
            if (colors[i] == 0)
            {
                cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }
            for (int neighbour : neighbour_sets[i])
            {
                if (colors[neighbour] == colors[i])
                {
                    cout << "Neighbour vertices " << i + 1 << ", " << neighbour + 1 <<  " have the same color\n";
                    return false;
                }
            }
        }
        return true;
    }

    int GetNumberOfColors()
    {
        return maxcolor;
    }

    const vector<int>& GetColors()
    {
        return colors;
    }

private:
    vector<int> colors;
    int maxcolor = 1;
    vector<unordered_set<int>> neighbour_sets;
};

int main() {
    vector<string> files = { "myciel3.col", "myciel7.col", "latin_square_10.col", "school1.col", "school1_nsh.col",
        "mulsol.i.1.col", "inithx.i.1.col", "anna.col", "huck.col", "jean.col", "miles1000.col", "miles1500.col",
        "fpsol2.i.1.col", "le450_5a.col", "le450_15b.col", "le450_25a.col", "games120.col",
        "queen11_11.col", "queen5_5.col" };

    ofstream fout("color.csv");
    fout << "Instance; Colors; Time (sec)\n";
    cout << "Instance; Colors; Time (sec)\n";

    for (string file : files) {
        ColoringProblem problem;
        problem.ReadGraphFile("data/" + file);
        clock_t start = clock();
        problem.GreedyGraphColoring();
        if (!problem.Check()) {
            fout << "*** WARNING: incorrect coloring: ***\n";
            cout << "*** WARNING: incorrect coloring: ***\n";
        }
        fout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << '\n';
        cout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << '\n';
    }

    fout.close();
    return 0;
}
