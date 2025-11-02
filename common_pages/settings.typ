#import "@preview/fletcher:0.5.8" as f: diagram, node, edge, shapes.diamond, shapes.parallelogram, shapes.pill, shapes.hexagon

#let settings(doc)={
  set page(
    paper: "a4",
    margin: (x: 1.8cm, y: 1.5cm),
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
  show figure.where(kind: "file"): set figure(
    supplement: "Файл"
  )
  show figure.where(kind: "table"): set figure(
    supplement: "Таблица",
  )
  show figure.where(kind: "table"): set figure.caption(
    position: top,
  )

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
      amount: 1.25em
    ),
    justify: true
  )
  set enum(
    numbering: "1.a.i."
  )

  doc
}

#let insert_code(code_path, code_caption)={
  let source = read(code_path) // requires absolute path
  figure(
    align(left)[
      #par(
        text(
          size: 12pt,
          raw(source, lang: "c")
        ),
        leading: 0.3em,
        first-line-indent: (
          amount: 0em
        )
      )
    ],
    caption: code_caption,
    kind: "file"
  )
}

#let n_if(nodeName, x, y, lbl, w: 33mm, h: 20mm) = node(name: nodeName, (x*2, y*2), lbl, shape: diamond, width: w, height: h)
#let n_io(nodeName, x, y, lbl) = node(name: nodeName, (x*2, y*2), lbl, shape:pill, width: 50mm, height: 20mm)
#let n_common(nodeName, x, y, lbl) = node(name: nodeName, (x*2, y*2), lbl, width: 40mm, height: 20mm)

#let n_for_up(nodeName, x, y, varname, st_val, cond) = node(name: nodeName, (x*2, y*2), varname + " = " + str(st_val) + [;\ ] + varname + " " + cond + [;\ ++] + varname,  width: 40mm, height: 25mm, shape: hexagon)
#let n_for_down(nodeName, x, y, varname, st_val, cond) = node(name: nodeName, (x*2, y*2), varname + " = " + str(st_val) + [;\ ] + varname + " " + cond + [;\ \-\-] + varname,  width: 40mm, height: 25mm, shape: hexagon)

#let n_code(nodeName, x, y, lbl) = {
  node(name: nodeName, (x*2, y*2), width: 40mm, height: 20mm)
  node((x*2, y*2), lbl, width: 35mm, height: 20mm)
}

#let edge_if_y(ifname, ifpath) = edge(ifname, ifpath, label: "ДА", label-pos: 10%, label-sep: 0pt)
#let edge_if_n(ifname, ifpath) = edge(ifname, ifpath, label: "НЕТ", label-pos: 10%, label-sep: 0pt)
#let arrow = "-|>"

#let diagram-defaults = (
  cell-size: 10pt,
  edge-stroke: 0.7pt,
  node-stroke: 1pt,
  spacing: (8mm, 4.5mm),
)
