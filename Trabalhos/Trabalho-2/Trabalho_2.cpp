#include <ilcplex/ilocplex.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

ILOSTLBEGIN

struct Grafo
{
    int n;                                    // Número de vértices
    int m;                                    // Número de arestas
    std::vector<std::pair<int, int>> arestas; // Lista de arestas
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

void resolver_grafo(const Grafo &grafo)
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

        // Função objetivo
        IloExpr obj(env);
        for (int j = 0; j < n; ++j)
        {
            obj += w[j];
        }
        model.add(IloMinimize(env, obj));

        // Restrição: Cada vértice recebe exatamente uma cor
        for (int i = 0; i < n; ++i)
        {
            IloExpr sum(env);
            for (int j = 0; j < n; ++j)
            {
                sum += x[i][j];
            }
            model.add(sum == 1);
        }

        // Restrição: Arestas não podem ter vértices com mesma cor
        for (const auto &aresta : grafo.arestas)
        {
            int i = aresta.first;
            int k = aresta.second;
            for (int j = 0; j < n; ++j)
            {
                model.add(x[i][j] + x[k][j] <= w[j]);
            }
        }

        // Restrição: Quebra de simetria
        for (int j = 0; j < n - 1; ++j)
        {
            model.add(w[j] >= w[j + 1]);
        }

        // Restrição: Restrição valida que fortalece
        for (int j = 0; j < n; ++j)
        {
            IloExpr sum(env);
            for (int i = 0; i < n; ++i)
            {
                sum += x[i][j];
            }
            model.add(w[j] <= sum);
        }

        // Resolver o modelo
        IloCplex cplex(model);
        // //Geração do modelo
        // cplex.exportModel("file_name.lp");
        cplex.solve();

        // Imprimir resultados
        for (int j = 0; j < n; ++j)
        {
            if (cplex.getValue(w[j]) > 0.5)
            {
                std::cout << "Cor " << j + 1 << " usada." << std::endl;
            }
        }
        std::cout << "Melhor resultado: " << cplex.getObjValue() << std::endl;

        env.out() << "Status da solução = " << cplex.getStatus() << endl;
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
        Grafo grafo = ler_base_dados("TPI_COL_3.txt");
        resolver_grafo(grafo);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
    }
    return 0;
}
