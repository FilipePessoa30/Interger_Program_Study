#include <ilcplex/ilocplex.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>

ILOSTLBEGIN

struct Grafo
{
    int n;
    int m;
    std::vector<std::pair<int, int>> arestas;
};

Grafo ler_base_dados(const std::string &arquivo)
{
    std::ifstream file(arquivo);

    Grafo grafo;
    std::string linha;
    std::getline(file, linha);
    std::istringstream iss(linha);
    std::string p, edge;
    iss >> p >> edge >> grafo.n >> grafo.m;

    while (std::getline(file, linha))
    {
        std::istringstream iss(linha);
        char e;
        int v1, v2;
        iss >> e >> v1 >> v2;
        grafo.arestas.emplace_back(v1 - 1, v2 - 1);
    }

    return grafo;
}

class UserCutCallback : public IloCplex::UserCutCallbackI
{
private:
    IloArray<IloBoolVarArray> x;
    IloBoolVarArray w;
    const Grafo &grafo;
    int numCortes;

public:
    UserCutCallback(IloEnv env, IloArray<IloBoolVarArray> &x, IloBoolVarArray &w, const Grafo &grafo)
        : IloCplex::UserCutCallbackI(env), x(x), w(w), grafo(grafo), numCortes(0) {}

    IloCplex::CallbackI *duplicateCallback() const override
    {
        return (new (getEnv()) UserCutCallback(*this));
    }

    void separarCortes()
    {
        for (int j = 0; j < grafo.n; ++j)
        {
            std::vector<int> clique = encontrarCliqueHeuristica(j);
            if (!clique.empty())
            {
                IloExpr lhs(getEnv());
                for (int i : clique)
                {
                    lhs += x[i][j];
                }
                lhs -= w[j];
                add(lhs <= 0).end();
                ++numCortes;
            }
        }
    }

    void main() override
    {
        separarCortes();
    }

    int getNumCortes() const
    {
        return numCortes;
    }

    std::vector<int> encontrarCliqueHeuristica(int j)
    {
        std::vector<int> clique;
        std::vector<int> vertices(grafo.n);
        std::iota(vertices.begin(), vertices.end(), 0);

        std::sort(vertices.begin(), vertices.end(), [&](int a, int b)
                  { return getValue(x[a][j]) > getValue(x[b][j]); });

        for (int v : vertices)
        {
            bool podeAdicionar = true;
            for (int u : clique)
            {
                if (std::find(grafo.arestas.begin(), grafo.arestas.end(), std::make_pair(v, u)) == grafo.arestas.end() &&
                    std::find(grafo.arestas.begin(), grafo.arestas.end(), std::make_pair(u, v)) == grafo.arestas.end())
                {
                    podeAdicionar = false;
                    break;
                }
            }
            if (podeAdicionar)
            {
                clique.push_back(v);
            }
        }

        return clique;
    }
};

void resolver_grafo(const Grafo &grafo, bool usarCortes)
{
    IloEnv env;
    try
    {
        IloModel model(env);

        int n = grafo.n;
        int m = grafo.m;

        // Variáveis de decisão
        IloArray<IloBoolVarArray> x(env, n);
        for (int i = 0; i < n; ++i)
        {
            x[i] = IloBoolVarArray(env, n);
        }
        IloBoolVarArray w(env, n);

        // Função objetivo: minimizar o número de cores usadas
        IloExpr obj(env);
        for (int j = 0; j < n; ++j)
        {
            obj += w[j];
        }
        model.add(IloMinimize(env, obj));

        // Restrição (2): Cada vértice recebe exatamente uma cor
        for (int i = 0; i < n; ++i)
        {
            IloExpr sum(env);
            for (int j = 0; j < n; ++j)
            {
                sum += x[i][j];
            }
            model.add(sum == 1);
        }

        // Restrição (3): Arestas não podem ter vértices com a mesma cor
        for (const auto &aresta : grafo.arestas)
        {
            int i = aresta.first;
            int k = aresta.second;
            for (int j = 0; j < n; ++j)
            {
                model.add(x[i][j] + x[k][j] <= w[j]);
            }
        }

        // Restrição (4): Ordem das cores
        for (int j = 0; j < n - 1; ++j)
        {
            model.add(w[j] >= w[j + 1]);
        }

        // Restrição (5): A cor j é usada se algum vértice a possui
        for (int j = 0; j < n; ++j)
        {
            IloExpr sum(env);
            for (int i = 0; i < n; ++i)
            {
                sum += x[i][j];
            }
            model.add(w[j] <= sum);
        }

        IloCplex cplex(model);

        // Parâmetros do CPLEX
        cplex.setParam(IloCplex::Param::TimeLimit, 100);
        cplex.setParam(IloCplex::Param::MIP::Limits::TreeMemory, 8192);
        cplex.setParam(IloCplex::Param::Threads, 10);
        cplex.setParam(IloCplex::Param::Preprocessing::Presolve, 1);
        cplex.setParam(IloCplex::Param::MIP::Limits::CutsFactor, 1);
        cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, 1);

        UserCutCallback *callback = nullptr;
        if (usarCortes)
        {
            callback = new (env) UserCutCallback(env, x, w, grafo);
            cplex.use(callback);
        }

        auto start = std::chrono::high_resolution_clock::now();
        cplex.solve();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tempo = end - start;

        std::cout << "Número cromático: " << cplex.getObjValue() << std::endl;
        std::cout << "Tempo total: " << tempo.count() << " segundos" << std::endl;
        if (usarCortes && callback)
        {
            std::cout << "Número de cortes gerados: " << callback->getNumCortes() << std::endl;
        }
    }
    catch (IloException &e)
    {
        std::cerr << "Erro de CPLEX: " << e.getMessage() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
    }
    env.end();
}

int main()
{
    try
    {
        Grafo grafo = ler_base_dados("TPI_BC_COL_0.txt");

        bool usarCortes;
        std::cout << "Deseja usar cortes de clique? (1 - Sim, 0 - Não): ";
        std::cin >> usarCortes;

        resolver_grafo(grafo, usarCortes);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
    }
    return 0;
}
