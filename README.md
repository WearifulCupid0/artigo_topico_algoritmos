# Benchmark de Algoritmos de Ordenação e Busca

Este repositório contém o código-fonte e os resultados de um estudo prático (benchmark) sobre o desempenho de diferentes algoritmos de ordenação e busca. O projeto avalia o comportamento computacional desses métodos utilizando uma base de dados reais de registros financeiros, medindo métricas como tempo de execução (ms), número de comparações e trocas em conjuntos de dados de diferentes tamanhos.

## 📊 Fonte dos Dados

Os dados reais utilizados para a realização dos testes referem-se aos gastos públicos da **Prefeitura Municipal de Medianeira** (Paraná), obtidos diretamente através do Portal da Transparência do município. A base de dados processada contempla os registros financeiros detalhados referentes aos meses de **Dezembro de 2025 e Janeiro de 2026**.

## 🤖 Desenvolvimento com Auxílio de IA

Gostaria de enfatizar que todo o ecossistema deste projeto — desde a estruturação dos algoritmos em C++ até os scripts de geração de gráficos em Python — foi desenvolvido com o suporte ativo de ferramentas de Inteligência Artificial. A utilização dessas tecnologias permitiu acelerar o processo de escrita, refatoração de código e análise de dados. As IAs utilizadas para a construção deste repositório foram:

* **ChatGPT**
* **Claude AI**
* **Gemini Pro**

## 💻 Códigos do Projeto

O pipeline de processamento e análise é dividido em 5 scripts principais:

* **`conversor_pdf_csv.py`**: Script em Python responsável pelo pré-processamento. Ele lê os extratos ou bases originais em formato PDF, extrai os registros financeiros (como débitos, créditos e históricos) e converte tudo para um formato estruturado em `.csv`, preparando os dados para os testes em C++.
* **`ordenacao.cpp`**: Código principal em C++ que executa o benchmark de ordenação. Ele carrega a base de dados em CSV e testa múltiplos algoritmos (Bubble Sort, Selection Sort, Insertion Sort, Shell Sort, Quick Sort, Merge Sort, Heap Sort e Radix Sort) em fatias crescentes de dados (ex: 10k a 70k registros). O script contabiliza o tempo de CPU, comparações lógicas e movimentações físicas na memória, exportando os resultados.
* **`busca.cpp`**: Código em C++ focado na recuperação de informação. Ele testa o desempenho da Busca Sequencial (em dados desordenados), Busca Binária (após ordenação) e Busca por Hashing (`unordered_map`). Avalia cenários de Melhor Caso, Caso Médio, Pior Caso e Chave Inexistente, exportando as métricas de performance.
* **`graficos_ordenacao.py`**: Script em Python (utilizando `pandas` e `matplotlib`) que consome os arquivos CSV gerados pelo teste de ordenação e plota os gráficos analíticos. Ele gera curvas de crescimento individuais por teste, médias globais, comparativos em escala Logarítmica e gráficos de barras absolutos.
* **`graficos_busca.py`**: Script em Python similar ao anterior, mas construído especificamente para compilar os resultados dos testes de busca, separando o desempenho por cenários e algoritmos para facilitar a análise visual no artigo final.

## ⚠️ Especificações da maquina e aplicativos utilizados para os benchmarks

* **`CPU`**: Intel i5-11400F 2.60 GHz
* **`RAM`**: RiseMode 16GB DDR4 2800 MT/s
* **`SSD`**: Western Digital SSD Sata 512GB
* **`GPU`**: NVIDIA GeForce RTX 3060 6GB OC
* **`OS`**: Microsoft Windows 11 Pro (25H2)
* **`IDEs`**: JetBrains CLion (2025.3.4), Microsoft Visual Studio Code (1.120.0)