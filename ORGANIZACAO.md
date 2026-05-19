# Estrutura e Organização do Repositório

Para facilitar a reprodução dos testes e a análise dos resultados, este repositório está organizado em uma estrutura clara de diretórios, separando os códigos-fonte, os dados brutos e os artefatos gerados.

## 📂 Árvore do Projeto

```text
/ (Raiz do Repositório)
│
├── README.md                    # Documentação principal e descrição dos códigos
├── ORGANIZACAO.md               # Explicação da estrutura de diretórios (este arquivo)
│
├── busca.cpp                    # Código-fonte do benchmark de busca
├── ordenacao.cpp                # Código-fonte do benchmark de ordenação
├── conversor_pdf_csv.py         # Script de conversão de dados
├── graficos_busca.py            # Script gerador de gráficos de busca
├── graficos_ordenacao.py        # Script gerador de gráficos de ordenação
│
├── dados/                       # Base de dados do projeto
│   ├── dezembro.pdf             # Arquivos PDF originais referentes aos gastos de Dezembro de 2025
│   ├── janeiro.pdf              # Arquivos PDF originais referentes aos gastos de Janeiro de 2026
│   ├── saida.csv                # Base de dados estruturada gerada pelo script Python referentes a Dezembro de 2025
│   └── saida1.csv               # Base de dados estruturada gerada pelo script Python referentes a Janeiro de 2026
│   └── resultado_ordenado.csv   # Resultado das duas saidas já ordenadas para os benchmarks dos algoritmos de busca
│
├── resultados/                  # Relatórios CSV gerados pela execução dos códigos C++
│   ├── ordenacao/               # CSVs individuais gerados por teste de ordenação
│   └── busca/                   # CSVs individuais gerados por teste de busca
│
└── graficos/                    # Imagens em .png geradas pelos scripts Python para o artigo
    ├── busca/                   # Gráficos específicos da análise de busca
    │   ├── comparativos/        # Gráficos gerais e escalas Logarítmicas
    │   └── testes_individuais/  # Gráficos específicos de cada execução (N)
    │
    └── ordenacao/               # Gráficos específicos da análise de ordenação
        ├── comparativos/        # Gráficos gerais (barras e médias globais)
        ├── por_algoritmo/       # Gráficos isolando o comportamento de 1 único algoritmo
        └── testes_individuais/  # Curvas geradas para cada teste específico