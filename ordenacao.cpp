#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono> // Substituindo <ctime> para altíssima precisão
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <cmath>

using namespace std;

// -------------------------------------------------------
// Estruturas
// -------------------------------------------------------
struct Registro {
    long long lancamento;
    string    conta;
    string    historico;
    double    debito;
    double    credito;
    long long chave; // (credito - debito) * 100
};

struct SortStats {
    long long comparisons = 0;
    long long swaps = 0;
};

struct SearchStats {
    long long comparacoes = 0;
    double tempoMs = 0.0;
    int encontrados = 0;
};

// -------------------------------------------------------
// Utilidades
// -------------------------------------------------------
void swapWithCount(Registro& a, Registro& b, SortStats& stats) {
    Registro tmp = a; a = b; b = tmp;
    stats.swaps++;
}

bool isSorted(const vector<Registro>& v) {
    for (size_t i = 1; i < v.size(); i++)
        if (v[i-1].chave > v[i].chave) return false;
    return true;
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n\"");
    size_t end   = s.find_last_not_of(" \t\r\n\"");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

double parseDouble(const string& s) {
    string t = trim(s);
    replace(t.begin(), t.end(), ',', '.');
    if (t.empty()) return 0.0;
    try { return stod(t); } catch (...) { return 0.0; }
}

// -------------------------------------------------------
// Leitura dos CSVs
// -------------------------------------------------------
vector<Registro> lerCSV(const string& caminho) {
    vector<Registro> dados;
    ifstream fin(caminho);
    if (!fin.is_open()) {
        cerr << "  [AVISO] Nao foi possivel abrir: " << caminho << "\n";
        return dados;
    }

    string linha;
    getline(fin, linha); // Pula o cabeçalho

    while (getline(fin, linha)) {
        if (linha.empty()) continue;
        istringstream ss(linha);
        string tok[5];
        for (int i = 0; i < 5; i++) getline(ss, tok[i], ';');
        if (tok[1].empty()) {
            istringstream ss2(linha);
            for (int i = 0; i < 5; i++) getline(ss2, tok[i], ',');
        }

        Registro r;
        try { r.lancamento = stoll(trim(tok[0])); } catch (...) { r.lancamento = 0; }
        r.conta     = trim(tok[1]);
        r.historico = trim(tok[2]);
        r.debito    = parseDouble(tok[3]);
        r.credito   = parseDouble(tok[4]);
        r.chave     = llround((r.credito - r.debito) * 100.0);
        dados.push_back(r);
    }
    return dados;
}

// -------------------------------------------------------
// Algoritmos de Busca
// -------------------------------------------------------
SearchStats buscaSequencial(const vector<Registro>& arr, long long alvo) {
    SearchStats stats;
    auto start = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < arr.size(); i++) {
        stats.comparacoes++;
        if (arr[i].chave == alvo) {
            stats.encontrados++;
            break; // Paramos na primeira ocorrência para medir o pior/médio/melhor caso real
        }
    }

    auto end = chrono::high_resolution_clock::now();
    stats.tempoMs = chrono::duration<double, milli>(end - start).count();
    return stats;
}

SearchStats buscaBinaria(const vector<Registro>& arr, long long alvo) {
    SearchStats stats;
    auto start = chrono::high_resolution_clock::now();

    int left = 0, right = arr.size() - 1;
    while (left <= right) {
        stats.comparacoes++;
        int mid = left + (right - left) / 2;

        if (arr[mid].chave == alvo) {
            stats.encontrados++;
            break;
        } else if (arr[mid].chave < alvo) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    stats.tempoMs = chrono::duration<double, milli>(end - start).count();
    return stats;
}

// -------------------------------------------------------
// Algoritmos de Ordenação (Resumidos para economia de espaço)
// -------------------------------------------------------
void bubbleSort(vector<Registro>& v, SortStats& s) {
    int n = v.size();
    for (int i = 0; i < n - 1; i++) {
        bool changed = false;
        for (int j = 0; j < n - 1 - i; j++) {
            s.comparisons++;
            if (v[j].chave > v[j+1].chave) { swapWithCount(v[j], v[j+1], s); changed = true; }
        }
        if (!changed) break;
    }
}

void selectionSort(vector<Registro>& v, SortStats& s) {
    int n = v.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i+1; j < n; j++) {
            s.comparisons++;
            if (v[j].chave < v[minIdx].chave) minIdx = j;
        }
        if (minIdx != i) swapWithCount(v[i], v[minIdx], s);
    }
}

void insertionSort(vector<Registro>& v, SortStats& s) {
    int n = v.size();
    for (int i = 1; i < n; i++) {
        Registro key = v[i];
        int j = i - 1;
        while (j >= 0) {
            s.comparisons++;
            if (v[j].chave > key.chave) { v[j+1] = v[j]; s.swaps++; j--; }
            else break;
        }
        v[j+1] = key;
    }
}

int partitionQS(vector<Registro>& arr, int lo, int hi, SortStats& s) {
    long long pivot = arr[hi].chave;
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        s.comparisons++;
        if (arr[j].chave < pivot) { i++; swapWithCount(arr[i], arr[j], s); }
    }
    swapWithCount(arr[i+1], arr[hi], s);
    return i+1;
}

void quickSortRec(vector<Registro>& arr, int lo, int hi, SortStats& s) {
    if (lo < hi) {
        int pi = partitionQS(arr, lo, hi, s);
        quickSortRec(arr, lo, pi-1, s);
        quickSortRec(arr, pi+1, hi, s);
    }
}

void quickSort(vector<Registro>& arr, SortStats& s) { quickSortRec(arr, 0, (int)arr.size()-1, s); }

// (Shell, Merge, Heap e Radix poderiam ser colados aqui. Usarei apenas QS, Bubble, Selection e Insertion por brevidade visual, adicione os outros de volta conforme o original se quiser testar todos).
// Adicionando Shell e Merge para manter diversidade de complexidade:
void shellSort(vector<Registro>& arr, SortStats& s) {
    int n = arr.size();
    for (int gap = n/2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            Registro key = arr[i];
            int j = i;
            while (j >= gap) {
                s.comparisons++;
                if (arr[j-gap].chave > key.chave) { arr[j] = arr[j-gap]; s.swaps++; j -= gap; }
                else break;
            }
            arr[j] = key;
        }
    }
}

void heapify(vector<Registro>& arr, int n, int i, SortStats& s) {
    int largest = i, l = 2*i+1, r = 2*i+2;
    if (l < n) { s.comparisons++; if (arr[l].chave > arr[largest].chave) largest = l; }
    if (r < n) { s.comparisons++; if (arr[r].chave > arr[largest].chave) largest = r; }
    if (largest != i) { swapWithCount(arr[i], arr[largest], s); heapify(arr, n, largest, s); }
}

void heapSort(vector<Registro>& arr, SortStats& s) {
    int n = arr.size();
    for (int i = n/2-1; i >= 0; i--) heapify(arr, n, i, s);
    for (int i = n-1; i > 0; i--) { swapWithCount(arr[0], arr[i], s); heapify(arr, i, 0, s); }
}

void merge(vector<Registro>& arr, int l, int m, int r, SortStats& s) {
    int n1 = m-l+1, n2 = r-m;
    vector<Registro> L(arr.begin()+l, arr.begin()+l+n1);
    vector<Registro> R(arr.begin()+m+1, arr.begin()+m+1+n2);
    s.swaps += n1 + n2;
    int i=0, j=0, k=l;
    while (i < n1 && j < n2) {
        s.comparisons++;
        if (L[i].chave <= R[j].chave) arr[k++] = L[i++];
        else                          arr[k++] = R[j++];
        s.swaps++;
    }
    while (i < n1) { arr[k++] = L[i++]; s.swaps++; }
    while (j < n2) { arr[k++] = R[j++]; s.swaps++; }
}

void mergeSortRec(vector<Registro>& arr, int l, int r, SortStats& s) {
    if (l < r) {
        int m = l + (r-l)/2;
        mergeSortRec(arr, l, m, s);
        mergeSortRec(arr, m+1, r, s);
        merge(arr, l, m, r, s);
    }
}

void mergeSort(vector<Registro>& arr, SortStats& s) {
    mergeSortRec(arr, 0, (int)arr.size()-1, s);
}

// Radix Sort adaptado para long long (inclui negativos via offset)
void radixSortLL(vector<Registro>& arr, SortStats& s) {
    int n = arr.size();
    long long mn = arr[0].chave;
    for (auto& r : arr) if (r.chave < mn) mn = r.chave;

    // offset para tornar todos não-negativos
    long long offset = (mn < 0) ? -mn : 0;
    vector<unsigned long long> keys(n);
    for (int i = 0; i < n; i++) { keys[i] = (unsigned long long)(arr[i].chave + offset); s.swaps++; }

    // 8 passes de 8 bits = 64 bits
    for (int shift = 0; shift < 64; shift += 8) {
        int count[256] = {0};
        for (int i = 0; i < n; i++) { count[(keys[i] >> shift) & 0xFF]++; s.swaps++; }
        for (int i = 1; i < 256; i++) count[i] += count[i-1];
        vector<unsigned long long> outK(n);
        vector<Registro> outR(n);
        for (int i = n-1; i >= 0; i--) {
            int idx = (keys[i] >> shift) & 0xFF;
            outK[count[idx]-1] = keys[i];
            outR[count[idx]-1] = arr[i];
            count[idx]--;
            s.swaps++;
        }
        keys = outK;
        arr  = outR;
        for (int i = 0; i < n; i++) s.swaps++;
    }
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------
typedef void (*SortFunction)(vector<Registro>&, SortStats&);
struct AlgorithmEntry { string name; SortFunction fn; };

int main() {
    cout << "=== SUPER BENCHMARK: ORDENACAO E BUSCA ===\n\n";

    // 1. Carregar Base
    vector<string> arquivos = {
        "C:\\Users\\User\\CLionProjects\\untitled3\\saida.csv",
        "C:\\Users\\User\\CLionProjects\\untitled3\\saida1.csv"
    };

    vector<Registro> baseData;
    for (auto& arq : arquivos) {
        auto parte = lerCSV(arq);
        baseData.insert(baseData.end(), parte.begin(), parte.end());
    }

    if (baseData.empty()) { cerr << "ERRO: Nenhum dado lido.\n"; return 1; }

    while (baseData.size() < 50000) {
        size_t atual = baseData.size();
        for (size_t i = 0; i < atual && baseData.size() < 50000; i++) {
            baseData.push_back(baseData[i]);
        }
    }
    cout << "Dados carregados/ajustados para bater pelo menos 50.000 registros.\n\n";

    vector<int> tamanhos = {10000, 20000, 30000, 40000, 50000, 60000, 70000};

    AlgorithmEntry algorithms[] = {
        {"Bubble Sort",    bubbleSort},
        {"Selection Sort", selectionSort},
        {"Insertion Sort", insertionSort},
        {"Shell Sort",     shellSort},
        {"Quick Sort",     quickSort},
        {"Heap Sort", heapSort},
        {"Merge Sort", mergeSort},
        {"Radix Soort", radixSortLL}
    };
    int ALG_COUNT = 8;

    // Arquivos de relatorio
    ofstream csvSort("C:\\Users\\User\\CLionProjects\\untitled3\\benchmark_ordenacao.csv");
    csvSort << "Tamanho;Algoritmo;Tempo_ms;Comparacoes;Trocas\n";

    ofstream csvBusca("C:\\Users\\User\\CLionProjects\\untitled3\\benchmark_buscas.csv");
    csvBusca << "Tamanho;Caso;Alvo_Chave;Seq_Tempo_ms;Seq_Comparacoes;Bin_Tempo_ms;Bin_Comparacoes\n";

    // Loop principal: Roda o fluxo completo para os 5 tamanhos diferentes
    for (int N : tamanhos) {
        cout << string(60, '=') << "\n";
        cout << ">>> INICIANDO TESTES PARA TAMANHO: " << N << " REGISTROS <<<\n";
        cout << string(60, '=') << "\n";

        // Corta o dataset para o tamanho atual
        vector<Registro> slice(baseData.begin(), baseData.begin() + N);

        // REGRA 2: Selecionando os alvos para Busca ANTES de ordenar
        long long alvoMelhor = slice[0].chave;
        long long alvoMedio  = slice[N / 2].chave;
        long long alvoPior   = slice[N - 1].chave;

        cout << "\n[1] BUSCA SEQUENCIAL (DADOS DESORDENADOS)\n";
        SearchStats sqMelhor = buscaSequencial(slice, alvoMelhor);
        SearchStats sqMedio  = buscaSequencial(slice, alvoMedio);
        SearchStats sqPior   = buscaSequencial(slice, alvoPior);

        cout << " -> Melhor (pos 0):    " << sqMelhor.comparacoes << " comp. | " << fixed << setprecision(4) << sqMelhor.tempoMs << " ms\n";
        cout << " -> Medio  (pos N/2):  " << sqMedio.comparacoes << " comp. | " << sqMedio.tempoMs << " ms\n";
        cout << " -> Pior   (pos N-1):  " << sqPior.comparacoes << " comp. | " << sqPior.tempoMs << " ms\n";

        // REGRA 1: Ordenação
        cout << "\n[2] ORDENACAO E STATUS FÍSICO/LÓGICO\n";
        vector<Registro> sliceOrdenado; // Guardaremos a versão ordenada aqui para a Busca Binária

        for (int a = 0; a < ALG_COUNT; a++) {
            vector<Registro> working = slice; // Copia fresca e desordenada
            SortStats stats;

            auto start = chrono::high_resolution_clock::now();
            algorithms[a].fn(working, stats);
            auto end = chrono::high_resolution_clock::now();

            double ms = chrono::duration<double, milli>(end - start).count();

            cout << left << " -> " << setw(15) << algorithms[a].name
                 << " | Tempo: " << setw(8) << ms << " ms"
                 << " | Comp: " << setw(12) << stats.comparisons
                 << " | Trocas: " << stats.swaps << "\n";

            csvSort << N << ";" << algorithms[a].name << ";"
                    << fixed << setprecision(4) << ms << ";"
                    << stats.comparisons << ";" << stats.swaps << "\n";

            // Guarda a versão ordenada do QuickSort para usar na Busca Binária
            if (algorithms[a].name == "Quick Sort") sliceOrdenado = working;
        }

        // REGRA 3: Busca Binária APÓS a ordenação
        cout << "\n[3] BUSCA BINARIA (DADOS ORDENADOS)\n";
        SearchStats binMelhor = buscaBinaria(sliceOrdenado, alvoMelhor);
        SearchStats binMedio  = buscaBinaria(sliceOrdenado, alvoMedio);
        SearchStats binPior   = buscaBinaria(sliceOrdenado, alvoPior);

        cout << " -> Alvo Melhor (Original): " << binMelhor.comparacoes << " comp. | " << fixed << setprecision(4) << binMelhor.tempoMs << " ms\n";
        cout << " -> Alvo Medio  (Original): " << binMedio.comparacoes << " comp. | " << binMedio.tempoMs << " ms\n";
        cout << " -> Alvo Pior   (Original): " << binPior.comparacoes << " comp. | " << binPior.tempoMs << " ms\n\n";

        // Salvar buscas no CSV
        csvBusca << N << ";Melhor;" << alvoMelhor << ";" << sqMelhor.tempoMs << ";" << sqMelhor.comparacoes << ";" << binMelhor.tempoMs << ";" << binMelhor.comparacoes << "\n";
        csvBusca << N << ";Medio;" << alvoMedio << ";" << sqMedio.tempoMs << ";" << sqMedio.comparacoes << ";" << binMedio.tempoMs << ";" << binMedio.comparacoes << "\n";
        csvBusca << N << ";Pior;" << alvoPior << ";" << sqPior.tempoMs << ";" << sqPior.comparacoes << ";" << binPior.tempoMs << ";" << binPior.comparacoes << "\n";
    }

    csvSort.close();
    csvBusca.close();

    cout << "\n*** BENCHMARK FINALIZADO ***\n";
    cout << "Os relatorios foram salvos em:\n";
    cout << " - C:\\Users\\User\\CLionProjects\\untitled3\\benchmark_ordenacao.csv\n";
    cout << " - C:\\Users\\User\\CLionProjects\\untitled3\\benchmark_buscas.csv\n";

    return 0;
}