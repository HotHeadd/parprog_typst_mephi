#let titlepage(
  lab-no: 1,
  lab-topic: "",
) = [
    #set page(
    margin: 2cm,
  )
  #set page(numbering: none)
  #set text(font: "Times New Roman", size: 12pt)

  #show heading.where(level: 1): it => align(center, it)
  #align(center)[
    #text(size: 13pt)[Национальный исследовательский ядерный университет «МИФИ»]
    #v(0.25cm)
    #text(size: 13pt)[Институт интеллектуальных кибернетических систем]
    #v(0.25cm)
    #text(size: 13pt)[Кафедра № 42 «Криптология и кибербезопасность»]
    #v(1cm)

    #figure(
        grid(
            columns: 3,
            gutter: 17mm,    // space between columns
            [#image("../common_image/logo_university.jpg", width: 5cm)],
            [#image("../common_image/logo_institute.png", width: 5cm)],
            [#image("../common_image/kaf42.png", width: 5cm)]
        )
    )

    #v(16mm)
    #text(size: 26pt, weight: "bold")[ОТЧЕТ]

    #v(3mm)

    #text(size: 13pt, weight: "bold")[
      О выполнении лабораторной работы № #lab-no
      #linebreak()
      «#lab-topic»
    ]

    #v(1fr)

    #align(right)[
      #text(weight: "bold")[Cтудент:] Доценко В. А. #linebreak()
      #text(weight: "bold")[Группа:] Б23‑505 #linebreak()
      #text(weight: "bold")[Преподаватель:] Куприяшин М.А.
    ]

    #v(0.5cm)

    Москва — 2025
  ]
]

#let sem_titlepage() = [
    #set page(
    margin: 2cm,
  )
  #set page(numbering: none)
  #set text(font: "Times New Roman", size: 12pt)

  #show heading.where(level: 1): it => align(center, it)
  #align(center)[
    #text(size: 13pt)[Национальный исследовательский ядерный университет «МИФИ»]
    #v(0.25cm)
    #text(size: 13pt)[Институт интеллектуальных кибернетических систем]
    #v(0.25cm)
    #text(size: 13pt)[Кафедра № 42 «Криптология и кибербезопасность»]
    #v(1cm)

    #figure(
        grid(
            columns: 3,
            gutter: 17mm,    // space between columns
            [#image("../common_image/logo_university.jpg", width: 5cm)],
            [#image("../common_image/logo_institute.png", width: 5cm)],
            [#image("../common_image/kaf42.png", width: 5cm)]
        )
    )

    #v(16mm)
    #text(size: 26pt, weight: "bold")[ОТЧЕТ]

    #v(3mm)

    #text(size: 13pt, weight: "bold")[
      О проведении анализа алгоритма умножения матриц в параллельных сетях
      #linebreak()
    ]

    #v(1fr)

    #align(right)[
      #text(weight: "bold")[Выполнили:] Доценко В. А., Дзюба С.Д. #linebreak()
      #text(weight: "bold")[Группа:] Б23‑505 #linebreak()
      #text(weight: "bold")[Преподаватель:] Куприяшин М.А.
    ]

    #v(0.5cm)

    Москва — 2025
  ]
]
