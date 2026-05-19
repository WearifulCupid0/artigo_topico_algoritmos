import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from zipfile import ZipFile
import re

# Configurações de caminhos (Mantidos os seus caminhos originais)
base = Path('C:/Users/User/Desktop/Projects/artigo-topicos')
zip_path = base / 'busca.zip'
extract_dir = base / 'busca_extract'
graphs_dir = base / 'busca_graphs'
individuais_dir = graphs_dir / 'testes_individuais'

# Criar as pastas necessárias
extract_dir.mkdir(exist_ok=True)
graphs_dir.mkdir(exist_ok=True)
individuais_dir.mkdir(exist_ok=True)

# Extrair ZIP se ele existir
if zip_path.exists():
    with ZipFile(zip_path, 'r') as z:
        z.extractall(extract_dir)
else:
    print(f"[AVISO] Arquivo ZIP não encontrado. Lendo CSVs diretamente de: {extract_dir}")

# Buscar todos os arquivos CSV na pasta
csv_files = list(extract_dir.rglob('*.csv'))
if not csv_files:
    print("[ERRO] Nenhum arquivo CSV encontrado! Certifique-se de que os arquivos dos testes estão na pasta.")
    exit()

dfs = []

# Lendo os arquivos e capturando o número do teste pelo nome do arquivo
for file in csv_files:
    # Captura qualquer número no nome do arquivo (ex: teste1.csv, teste_2.csv, resultado3.csv)
    match = re.search(r'(\d+)', file.name)
    num_teste = int(match.group(1)) if match else 1

    try:
        df = pd.read_csv(file, sep=';')
    except Exception as e:
        print(f"Erro ao ler {file.name}: {e}")
        continue
        
    # Adiciona a coluna identificando a qual teste esse bloco pertence
    df['Execucao'] = num_teste
    dfs.append(df)

# Une todos os dados em um único DataFrame do Pandas
df_all = pd.concat(dfs, ignore_index=True)

# Garantir que as colunas métricas sejam numéricas
metrics = ['Tempo_Busca_ms', 'Comparacoes']
for col in metrics:
    df_all[col] = pd.to_numeric(df_all[col], errors='coerce')

# ========================================================
# 1. GERAÇÃO DOS GRÁFICOS INDIVIDUAIS (POR TESTE/EXECUÇÃO)
# ========================================================
print("Gerando gráficos para cada teste individual...")
for num_exec in df_all['Execucao'].unique():
    df_teste = df_all[df_all['Execucao'] == num_exec]
    
    for algoritmo in df_teste['Algoritmo'].unique():
        df_alg = df_teste[df_teste['Algoritmo'] == algoritmo]
        
        for metric in metrics:
            plt.figure(figsize=(10, 6))
            
            for cenario in df_alg['Cenario'].unique():
                subset = df_alg[df_alg['Cenario'] == cenario]
                # Ordena por tamanho para a linha não ficar zig-zag
                subset = subset.sort_values('Tamanho_N') 
                
                plt.plot(
                    subset['Tamanho_N'],
                    subset[metric],
                    marker='o',
                    linewidth=2,
                    label=cenario.replace('_', ' ')
                )
                
            plt.title(f'Teste {num_exec} - {algoritmo} - {metric.replace("_", " ")}')
            plt.xlabel('Tamanho da Base (N)')
            plt.ylabel(metric)
            plt.grid(True, linestyle='--', alpha=0.7)
            plt.legend(title="Cenário")
            
            # Salva na pasta de testes individuais
            filename = individuais_dir / f'Teste_{num_exec}_{algoritmo}_{metric}.png'
            plt.savefig(filename, bbox_inches='tight')
            plt.close()

# ========================================================
# 2. COMPARATIVO GERAL (MÉDIA DE TODOS OS TESTES)
# ========================================================
print("Gerando gráficos comparativos gerais (médias)...")
for metric in metrics:
    plt.figure(figsize=(10, 6))

    for algoritmo in df_all['Algoritmo'].unique():
        # Agrupa por tamanho e tira a média das 5 execuções para o gráfico do artigo
        grouped = (
            df_all[df_all['Algoritmo'] == algoritmo]
            .groupby('Tamanho_N')[metric]
            .mean()
        )

        plt.plot(
            grouped.index,
            grouped.values,
            marker='o',
            linewidth=2,
            label=algoritmo
        )

    plt.title(f'Comparação Geral Média - {metric.replace("_", " ")}')
    plt.xlabel('Tamanho da Base (N)')
    plt.ylabel(metric)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(title="Algoritmo")
    
    # Salva o gráfico linear regular
    plt.savefig(graphs_dir / f'Comparacao_Geral_{metric}.png', bbox_inches='tight')
    
    # Salva também em escala LOGARÍTMICA (ótimo para destacar a diferença gritante entre Sequencial e Binária/Hash)
    plt.yscale('symlog')
    plt.title(f'Comparação Geral Média - {metric.replace("_", " ")} (Escala LOG)')
    plt.savefig(graphs_dir / f'Comparacao_Geral_{metric}_LOG.png', bbox_inches='tight')
    plt.close()

# ========================================================
# 3. GRÁFICO DE BARRAS COM AS MÉDIAS FINAIS
# ========================================================
media_df = df_all.groupby('Algoritmo')[metrics].mean()