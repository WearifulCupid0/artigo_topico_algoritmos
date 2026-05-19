import pdfplumber
import csv
import re

pdf_path = "dados.pdf"
output_csv = "saida.csv"

def is_numero(valor):
    return re.match(r"^[\d\.,]+$", valor)

def limpar_valor(valor):
    return valor.replace(".", "").replace(",", ".")

linha_atual = None

with open(output_csv, "w", newline="", encoding="utf-8-sig") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Lancamento", "Conta", "Historico", "Debito", "Credito"])

    with pdfplumber.open(pdf_path) as pdf:
        for i, pagina in enumerate(pdf.pages):
            print(f"Processando página {i+1}/{len(pdf.pages)}")

            texto = pagina.extract_text()
            if not texto:
                continue

            for linha in texto.split("\n"):
                linha = linha.strip()

                if not linha:
                    continue

                partes = linha.split()

                # ✅ REGRA FORTE: linha válida termina com 2 números
                if len(partes) >= 4 and is_numero(partes[-1]) and is_numero(partes[-2]) and "-" in linha:

                    credito = limpar_valor(partes[-1])
                    debito = limpar_valor(partes[-2])

                    idx_traco = linha.find("-")
                    antes = linha[:idx_traco].strip()
                    historico = linha[idx_traco:].strip()

                    antes_partes = antes.split()

                    if len(antes_partes) >= 2:
                        lancamento = antes_partes[0]
                        conta = antes_partes[1]

                        # salva anterior
                        if linha_atual:
                            writer.writerow([
                                linha_atual["Lancamento"],
                                linha_atual["Conta"],
                                linha_atual["Historico"],
                                linha_atual["Debito"],
                                linha_atual["Credito"]
                            ])
                            print(f"Escrevendo página {i+1}/{len(pdf.pages)}")

                        linha_atual = {
                            "Lancamento": lancamento,
                            "Conta": conta,
                            "Historico": historico,
                            "Debito": debito,
                            "Credito": credito,
                        }

                else:
                    # 🔥 continuação SEM "-"
                    if linha_atual:
                        linha_atual["Historico"] += " " + linha

    # última linha
    if linha_atual:
        writer.writerow([
            linha_atual["Lancamento"],
            linha_atual["Conta"],
            linha_atual["Historico"],
            linha_atual["Debito"],
            linha_atual["Credito"]
        ])

print("Finalizado!")