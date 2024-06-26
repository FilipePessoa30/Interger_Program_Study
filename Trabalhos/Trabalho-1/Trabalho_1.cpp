#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <vector>

ILOSTLBEGIN

int main()
{
    std::ifstream inputFile("TPI_F_2.txt");

    int ni, nj, c, Q, NL;
    inputFile >> ni >> nj >> c >> Q >> NL;

    // Serve para evitar tomar esse caminho, uma vez, que é um problema de minimização e zero é um valor muito baixo
    // Valor do big_M, ou um valor suficientemente alto como 1000000
    const int big_M = 1000000;

    // Leitura dos dados da segunda linha em colunas
    std::vector<std::vector<double>> tabela_valores(NL, std::vector<double>(4));
    for (int i = 0; i < NL; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            inputFile >> tabela_valores[i][j];
        }
    }

    // Impressão das variáveis para verificação
    std::cout << "Facilidade(i) e Clientes(j):" << std::endl;
    for (int i = 0; i < NL; ++i)
    {
        std::cout << "Facilidade " << tabela_valores[i][0] << ", Cliente " << tabela_valores[i][1] << std::endl;
    }

    std::cout << "Variáveis g_ij e p_ij:" << std::endl;
    for (int i = 0; i < NL; ++i)
    {
        std::cout << "g_" << tabela_valores[i][0] << tabela_valores[i][1] << " = " << tabela_valores[i][2] << ", ";
        std::cout << "p_" << tabela_valores[i][0] << tabela_valores[i][1] << " = " << tabela_valores[i][3] << std::endl;
    }

    // Criação do ambiente
    IloEnv env;
    try
    {
        // Criação do modelo
        IloModel model(env);

        // Criação das variáveis de decisão
        IloBoolVarArray y(env, ni);           // yi
        IloArray<IloBoolVarArray> x(env, ni); // xij
        for (int i = 0; i < ni; ++i)
        {
            x[i] = IloBoolVarArray(env, nj);
        }

        // Restrições
        // Para todo j em J, temos Σxij = 1
        for (int j = 0; j < nj; ++j)
        {
            IloExpr sum2(env);
            for (int i = 0; i < ni; ++i)
            {
                sum2 += x[i][j];
            }
            model.add(sum2 == 1); // Σxij = 1
            sum2.end();
        }

        // Para todo (i,j) em E, temos xij ≤ yi
        for (int i = 0; i < ni; ++i)
        {
            for (int j = 0; j < nj; ++j)
            {
                model.add(x[i][j] <= y[i]); // xij ≤ yi
            }
        }

        // Para todo i em I, temos Σpij * xij ≤ Q * yi
        for (int i = 0; i < ni; ++i)
        {
            IloExpr sum1(env);
            for (int j = 0; j < nj; ++j)
            {
                bool found = false;
                for (int k = 0; k < NL; ++k)
                {
                    if (tabela_valores[k][0] == i + 1 && tabela_valores[k][1] == j + 1)
                    {
                        sum1 += tabela_valores[k][3] * x[i][j]; // p_ij * x_ij
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    sum1 += 0;
                }
            }
            model.add(sum1 <= Q * y[i]);
            sum1.end();
        }

        // Função objetivo
        IloExpr objExpr(env);
        IloExpr objExprY(env); // c * y_i
        IloExpr objExprX(env); // g_ij * x_ij

        // Adiciona c * y_i
        for (int i = 0; i < ni; ++i)
        {
            objExprY += c * y[i];
        }

        // Adiciona g_ij * x_ij
        for (int i = 0; i < ni; ++i)
        {
            for (int j = 0; j < nj; ++j)
            {
                bool found = false;
                for (int k = 0; k < NL; ++k)
                {
                    if (tabela_valores[k][0] == i + 1 && tabela_valores[k][1] == j + 1)
                    {
                        objExprX += tabela_valores[k][2] * x[i][j];
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    objExprX += big_M * x[i][j];
                }
            }
        }

        // Soma as partes da função objetivo
        objExpr = objExprY + objExprX;
        model.add(IloMinimize(env, objExpr));

        // Finaliza as expressões
        objExpr.end();
        objExprY.end();
        objExprX.end();

        // Criação do solver
        IloCplex cplex(model);
        // Gerando o arquivo .lp, que contém o modelo, está comentado para não gerar o arquivo
        cplex.exportModel("modelo.lp");

        // Resolvendo o modelo
        cplex.solve();
        env.out() << "Status da solução = " << cplex.getStatus() << endl;

        // // Exibindo os valores das variáveis de decisão
        // std::cout << "Valor das variáveis y_i:" << std::endl;
        // for (int i = 0; i < ni; ++i)
        // {
        //     std::cout << "y_" << i << " = " << cplex.getValue(y[i]) << std::endl;
        // }

        // std::cout << "Valor das variáveis x_ij:" << std::endl;
        // for (int i = 0; i < ni; ++i)
        // {
        //     for (int j = 0; j < nj; ++j)
        //     {
        //         std::cout << "x_" << i << j << " = " << cplex.getValue(x[i][j]) << std::endl;
        //     }
        // }

        // Valor da função objetivo
        std::cout << "Valor ótimo da função objetivo: " << cplex.getObjValue() << std::endl;

        // Fechando o arquivo e liberando recursos
        inputFile.close();
        env.end();
    }
    catch (IloException &e)
    {
        std::cerr << "Erro: " << e << std::endl;
    }

    return 0;
}