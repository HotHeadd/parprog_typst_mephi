#import "@preview/fletcher:0.5.8" as f: diagram, node, edge, shapes.diamond, shapes.parallelogram, shapes.pill, shapes.hexagon

#let settings(doc)={
  set page(
    paper: "a4",
    margin: (x: 2cm, y: 1.5cm),
    numbering: "1",
  )
  set figure(
    numbering: "1",
    supplement: [Рисунок],
    gap: 20pt
  )
  set figure.caption(
    separator: [ \u{2013} ],
  )
  show figure.caption: set text(
    size: 12pt // размер шрифта в подписи
  )
  show figure.caption: set par(
    leading: 0.7em // оступ между строками в подписи
  )
  show figure.where(kind: "file"): set figure(
    supplement: "Файл"
  )
  show figure.where(kind: "table"): set figure(
    supplement: "Таблица",
  )
  show figure.where(kind: "table"): set figure.caption(
    position: top,
  )
  show figure.where(kind: "table"): set align(
    left
  )
  show figure.caption.where(kind: "table"): it => pad(left: 1.5em)[#it]

  set heading(
    numbering: "1.",
    bookmarked: true, // попадает в оглавление (outline)
  )
  show heading: it => pad(left: 1em * it.level, it) // отступ заголовка влево
  show heading: set block(below: 15pt)

  set text(
    font: "Times New Roman",
    size: 14pt,
  )
  set par(
    spacing: 1em, // расстояние между абзацам
    leading: 1em, // между строками
    first-line-indent: (
      all: true,
      amount: 1.5em
    ),
    justify: true
  )
  set enum(
    numbering: "1.a.i."
  )

  doc
}

#let listing-counter = counter("listing")

#let insert_code(code_path, code_caption) = {
  let source = read(code_path)
  block(
    par(
        text(
        size: 10pt,
        raw(source, lang: "c"),
      ),
      leading: 0.5em,
      first-line-indent: (amount: 0pt)
    ),
    stroke: none,
    fill: luma(98%), // лёгкий фон
    inset: 8pt,
    radius: 4pt,
    width: 100%,
  )
  context listing-counter.step()
  align(center)[
    #text("Файл " + context listing-counter.display() + [ \u{2013} ] + code_caption, size: 12pt)
  ]
}

#let insert_format_table(table_path, table_caption) = {
  let source = read(table_path)
  let lines = source.split("\n").filter(l => l != "")
  let cols = int(lines.at(0))
  let headers = lines.slice(1, cols + 1)
  let x_axis = lines.at(cols+1)
  let data = lines.slice(cols + 4)

  let rows = data.map(l => l.split(regex("\s+")).filter(x => x != ""))

  figure(
    table(
      columns: cols + 1,
      align: center,
      x_axis, ..headers,
      ..rows.flatten()
    ),
    caption: table_caption,
    kind: "table"
  )

}

#let n_if(nodeName, x, y, lbl, w: 33mm, h: 20mm) = node(name: nodeName, (x*2, y*2), lbl, shape: diamond, width: w, height: h)
#let n_io(nodeName, x, y, lbl) = node(name: nodeName, (x*2, y*2), lbl, shape:pill, width: 50mm, height: 20mm)
#let n_common(nodeName, x, y, lbl, s: 14pt) = node(name: nodeName, (x*2, y*2), text(lbl, size: s), width: 40mm, height: 20mm)

#let n_for_up(nodeName, x, y, varname, st_val, cond) = node(name: nodeName, (x*2, y*2), varname + " = " + str(st_val) + [;\ ] + varname + " " + cond + [;\ ++] + varname,  width: 40mm, height: 25mm, shape: hexagon)
#let n_for_down(nodeName, x, y, varname, st_val, cond) = node(name: nodeName, (x*2, y*2), varname + " = " + str(st_val) + [;\ ] + varname + " " + cond + [;\ \-\-] + varname,  width: 40mm, height: 25mm, shape: hexagon)
#let n_for_custom(nodeName, x, y, custom, s: 14pt) = node(name: nodeName, (x*2, y*2), text(custom, size: s),  width: 40mm, height: 25mm, shape: hexagon)

#let n_code(nodeName, x, y, lbl) = {
  node(name: nodeName, (x*2, y*2), width: 40mm, height: 20mm)
  node((x*2, y*2), lbl, width: 35mm, height: 20mm)
}

#let arrow = "-|>"
#let edge_if_y(ifname, ifpath) = edge(ifname, ifpath, label: "ДА", label-pos: 10%, label-sep: 0pt)
#let edge_if_n(ifname, ifpath, p: 10%) = edge(ifname, ifpath, label: "НЕТ", label-pos: p, label-sep: 0pt, arrow)

#let diagram-defaults = (
  cell-size: 10pt,
  edge-stroke: 0.7pt,
  node-stroke: 1pt,
  spacing: (8mm, 4.5mm),
)
