#!/usr/bin/env python3
"""Compila um firmware original ESP-IDF e confere 6/6 + 8/8 via QEMU.

Requer ESP-IDF e fork Espressif QEMU corretamente instalados e exportados no shell.
"""
import argparse
import os
import pathlib
import pty
import re
import select
import shutil
import signal
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
PROJECTS = {"fp32": ROOT / "firmware" / "mlp_esp32s3", "int8": ROOT / "firmware" / "mlp_esp32s3_int8"}
PATTERN = re.compile(r"Resultado:\s*(\d+)\s*/\s*(\d+)\s+classificacoes coincidentes")


def executar(nome, destino, timeout, skip_build):
    project = PROJECTS[nome]
    if not project.joinpath("CMakeLists.txt").is_file():
        raise RuntimeError(f"Projeto nao encontrado: {project}")
    if not skip_build:
        print(f"\n== Compilando {nome} em {project} ==", flush=True)
        subprocess.run(["idf.py", "build"], cwd=project, check=True)

    print(f"\n== Iniciando QEMU {nome} ==", flush=True)
    master, slave = pty.openpty()
    proc = subprocess.Popen(["idf.py", "qemu"], cwd=project, stdin=slave, stdout=slave, stderr=slave, start_new_session=True, close_fds=True)
    os.close(slave)
    seen = []
    output = ""
    started = time.monotonic()
    try:
        while time.monotonic() - started < timeout:
            readable, _, _ = select.select([master], [], [], 0.5)
            if readable:
                try:
                    data = os.read(master, 8192)
                except OSError:
                    break
                if not data:
                    break
                chunk = data.decode("utf-8", errors="replace")
                output += chunk
                sys.stdout.write(chunk)
                sys.stdout.flush()
                seen = PATTERN.findall(output)
                if len(seen) >= 2 and "concluida." in output:
                    break
            if proc.poll() is not None:
                break
    finally:
        if proc.poll() is None:
            try:
                os.killpg(proc.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(proc.pid, signal.SIGKILL)
                proc.wait(timeout=5)
        os.close(master)
        destino.parent.mkdir(parents=True, exist_ok=True)
        destino.write_text(output, encoding="utf-8")
    esperado = [("6", "6"), ("8", "8")]
    if seen[:2] != esperado or "[ERRO]" in output:
        raise RuntimeError(f"QEMU {nome} nao confirmou 6/6 e 8/8: {seen}. Veja {destino}")
    print(f"\nOK: {nome} passou (6/6 bomba, 8/8 acumulador). Log: {destino}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--projeto", choices=["fp32", "int8", "ambos"], default="ambos")
    parser.add_argument("--timeout", type=float, default=180, help="Tempo maximo por execucao QEMU, em segundos")
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()
    if not shutil.which("idf.py") or not shutil.which("qemu-system-xtensa"):
        parser.error("idf.py ou qemu-system-xtensa indisponivel. Ative o ESP-IDF e instale QEMU Xtensa.")
    names = list(PROJECTS) if args.projeto == "ambos" else [args.projeto]
    for name in names:
        executar(name, ROOT / "artefatos_reproducao" / "qemu" / f"{name}.log", args.timeout, args.skip_build)


if __name__ == "__main__":
    main()
