#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <stack>     
#include <vector>    
#include <cctype>    
#include <iomanip>   
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

        std::map<std::string, std::set<std::string>> listaadj;

        for (auto e : boost::make_iterator_range(edges(g))) {
            const std::string& u = g[source(e, g)].name;
            const std::string& v = g[target(e, g)].name;
            listaadj[u].insert(v); 
            listaadj[v].insert(u);
        }

        // ===================== Início da DFS =====================
        auto index = get(boost::vertex_index, g);
        const std::size_t n = num_vertices(g);

        std::vector<char> visitado(n, 0);
        std::vector<int>  pred(n, -1);
        std::vector<int>  tin(n, -1), tout(n, -1);

        int tempo = 0;
        std::stack<DG::vertex_descriptor> P;

        for (auto s : boost::make_iterator_range(vertices(g))) {
            if (visitado[index[s]]) continue;

            visitado[index[s]] = 1;
            tin[index[s]] = ++tempo;
            pred[index[s]] = -1;
            P.push(s);

            while (!P.empty()) {
                auto u = P.top();

                bool empilhou_vizinho = false;
                for (auto e : boost::make_iterator_range(out_edges(u, g))) {
                    auto v = target(e, g);
                    if (!visitado[index[v]]) {
                        visitado[index[v]] = 1;
                        pred[index[v]] = static_cast<int>(index[u]);
                        tin[index[v]] = ++tempo;     
                        P.push(v);                   
                        empilhou_vizinho = true;
                        break;                       
                    }
                }

                if (!empilhou_vizinho) {
                    P.pop();
                    if (tout[index[u]] == -1) {
                        tout[index[u]] = ++tempo;
                    }
                }
            }
        }
        // ====================== Fim da DFS ======================

        std::cout << "\nTempos (descoberta/finalização):\n";
        for (auto v : boost::make_iterator_range(vertices(g))) {
            std::cout << std::left << std::setw(12) << g[v].name
                      << " d=" << tin[index[v]] << " f=" << tout[index[v]] << "\n";
        }

        std::vector<DG::vertex_descriptor> ordem;
        for (auto v : boost::make_iterator_range(vertices(g)))
            if (tin[index[v]] != -1) ordem.push_back(v);

        std::sort(ordem.begin(), ordem.end(),
                [&](auto a, auto b){ return tin[index[a]] < tin[index[b]]; });

        std::cout << "\nOrdem de descoberta (DFS): ";
        for (auto v : ordem) std::cout << g[v].name << ' ';
        std::cout << "\n";

        // ---------- DFS: Pai -> Filhos ----------
        using VDesc = decltype(*vertices(g).first);

        std::vector<std::vector<VDesc>> filhos(n);
        for (auto v : boost::make_iterator_range(vertices(g))) {
            int p = pred[index[v]];
            if (p != -1) {
                auto pai = vertex(static_cast<std::size_t>(p), g);
                filhos[p].push_back(v);
            }
        }

        for (auto& vec : filhos) {
            std::sort(vec.begin(), vec.end(),
                    [&](VDesc a, VDesc b){ return tin[index[a]] < tin[index[b]]; });
        }

        std::cout << "\nDFS (pai -> filhos):\n";
        for (auto v : ordem) {
            std::cout << g[v].name << " -> {";
            const auto& ch = filhos[index[v]];
            for (std::size_t i = 0; i < ch.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << g[ch[i]].name;
            }
            std::cout << "}\n";
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

        // ===================== Início da DFS =====================
        auto index = get(boost::vertex_index, g);
        const std::size_t n = num_vertices(g);

        std::vector<char> visitado(n, 0);
        std::vector<int>  pred(n, -1);
        std::vector<int>  tin(n, -1), tout(n, -1);

        int tempo = 0;
        std::stack<DG::vertex_descriptor> P;

        for (auto s : boost::make_iterator_range(vertices(g))) {
            if (visitado[index[s]]) continue;

            visitado[index[s]] = 1;
            tin[index[s]] = ++tempo;
            pred[index[s]] = -1;
            P.push(s);

            while (!P.empty()) {
                auto u = P.top();

                bool empilhou_vizinho = false;
                for (auto e : boost::make_iterator_range(out_edges(u, g))) {
                    auto v = target(e, g);
                    if (!visitado[index[v]]) {
                        visitado[index[v]] = 1;
                        pred[index[v]] = static_cast<int>(index[u]);
                        tin[index[v]] = ++tempo;
                        P.push(v);                  
                        empilhou_vizinho = true;
                        break;                      
                    }
                }

                if (!empilhou_vizinho) {
                    P.pop();
                    if (tout[index[u]] == -1) {
                        tout[index[u]] = ++tempo;
                    }
                }
            }
        }
        // ====================== Fim da DFS ======================

        std::cout << "\nTempos (descoberta/finalização):\n";
        for (auto v : boost::make_iterator_range(vertices(g))) {
            std::cout << std::left << std::setw(12) << g[v].name
                      << " d=" << tin[index[v]] << " f=" << tout[index[v]] << "\n";
        }

        std::vector<DG::vertex_descriptor> ordem;
        for (auto v : boost::make_iterator_range(vertices(g)))
            if (tin[index[v]] != -1) ordem.push_back(v);

        std::sort(ordem.begin(), ordem.end(),
                [&](auto a, auto b){ return tin[index[a]] < tin[index[b]]; });

        std::cout << "\nOrdem de descoberta (DFS): ";
        for (auto v : ordem) std::cout << g[v].name << ' ';
        std::cout << "\n";

        // ---------- DFS: Pai -> Filhos ----------
        using VDesc = decltype(*vertices(g).first);

        std::vector<std::vector<VDesc>> filhos(n);
        for (auto v : boost::make_iterator_range(vertices(g))) {
            int p = pred[index[v]];
            if (p != -1) {
                auto pai = vertex(static_cast<std::size_t>(p), g);
                filhos[p].push_back(v);
            }
        }

        for (auto& vec : filhos) {
            std::sort(vec.begin(), vec.end(),
                    [&](VDesc a, VDesc b){ return tin[index[a]] < tin[index[b]]; });
        }

        std::cout << "\nDFS (pai -> filhos):\n";
        for (auto v : ordem) {
            std::cout << g[v].name << " -> {";
            const auto& ch = filhos[index[v]];
            for (std::size_t i = 0; i < ch.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << g[ch[i]].name;
            }
            std::cout << "}\n";
        }

    }
}
