#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <queue>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

struct V { std::string name, label; };
struct E { std::string label; };

using DG = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, V, E>;
using UG = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, V, E>;

bool is_digraph(std::istream& is) {
    std::streampos p = is.tellg();
    std::string first_line;
    std::getline(is, first_line);
    is.clear();              
    is.seekg(p);              
    for (auto& c : first_line) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return first_line.find("digraph") != std::string::npos;
}

std::vector<int> bfs_levels_from_matrix(const std::vector<std::vector<int>>& matriz, int s) {
            int n = (int)matriz.size();
            const int INF = INT_MAX;
            std::vector<int> dist(n, INF);
            std::queue<int> q;
            dist[s] = 0;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v = 0; v < n; ++v) {
                    if (matriz[u][v] != 0 && dist[v] == INF) {
                        dist[v] = dist[u] + 1;
                        q.push(v);
                    }
                }
            }
            return dist; 
        }

void print_levels(const std::vector<int>& dist, const std::vector<std::string>& nomes = {}) {
    const int INF = INT_MAX;
    int maxd = -1;
    for (int d : dist) if (d != INF) maxd = std::max(maxd, d);
    for (int k = 0; k <= maxd; ++k) {
        std::cout << "Camada " << k << ": ";
        for (int i = 0; i < (int)dist.size(); ++i)
            if (dist[i] == k)
                std::cout << (nomes.empty() ? std::to_string(i) : nomes[i]) << " ";
        std::cout << "\n";
    }
}

int main() {
    std::string caminho;
    std::cout << "Insira o nome do arquivo a ser lido: ";
    std::cin >> caminho;

    std::ifstream in(caminho);
    if (!in) { std::cerr << caminho << " não existe\n"; return 1; }
    
    bool directed = is_digraph(in);

    in.clear();
    in.seekg(0);

    if (directed) {
        DG g;
        boost::dynamic_properties dp(boost::ignore_other_properties);
        dp.property("node_id", get(&V::name, g));
        dp.property("label",   get(&V::label, g));
        dp.property("label",   get(&E::label, g));
        if (!boost::read_graphviz(in, g, dp)) { std::cerr << "falha DOT\n"; return 2; }
        std::cout << "Grafo dirigido lido.\n\n";

        std::set<std::string> s;
        for (auto e : boost::make_iterator_range(edges(g))){
            s.insert(g[source(e,g)].name);
            s.insert(g[target(e,g)].name);
        }

        int n = s.size();
        std::vector<std::vector<int>> matriz(n, std::vector<int>(n, 0));
        std::vector<std::string> nomes(s.begin(), s.end());
        std::unordered_map<std::string,int> idx;
        idx.reserve(nomes.size());
        for (int i = 0; i < (int)nomes.size(); ++i) idx[nomes[i]] = i;

        // preencher a matriz de adjacência
        for (auto e : boost::make_iterator_range(edges(g))) {
            const std::string& su = g[source(e,g)].name;
            const std::string& sv = g[target(e,g)].name;
            int iu = idx[su];
            int iv = idx[sv];
            matriz[iu][iv] = 1;
        }

        std::vector<int> dist = bfs_levels_from_matrix(matriz, 0);
        std::cout << "Matriz de adjacência:\n";
        for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            std::cout << matriz[i][j] << " ";
            std::cout << "\n";
        }
        std::cout << "\n";
        std::cout << "Vértices a partir da primeira camada:\n";
        print_levels(dist, nomes); 
        
    } else {
        UG g;
        boost::dynamic_properties dp(boost::ignore_other_properties);
        dp.property("node_id", get(&V::name, g));
        dp.property("label",   get(&V::label, g));
        dp.property("label",   get(&E::label, g));
        if (!boost::read_graphviz(in, g, dp)) { std::cerr << "falha DOT\n"; return 2; }
        std::cout << "Grafo não dirigido lido.\n\n";

        std::set<std::string> s;
        for (auto e : boost::make_iterator_range(edges(g))){
            s.insert(g[source(e,g)].name);
            s.insert(g[target(e,g)].name);
        }

        int n = s.size();
        std::vector<std::vector<int>> matriz(n, std::vector<int>(n, 0));
        std::vector<std::string> nomes(s.begin(), s.end());
        std::unordered_map<std::string,int> idx;
        idx.reserve(nomes.size());
        for (int i = 0; i < (int)nomes.size(); ++i) idx[nomes[i]] = i;

        // preencher matriz simétrica para grafo não direcionado
        for (auto e : boost::make_iterator_range(edges(g))) {
        const std::string& su = g[source(e,g)].name;
        const std::string& sv = g[target(e,g)].name;
        int iu = idx[su];
        int iv = idx[sv];
        matriz[iu][iv] = 1;
        matriz[iv][iu] = 1; // simetria no não direcionado
        }
        
        std::vector<int> dist = bfs_levels_from_matrix(matriz, 0);
        std::cout << "Matriz de adjacência:\n";
        for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            std::cout << matriz[i][j] << " ";
            std::cout << "\n";
        }
        std::cout << "\n";
        std::cout << "Vértices a partir da primeira camada:\n";
        print_levels(dist, nomes); 
    }
}