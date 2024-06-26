Trabalho de Otimização com CPLEX


Requisitos
Sistema Operacional: Linux (x86-64)
Compilador: g++
IBM ILOG CPLEX Optimization Studio instalado nos diretórios especificados no Makefile
Estrutura do Projeto
Trabalho_1.cpp: Código fonte do programa de otimização.
Makefile: Arquivo para automatizar o processo de compilação.
Configuração
Instale o IBM ILOG CPLEX Optimization Studio:
Certifique-se de que o CPLEX está instalado nos diretórios especificados:

CPLEXDIR: /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERTDIR: /opt/ibm/ILOG/CPLEX_Studio2211/concert
Configure o ambiente:


Compilação e Execução

Clone o repositório ou copie os arquivos do projeto para o seu diretório de trabalho.

Abra um terminal e navegue até o diretório do projeto.

Compile o programa:

Execute o comando make para compilar o programa:

make
Execute o programa:

Após a compilação, execute o programa gerado:
./Trabalho_1

Limpeza:
Para remover os arquivos gerados pela compilação, use o comando:

make clean

Detalhes do Makefile
Compilador e Opções de Compilação:

CCC = g++ -O0: Usa o compilador g++ com otimização nível 0 (nenhuma otimização).
CCOPT = -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -Wno-ignored-attributes: Flags de compilação específicas.
Bibliotecas e Includes:

Define os diretórios de inclusão e bibliotecas para o CPLEX e Concert.
Regras do Makefile:

all: Compila o executável Trabalho_1.
Trabalho_1: Linka o objeto compilado para criar o executável.
Trabalho_1.o: Compila o arquivo fonte Trabalho_1.cpp para um objeto.
clean: Remove os arquivos gerados (Trabalho_1 e Trabalho_1.o).