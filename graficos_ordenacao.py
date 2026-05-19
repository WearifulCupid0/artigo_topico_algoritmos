import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from zipfile import ZipFile
import re

# ==========================================
# 1. CONFIGURAÇÕES DE CAMINHOS E PASTAS
# ==========================================
base = Path('C:/Users/User/Desktop/Projects/artigo-topicos') 
zip_path = base / 'ordenacao.zip'
extract_dir = base / 'ordenacao_extract'
graphs_dir = base / 'ordenacao_graphs'

# Subpastas para manter tudo organizado
individuais_dir = graphs_dir / 'testes_individuais'
algoritmos_dir = graphs_dir / 'por_algoritmo'

extract_dir.mkdir(exist_ok=True)
graphs_dir.mkdir(exist_ok=True)
individuais_dir.mkdir(exist_ok=True)
algoritmos_dir.mkdir(exist_ok=True)

if zip_path.exists():
    with ZipFile(zip_path, 'r') as z:
        z.extractall(extract_dir)
else:
    print(f"[AVISO] Arquivo {zip_path.name} não encontrado. Lendo CSVs de: {extract_dir}")

# ==========================================
# 2. LEITURA E TRATAMENTO DE DADOS
# ==========================================
csv_files = list(extract_dir.rglob('*.csv'))

if not csv_files:
    print("[ERRO] Nenhum arquivo CSV encontrado! Coloque seus arquivos na pasta.")
    exit()

dfs = []
for file in csv_files:
    match = re.search(r'(\d+)', file.name)
    num_teste = int(match.group(1)) if match else 1

    try:
        df = pd.read_csv(file, sep=';')
    except Exception as e:
        print(f"Erro ao ler {file.name}: {e}")
        continue
        
    if all(col in df.columns for col in ['Tamanho', 'Algoritmo', 'Tempo_ms', 'Comparacoes', 'Trocas']):
        df['Execucao'] = num_teste
        dfs.append(df)

if not dfs:
    print("[ERRO] Nenhum dado de ORDENAÇÃO válido encontrado nos arquivos CSV.")
    exit()

df_all = pd.concat(dfs, ignore_index=True)

metrics = ['Tempo_ms', 'Comparacoes', 'Trocas']
for col in metrics:
    df_all[col] = pd.to_numeric(df_all[col], errors='coerce')

# ========================================================
# 3. GERAÇÃO DOS GRÁFICOS INDIVIDUAIS (POR TESTE)
# ========================================================
print("Gerando gráficos para cada teste individual...")
for num_exec in df_all['Execucao'].unique():
    df_teste = df_all[df_all['Execucao'] == num_exec]
    
    for metric in metrics:
        plt.figure(figsize=(10, 6))
        
        for algoritmo in df_teste['Algoritmo'].unique():
            subset = df_teste[df_teste['Algoritmo'] == algoritmo].sort_values('Tamanho')
            
            plt.plot(
                subset['Tamanho'],
                subset[metric],
                marker='o',
                linewidth=2,
                label=algoritmo
            )
            
        plt.title(f'Teste {num_exec} - Ordenação - {metric.replace("_", " ")}')
        plt.xlabel('Tamanho da Base (N)')
        plt.ylabel(metric)
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend(title="Algoritmo", bbox_to_anchor=(1.05, 1), loc='upper left')
        
        filename = individuais_dir / f'Teste_{num_exec}_Ordenacao_{metric}.png'
        plt.savefig(filename, bbox_inches='tight')
        plt.close()

# ========================================================
# 4. GERAÇÃO DOS GRÁFICOS ISOLADOS (POR ALGORITMO)
# ========================================================
print("Gerando gráficos isolados por algoritmo...")
for algoritmo in df_all['Algoritmo'].unique():
    df_alg = df_all[df_all['Algoritmo'] == algoritmo]
    
    for metric in metrics:
        plt.figure(figsize=(10, 6))
        
        # Plota cada um dos 5 testes
        for num_exec in df_alg['Execucao'].unique():
            subset = df_alg[df_alg['Execucao'] == num_exec].sort_values('Tamanho')
            plt.plot(
                subset['Tamanho'],
                subset[metric],
                marker='o',
                alpha=0.4, # Deixa as linhas dos testes individuais um pouco transparentes
                label=f'Teste {num_exec}'
            )
        
        # Calcula e plota a MÉDIA como uma linha destacada
        grouped_media = df_alg.groupby('Tamanho')[metric].mean()
        plt.plot(
            grouped_media.index,
            grouped_media.values,
            marker='s',
            linewidth=3,
            color='black',
            label='Média Geral'
        )
            
        plt.title(f'{algoritmo} Isolado - {metric.replace("_", " ")}')
        plt.xlabel('Tamanho da Base (N)')
        plt.ylabel(metric)
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend(title="Execuções", bbox_to_anchor=(1.05, 1), loc='upper left')
        
        alg_filename = algoritmo.replace(" ", "_")
        filename = algoritmos_dir / f'{alg_filename}_{metric}.png'
        plt.savefig(filename, bbox_inches='tight')
        plt.close()

# ========================================================
# 5. COMPARATIVO GERAL (MÉDIA DE TODOS OS TESTES)
# ========================================================
print("Gerando gráficos comparativos gerais (médias)...")
for metric in metrics:
    plt.figure(figsize=(10, 6))

    for algoritmo in df_all['Algoritmo'].unique():
        grouped = (
            df_all[df_all['Algoritmo'] == algoritmo]
            .groupby('Tamanho')[metric]
            .mean()
        )

        plt.plot(
            grouped.index,
            grouped.values,
            marker='o',
            linewidth=2,
            label=algoritmo
        )

    plt.title(f'Comparação Geral Média - Ordenação - {metric.replace("_", " ")}')
    plt.xlabel('Tamanho da Base (N)')
    plt.ylabel(metric)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(title="Algoritmo", bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.savefig(graphs_dir / f'Media_Geral_Curvas_{metric}.png', bbox_inches='tight')
    
    plt.yscale('symlog')
    plt.title(f'Comparação Geral Média - {metric.replace("_", " ")} (Escala LOG)')
    plt.savefig(graphs_dir / f'Media_Geral_Curvas_{metric}_LOG.png', bbox_inches='tight')
    plt.close()

# ========================================================
# 6. GRÁFICO DE BARRAS (MÉDIAS FINAIS ABSOLUTAS)
# ========================================================
print("Gerando gráficos de barras...")
media_df = df_all.groupby('Algoritmo')[metrics].mean()

for metric in metrics:
    plt.figure(figsize=(10, 6))
    
    ax = media_df[metric].sort_values(ascending=False).plot(kind='bar', color='#ff9999', edgecolor='black')
    
    plt.title(f'Média Absoluta (Total) - {metric.replace("_", " ")}')
    plt.ylabel(metric)
    plt.xlabel('Algoritmo')
    plt.xticks(rotation=45, ha='right')
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    for p in ax.patches:
        altura = p.get_height()
        texto = f"{altura:,.2f}" if "Tempo" in metric else f"{int(altura):,}"
        ax.annotate(texto, 
                    (p.get_x() + p.get_width() / 2., altura), 
                    ha='center', va='bottom', fontsize=9, xytext=(0, 5), 
                    textcoords='offset points')

    plt.savefig(graphs_dir / f'Media_Final_Barras_{metric}.png', bbox_inches='tight')
    plt.close()

media_df.to_csv(graphs_dir / 'medias_ordenacao_compilado.csv')

# ========================================================
# 7. COMPACTAÇÃO EM ZIP
# ========================================================
output_zip = base / 'graficos_ordenacao_final.zip'
if output_zip.exists():
    output_zip.unlink()

with ZipFile(output_zip, 'w') as z:
    for file in graphs_dir.rglob('*'):
        if file.is_file():
            z.write(file, arcname=file.relative_to(graphs_dir))

print("\n=== CONCLUÍDO COM SUCESSO ===")
print(f"Gráficos por algoritmo salvos em: {algoritmos_dir}")
print(f"Gráficos individuais salvos em: {individuais_dir}")
print(f"Gráficos globais salvos em: {graphs_dir}")
print(f"Arquivo ZIP para entrega gerado: {output_zip}")