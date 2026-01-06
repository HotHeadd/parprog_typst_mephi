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

    # Легенда
    handles, labels = ax.get_legend_handles_labels()
    ncol = min(len(labels), 4) if labels else 1
    ax.legend(handles, labels, loc="lower center", bbox_to_anchor=(0.5, 1.02), ncol=ncol, frameon=False)

        # ---- Выбрать угол, максимально удалённый от данных (с учётом реального bbox текста) ----
    bench_text = (
        "Apple M3 | 8/8 | 24GB\n"
        "macOS 15.6 | clang 16\n"
        "OpenMP 4.0 | N = 30 млн. | 100 repeats"
    )

    # отступ от краёв области графика (axes fraction)
    margin = 0.07

    corners = [
        (margin, 1 - margin, "left",  "top"),     # UL
        (1 - margin, 1 - margin, "right", "top"), # UR
        (margin, margin, "left",  "bottom"),      # LL
        (1 - margin, margin, "right", "bottom"),  # LR
    ]

    fig.canvas.draw()
    renderer = fig.canvas.get_renderer()

    # диапазоны данных для нормализации
    x_min, x_max = float(np.min(x_arr)), float(np.max(x_arr))
    all_y = np.hstack(y_arrs) if len(y_arrs) > 0 else np.array([0.0])
    y_min, y_max = float(np.min(all_y)), float(np.max(all_y))
    dx = x_max - x_min or 1.0
    dy = y_max - y_min or 1.0

    def bbox_data_coords_for_text(fx, fy, ha, va):
        """Вернуть bbox текста в координатах данных (x_lo,x_hi,y_lo,y_hi)."""
        txt = ax.text(
            fx, fy, bench_text,
            transform=ax.transAxes,
            ha=ha, va=va,
            fontsize=9,
            linespacing=1.25,
            alpha=0.0  # невидимый при измерении
        )
        fig.canvas.draw()
        bbox_disp = txt.get_window_extent(renderer)
        txt.remove()  # убираем временный объект

        inv = ax.transData.inverted()
        x0, y0 = inv.transform((bbox_disp.x0, bbox_disp.y0))
        x1, y1 = inv.transform((bbox_disp.x1, bbox_disp.y1))
        x_lo, x_hi = sorted((x0, x1))
        y_lo, y_hi = sorted((y0, y1))
        return x_lo, x_hi, y_lo, y_hi

    def point_rect_distance(px, py, x_lo, x_hi, y_lo, y_hi):
        """Евклид. расстояние точки до прямоугольника в data coords, затем нормализовано."""
        dx_out = 0.0
        if px < x_lo:
            dx_out = x_lo - px
        elif px > x_hi:
            dx_out = px - x_hi
        dy_out = 0.0
        if py < y_lo:
            dy_out = y_lo - py
        elif py > y_hi:
            dy_out = py - y_hi
        # нормализация по общим диапазонам
        nx = dx_out / dx
        ny = dy_out / dy
        return np.hypot(nx, ny)

    # Для каждого угла: получаем bbox в data coords и считаем минимальное расстояние
    corner_scores = []
    for fx, fy, ha, va in corners:
        x_lo, x_hi, y_lo, y_hi = bbox_data_coords_for_text(fx, fy, ha, va)

        # Минимальное расстояние от bbox до любой точки (в нормализованных единицах)
        min_dist = float("inf")
        for y in y_arrs:
            # векторизированно вычислим расстояния для всех точек серии
            px = x_arr
            py = y
            # расстояния к прямоугольнику для векторов
            dxs = np.where(px < x_lo, x_lo - px, np.where(px > x_hi, px - x_hi, 0.0))
            dys = np.where(py < y_lo, y_lo - py, np.where(py > y_hi, py - y_hi, 0.0))
            dists = np.hypot(dxs / dx, dys / dy)
            cur_min = float(np.min(dists))
            if cur_min < min_dist:
                min_dist = cur_min

        corner_scores.append(((fx, fy, ha, va), min_dist))

    # Выбираем угол с максимальным минимальным расстоянием (т.е. максимально удалённый)
    best, best_dist = max(corner_scores, key=lambda t: t[1])

    # Если все углы дают нулевое расстояние (крайний случай), всё равно выбираем лучший из них
    fx, fy, ha, va = best

    # Рисуем финальную подпись
    ax.text(
        fx, fy, bench_text,
        transform=ax.transAxes,
        ha=ha,
        va=va,
        multialignment="left",
        fontsize=9,
        linespacing=1.25,
        bbox=dict(
            facecolor="white",
            edgecolor="0.8",
            alpha=0.95,
            boxstyle="round,pad=0.35"
        ),
        zorder=10
    )


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
