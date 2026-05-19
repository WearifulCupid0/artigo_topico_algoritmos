#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <cmath>
#include <algorithm>

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
    long long chave;
};

struct SearchStats {
    long long comparacoes = 0;
    double tempoBuscaMs = 0.0;
    double tempoConstrucaoMs = 0.0; // Usado apenas pelo Hashing
    bool encontrou = false;
};

// -------------------------------------------------------
// Utilidades
// -------------------------------------------------------
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

// Leitura do arquivo (já ordenado)
vector<Registro> lerCSV(const string& caminho) {
    vector<Registro> dados;
    ifstream fin(caminho);
    if (!fin.is_open()) {
        cerr << "  [ERRO] Nao foi possivel abrir: " << caminho << "\n";
        return dados;
    }

    string linha;
    getline(fin, linha); // Pula cabeçalho

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
            stats.encontrou = true;
            break;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    stats.tempoBuscaMs = chrono::duration<double, milli>(end - start).count();
    return stats;
}

SearchStats buscaBinaria(const vector<Registro>& arr, long long alvo) {
    SearchStats stats;
    auto start = chrono::high_resolution_clock::now();

    int left = 0, right = (int)arr.size() - 1;
    while (left <= right) {
        stats.comparacoes++;
        int mid = left + (right - left) / 2;

        if (arr[mid].chave == alvo) {
            stats.encontrou = true;
            break;
        } else if (arr[mid].chave < alvo) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    stats.tempoBuscaMs = chrono::duration<double, milli>(end - start).count();
    return stats;
}

// O Hashing precisa de duas etapas: Construir e Buscar.
unordered_map<long long, int> construirHash(const vector<Registro>& arr, double& tempoConstrucaoMs) {
    auto start = chrono::high_resolution_clock::now();

    unordered_map<long long, int> mapa;
    // Reserva espaço para evitar realocações dinâmicas e otimizar tempo
    mapa.reserve(arr.size());
    for (size_t i = 0; i < arr.size(); i++) {
        mapa[arr[i].chave] = i;
    }

    auto end = chrono::high_resolution_clock::now();
    tempoConstrucaoMs = chrono::duration<double, milli>(end - start).count();
    return mapa;
}

SearchStats buscaHash(const unordered_map<long long, int>& mapa, long long alvo, double tempoConstrucaoMs) {
    SearchStats stats;
    stats.tempoConstrucaoMs = tempoConstrucaoMs;

    auto start = chrono::high_resolution_clock::now();

    // std::unordered_map tem complexidade O(1) média para busca
    stats.comparacoes = 1;
    if (mapa.find(alvo) != mapa.end()) {
        stats.encontrou = true;
    }

    auto end = chrono::high_resolution_clock::now();
    stats.tempoBuscaMs = chrono::duration<double, milli>(end - start).count();
    return stats;
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main() {
    cout << "=== BENCHMARK ISOLADO: SEQUENCIAL vs BINARIA vs HASHING ===\n\n";

    // 1. Caminho para o seu arquivo já ordenado
    string dir = "C:\\Users\\User\\CLionProjects\\untitled3\\";
    string arquivoEntrada = dir + "resultado_ordenado.csv";
    // OBS: se o seu arquivo se chamar exatamente apenas "resultado_ordenacao" sem o .csv no final,
    // ajuste o nome da string acima.

    cout << "Lendo arquivo ordenado: " << arquivoEntrada << " ...\n";
    vector<Registro> baseData = lerCSV(arquivoEntrada);

    if (baseData.empty()) {
        cerr << "\n[ERRO] Nenhum dado lido. O arquivo existe nesse caminho?\n";
        return 1;
    }

    cout << "Registros lidos com sucesso: " << baseData.size() << "\n";

    // Garante que temos pelo menos 70.000 registros duplicando se necessário
    while (baseData.size() < 70000) {
        size_t atual = baseData.size();
        for (size_t i = 0; i < atual && baseData.size() < 70000; i++) {
            baseData.push_back(baseData[i]);
        }
    }

    // Regarante que a base estendida permaneça perfeitamente ordenada
    sort(baseData.begin(), baseData.end(), [](const Registro& a, const Registro& b){ return a.chave < b.chave; });

    vector<int> tamanhos = {10000, 20000, 30000, 40000, 50000, 60000, 70000};

    // 2. Preparar arquivo de saída
    string arquivoSaida = dir + "benchmark_completo_buscas.csv";
    ofstream csv(arquivoSaida);
    // Cabeçalho com o máximo de informações possível
    csv << "Tamanho_N;Algoritmo;Cenario;Chave_Buscada;Sucesso;Tempo_Construcao_ms;Tempo_Busca_ms;Comparacoes\n";

    // 3. Loop de Testes
    for (int N : tamanhos) {
        cout << "\n" << string(60, '-') << "\n";
        cout << ">>> TESTANDO CARGA N = " << N << " REGISTROS\n";
        cout << string(60, '-') << "\n";

        // Pega a fatia do tamanho exato N
        vector<Registro> slice(baseData.begin(), baseData.begin() + N);

        // Define os 4 alvos de busca
        long long alvoMelhor  = slice[0].chave;             // Posição 0
        long long alvoMedio   = slice[N / 2].chave;         // Posição central
        long long alvoPior    = slice[N - 1].chave;         // Última posição
        long long alvoInexist = -999999999999;              // Chave impossível para forçar a busca a varrer tudo

        struct Cenario { string nome; long long chave; };
        Cenario cenarios[] = {
            {"Melhor_Caso", alvoMelhor},
            {"Medio_Caso", alvoMedio},
            {"Pior_Caso", alvoPior},
            {"Inexistente", alvoInexist}
        };

        // ==========================================
        // Construção da Tabela Hash (Feita 1 vez por N)
        // ==========================================
        double hashConstrucaoMs = 0.0;
        unordered_map<long long, int> tabelaHash = construirHash(slice, hashConstrucaoMs);

        // ==========================================
        // Execução das Buscas
        // ==========================================
        for (const auto& c : cenarios) {
            // Sequencial
            SearchStats resSeq = buscaSequencial(slice, c.chave);
            csv << N << ";Sequencial;" << c.nome << ";" << c.chave << ";"
                << (resSeq.encontrou ? "Sim" : "Nao") << ";"
                << fixed << setprecision(5) << resSeq.tempoConstrucaoMs << ";"
                << resSeq.tempoBuscaMs << ";" << resSeq.comparacoes << "\n";

            // Binária
            SearchStats resBin = buscaBinaria(slice, c.chave);
            csv << N << ";Binaria;" << c.nome << ";" << c.chave << ";"
                << (resBin.encontrou ? "Sim" : "Nao") << ";"
                << fixed << setprecision(5) << resBin.tempoConstrucaoMs << ";"
                << resBin.tempoBuscaMs << ";" << resBin.comparacoes << "\n";

            // Hashing
            SearchStats resHash = buscaHash(tabelaHash, c.chave, hashConstrucaoMs);
            csv << N << ";Hashing;" << c.nome << ";" << c.chave << ";"
                << (resHash.encontrou ? "Sim" : "Nao") << ";"
                << fixed << setprecision(5) << resHash.tempoConstrucaoMs << ";"
                << resHash.tempoBuscaMs << ";" << resHash.comparacoes << "\n";
        }

        cout << " -> " << N << " processado. (Hash build: " << fixed << setprecision(2) << hashConstrucaoMs << " ms)\n";
    }

    csv.close();
    cout << "\n*** TODOS OS TESTES CONCLUIDOS ***\n";
    cout << "Resultados salvos com sucesso em: " << arquivoSaida << "\n";

    return 0;
}