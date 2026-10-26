#!/usr/bin/env python3
"""Verificação auxiliar C++ nativa. NÃO usa ESP-IDF nem QEMU."""
import pathlib
import subprocess
import tempfile
import re
import shutil

ROOT = pathlib.Path(__file__).resolve().parents[1]
PROJETOS = ("mlp_esp32s3", "mlp_esp32s3_int8")

MOCKS = {'esp_heap_caps.h': '#pragma once\n#include <cstddef>\n#define MALLOC_CAP_8BIT 1\ninline size_t heap_caps_get_free_size(int) {return 400000;}\ninline size_t heap_caps_get_minimum_free_size(int) {return 390000;}\n', 'freertos/FreeRTOS.h': '#pragma once\n', 'freertos/task.h': '#pragma once\ninline unsigned uxTaskGetStackHighWaterMark(void*) {return 2000;}\n', 'esp_timer.h': '#pragma once\n#include <cstdint>\n#include <chrono>\ninline int64_t esp_timer_get_time() {return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}\n', 'entry.cpp': 'extern "C" void app_main(void); int main(){app_main(); return 0;}\n'}


def executar():
    if not shutil.which("g++"):
        raise SystemExit("ERRO: g++ nao encontrado. Instale build-essential.")
    with tempfile.TemporaryDirectory(prefix="edge_ai_cpp_") as d:
        d = pathlib.Path(d)
        for name, content in MOCKS.items():
            path = d / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        for project in PROJETOS:
            src = ROOT / "firmware" / project / "main" / "main.cpp"
            dst = d / project
            cmd = ["g++", "-std=c++17", "-O2", "-I", str(d), str(d / "entry.cpp"), str(src), "-o", str(dst)]
            subprocess.run(cmd, check=True)
            proc = subprocess.run([str(dst)], check=True, capture_output=True, text=True)
            contagens = re.findall(r"Resultado:\s*(\d+)\s*/\s*(\d+)\s+classificacoes coincidentes", proc.stdout)
            if contagens != [("6", "6"), ("8", "8")] or "[ERRO]" in proc.stdout:
                print(proc.stdout)
                raise SystemExit(f"FALHA no projeto {project}: contagens={contagens}")
            print(f"OK - {project}: 6/6 bomba; 8/8 acumulador (teste auxiliar no PC)")
    print("ATENCAO: esse teste nao equivale a compilar ou executar ESP-IDF/QEMU.")


if __name__ == "__main__":
    executar()
