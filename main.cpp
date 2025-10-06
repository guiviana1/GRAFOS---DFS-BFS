#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
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
        std::cout << "Grafo dirigido lido.\n";
        // for (auto e : boost::make_iterator_range(edges(g)))
        //     std::cout << g[source(e,g)].name << " -> " << g[target(e,g)].name
        //               << " label=" << g[e].label << "\n";

        std::map<std::string, std::set<std::string>> listaadj;
        for (auto e : boost::make_iterator_range(edges(g))) {
            const std::string& u = g[source(e, g)].name;
            const std::string& v = g[target(e, g)].name;
            listaadj[u].insert(v); 
        }

        // Inicio da BFS

        auto index = get(boost::vertex_index, g);

        std::vector<char> visitado(num_vertices(g), 0);
        std::vector<int>  tin(num_vertices(g), -1);
        std::vector<int>  tout(num_vertices(g), -1);
        int tempo = 0;

        for (auto v : boost::make_iterator_range(vertices(g))) {
            if (visitado[index[v]]) {
                continue;
            }

            std::queue<UG::vertex_descriptor> fila;

            visitado[index[v]] = 1;
            tin[index[v]] = ++tempo;
            fila.push(v);

            std::cout << "Componente (BFS): ";
            while (!fila.empty()) {
                auto atual = fila.front();
                fila.pop();
                std::cout << g[atual].name << " ";

                for (auto e : boost::make_iterator_range(out_edges(atual, g))) {
                    auto w = target(e, g);
                    if (!visitado[index[w]]) {
                        visitado[index[w]] = 1;
                        tin[index[w]] = ++tempo;
                        fila.push(w);
                    }
                }

                tout[index[atual]] = ++tempo;
            }
            std::cout << "\n";
        }

        std::cout << "\nTempos (descoberta/finalização):\n";
        for (auto v : boost::make_iterator_range(vertices(g))) {
            std::cout << std::left << std::setw(12) << g[v].name
                    << " d=" << tin[index[v]] << " f=" << tout[index[v]] << "\n";
        }

    } else {
        UG g;
        boost::dynamic_properties dp(boost::ignore_other_properties);
        dp.property("node_id", get(&V::name, g));
        dp.property("label",   get(&V::label, g));
        dp.property("label",   get(&E::label, g));
        if (!boost::read_graphviz(in, g, dp)) { std::cerr << "falha DOT\n"; return 2; }
        std::cout << "Grafo não dirigido lido.\n";
        // for (auto e : boost::make_iterator_range(edges(g)))
        //     std::cout << g[source(e,g)].name << " -- " << g[target(e,g)].name
        //               << " label=" << g[e].label << "\n";

        std::map<std::string, std::set<std::string>> listaadj;

        for (auto e : boost::make_iterator_range(edges(g))) {
            const std::string& u = g[source(e, g)].name;
            const std::string& v = g[target(e, g)].name;
            listaadj[u].insert(v); 
            listaadj[v].insert(u);
        }

        for (const auto& [u, vizinhos] : listaadj) {
            std::cout << u << ":";
            for (const auto& v : vizinhos) {
                std::cout << " " << v;
            }
            std::cout << "\n";
        }

        // Inicio da BFS

        auto index = get(boost::vertex_index, g);

        std::vector<char> visitado(num_vertices(g), 0);
        std::vector<int>  tin(num_vertices(g), -1);
        std::vector<int>  tout(num_vertices(g), -1);
        int tempo = 0;

        for (auto v : boost::make_iterator_range(vertices(g))) {
            if (visitado[index[v]]) {
                continue;
            }

            std::queue<UG::vertex_descriptor> fila;

            visitado[index[v]] = 1;
            tin[index[v]] = ++tempo;
            fila.push(v);

            std::cout << "Componente (BFS): ";
            while (!fila.empty()) {
                auto atual = fila.front();
                fila.pop();
                std::cout << g[atual].name << " ";

                for (auto e : boost::make_iterator_range(out_edges(atual, g))) {
                    auto w = target(e, g);
                    if (!visitado[index[w]]) {
                        visitado[index[w]] = 1;
                        tin[index[w]] = ++tempo;
                        fila.push(w);
                    }
                }

                tout[index[atual]] = ++tempo;
            }
            std::cout << "\n";
        }

        std::cout << "\nTempos (descoberta/finalização):\n";
        for (auto v : boost::make_iterator_range(vertices(g))) {
            std::cout << std::left << std::setw(12) << g[v].name
                    << " d=" << tin[index[v]] << " f=" << tout[index[v]] << "\n";
        }
    }
}
