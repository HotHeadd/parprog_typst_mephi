#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, AutoMinorLocator


def _strip_inline_comment(s: str) -> str:
    # Удаляем все после // или #
    for marker in ("//", "#"):
        idx = s.find(marker)
        if idx != -1:
            s = s[:idx]
    return s.strip()


def _next_meaningful_line(f):
    for raw in f:
        line = _strip_inline_comment(raw)
        if line:
            yield line


def parse_file(path: Path):
    with path.open("r", encoding="utf-8") as f:
        lines = _next_meaningful_line(f)

        # 1) Кол-во серий (число столбцов, кроме первой оси X)
        first = next(lines)
        try:
            n_series = int(first.split()[0])
        except Exception as e:
            raise ValueError(f"Не удалось прочитать число серий из первой строки: {first}") from e
        if n_series <= 0:
            raise ValueError("Число серий должно быть положительным")

        # 2) Названия серий (n_series строк)
        series_names = []
        for i in range(n_series):
            try:
                name = next(lines)
            except StopIteration:
                raise ValueError("Не хватает строк с названиями серий")
            series_names.append(name.strip() or f"Серия {i+1}")

        # 3) Подписи осей
        try:
            x_label = next(lines)
            y_label = next(lines)
        except StopIteration:
            raise ValueError("Не хватает строк с подписями осей X/Y")

        # 4) Тип аппроксимации: none | linear
        try:
            approx_line = next(lines)
        except StopIteration:
            raise ValueError("Не найдена строка с видом аппроксимации (none | linear)")

        t0 = approx_line.strip().lower()
        if t0 == "none":
            approx_type = "none"
        elif t0 == "linear":
            approx_type = "linear"
        else:
            raise ValueError(f"Неизвестный тип аппроксимации: {t0}. Допустимо: none | linear")

        # 5) Данные: каждая строка — X и далее n_series значений Y
        xs = []
        ys = [[] for _ in range(n_series)]
        for line in lines:
            tokens = [t.replace(",", ".") for t in line.split()]
            if len(tokens) < 1 + n_series:
                raise ValueError(
                    f"Строка данных содержит меньше значений, чем требуется ({1+n_series}): {line}"
                )
            try:
                x = float(tokens[0])
                xs.append(x)
                for i in range(n_series):
                    yi = float(tokens[1 + i])
                    ys[i].append(yi)
            except Exception as e:
                raise ValueError(f"Ошибка парсинга чисел в строке: {line}") from e

    if not xs:
        raise ValueError("Не найдено ни одной строки с данными")

    x_arr = np.array(xs, dtype=float)
    y_arrs = [np.array(col, dtype=float) for col in ys]

    # Проверим одинаковую длину
    n = len(x_arr)
    for i, arr in enumerate(y_arrs):
        if len(arr) != n:
            raise ValueError(f"Серия {i+1} имеет длину {len(arr)}, ожидается {n}")

    return series_names, x_label, y_label, approx_type, x_arr, y_arrs


def plot(series_names, x_label, y_label, approx_type, x_arr, y_arrs, out_path=None, dpi=150, show=False):
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.xaxis.set_major_formatter(FuncFormatter(lambda x, pos: f"{x:,.0f}".replace(",", " ")))

    # Основные линии
    for name, y in zip(series_names, y_arrs):
        ax.plot(x_arr, y, marker="o", linewidth=1.8, markersize=4, label=name)

    # ---- Автоматическая аппроксимация linear ----
    if approx_type == "linear":
        xmin, xmax = float(np.min(x_arr)), float(np.max(x_arr))
        x_line = np.linspace(xmin, xmax, 200)
        series_lines = ax.get_lines()[:len(series_names)]

        for name, y, ln in zip(series_names, y_arrs, series_lines):
            # Модель: y = y0 + k*(x - x0), где (x0, y0) — первая точка
            x0 = float(x_arr[0])
            y0 = float(y[0])

            dx = x_arr - x0
            dy = y - y0
            denom = np.sum(dx * dx)

            if denom == 0.0:
                k = 0.0
            else:
                k = np.sum(dx * dy) / denom

            y_line = y0 + k * (x_line - x0)
            ax.plot(
                x_line,
                y_line,
                linestyle="--",
                alpha=0.8,
                color=ln.get_color(),
                label=f"{name} (аппр., k={k:.3g})"
            )

    # Подписи и оформление
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)

    ax.minorticks_on()
    ax.xaxis.set_minor_locator(AutoMinorLocator(10))
    ax.yaxis.set_minor_locator(AutoMinorLocator(10))
    ax.grid(which="major", linestyle="-", color="0.6", linewidth=0.7, alpha=0.6)
    ax.grid(which="minor", linestyle="-", color="0.85", linewidth=0.5, alpha=0.7)

    handles, labels = ax.get_legend_handles_labels()
    ncol = min(len(labels), 4) if labels else 1
    ax.legend(handles, labels, loc="lower center", bbox_to_anchor=(0.5, 1.02), ncol=ncol, frameon=False)

    fig.tight_layout()

    if out_path and not show:
        fig.savefig(out_path, dpi=dpi, bbox_inches="tight")
        print(f"Сохранено: {out_path}")
    if show or not out_path:
        plt.show()


def main():
    parser = argparse.ArgumentParser(description="Построение графика из файла с результатами эксперимента")
    parser.add_argument("input", help="Путь к входному файлу")
    parser.add_argument("-o", "--output", help="Путь для сохранения графика (PNG, SVG и т.п.)")
    parser.add_argument("--dpi", type=int, default=150, help="DPI при сохранении (по умолчанию 150)")
    parser.add_argument("--show", action="store_true", help="Показать окно с графиком")
    args = parser.parse_args()

    in_path = Path(args.input)
    if not in_path.exists():
        print(f"Файл не найден: {in_path}", file=sys.stderr)
        sys.exit(1)

    series_names, x_label, y_label, approx_type, x_arr, y_arrs = parse_file(in_path)
    plot(series_names, x_label, y_label, approx_type, x_arr, y_arrs,
         out_path=args.output, dpi=args.dpi, show=args.show)


if __name__ == "__main__":
    main()
