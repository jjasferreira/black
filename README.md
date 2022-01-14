# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black) ![appveyor](https://ci.appveyor.com/api/projects/status/github/black-sat/black?branch=master&svg=true) ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v0.7.0) [![codecov](https://codecov.io/gh/black-sat/black/branch/master/graph/badge.svg?token=ZETQF5NZ6X)](https://codecov.io/gh/black-sat/black)

BLACK (short for Bounded Lᴛʟ sAtisfiability ChecKer) is a tool for testing the
satisfiability of LTL and LTLf formulas based on the SAT encoding of the tableau
method described [here][Reynolds]. An in depth description of the encoding and
the whole algorithm has been published in the proceedings of the TABLEAUX 2019
conference (see the Publications section below).

BLACK is:
* **Fast**: based on a state-of-the-art SAT-based encoding 
* **Lightweight**: low memory consuption even for large formulas
* **Flexible**: supports LTL and LTL+Past both on infinite and finite models
* **Robust**: rock-solid stability with 100% test coverage
* **Multiplatform**: works on Linux, macOS, Windows and FreeBSD
* **Easy to use**: easy to install binary packages provided for all platforms
* **Embeddable**: use BLACK's library API to integrate BLACK's solver into your code

See the [Documentation][Home] on how to use BLACK. In particular:

1. [Downloads and Installation][Installation]
2. [Usage][Usage]
3. (coming soon...) BLACK Library API

Quick usage help:
```
$ black --help

BLACK - Bounded Lᴛʟ sAtisfiability ChecKer
        version 0.7.0


SYNOPSIS
   ./black solve [-k <bound>] [-B <backend>] [--remove-past] [--finite] [-m]
           [-d <sort>] [-s] [-o <fmt>] [-f <formula>] [--debug <debug>]
           [<file>]

   ./black check -t <trace> [-e <result>] [-i <state>] [--finite] [--verbose]
           [-f <formula>] [<file>]

   ./black dimacs [-B <backend>] <file>
   ./black --sat-backends
   ./black -v
   ./black -h

OPTIONS
   solving mode: 
       -k, --bound <bound>         maximum bound for BMC procedures
       -B, --sat-backend <backend> select the SAT backend to use
       --remove-past               translate LTL+Past formulas into LTL before
                                   checking satisfiability

       --finite                    treat formulas as LTLf and look for finite
                                   models

       -m, --model                 print the model of the formula, if any
       -d, --domain <sort>         select the domain for first-order variables.
                                   Mandatory for first-order formulas.
                                   Accepted domains: integers, reals
       -s, --semi-decision         disable termination checks for unsatisfiable
                                   formulas, speeding up the execution for
                                   satisfiable ones.
                                   Note: the use of `next(x)` terms in formulas
                                   implies this option.

       -o, --output-format <fmt>   Output format.
                                   Accepted formats: readable, json
                                   Default: readable
       -f, --formula <formula>     LTL formula to solve
       <file>                      input formula file name.
                                   If '-', reads from standard input.

   trace checking mode: 
       -t, --trace <trace>         trace file to check against the formula.
                                   If '-', reads from standard input.
       -e, --expected <result>     expected result (useful in testing)
       -i, --initial-state <state> index of the initial state over which to
                                   evaluate the formula. Default: 0

       --finite                    treat formulas as LTLf and expect a finite
                                   model

       --verbose                   output a verbose log
       -f, --formula <formula>     formula against which to check the trace
       <file>                      formula file against which to check the
                                   trace

   DIMACS mode: 
       -B, --sat-backend <backend> select the SAT backend to use
       <file>                      DIMACS file to solve

   --sat-backends                  print the list of available SAT backends
   -v, --version                   show version and license information
   -h, --help                      print this help message
```

## Publications
The algorithm and encoding employed by BLACK is described in detail in this paper:

[Luca Geatti][Geatti], [Nicola Gigante][Gigante], [Angelo Montanari][Montanari]  
A SAT-based encoding of the one-pass and tree-shaped tableau system for LTL.  
In: *Proceedings of the 28th International Conference on Automated Reasoning with 
Analytic Tableaux and Related Methods*, [TABLEAUX 2019][Tableaux], pages 3‑20  
DOI: 10.1007/978-3-030-29026-9_1  
[Full Text][TableauxPaper], [DBLP][TableauxDBLP]

You can cite the above paper with the following BibTeX entry:

```
@inproceedings{GeattiGM19,
  author    = {Luca Geatti and
               Nicola Gigante and
               Angelo Montanari},
  editor    = {Serenella Cerrito and Andrei Popescu},
  title     = {A SAT-Based Encoding of the One-Pass and Tree-Shaped Tableau System
               for {LTL}},
  booktitle = {Proceedings of the 28th International Conference on Automated Reasoning 
               with Analytic Tableaux and Related Methods},
  series    = {Lecture Notes in Computer Science},
  volume    = {11714},
  pages     = {3--20},
  publisher = {Springer},
  year      = {2019},
  doi       = {10.1007/978-3-030-29026-9\_1}
}
```

The handling of *past operators* has been described in the following paper:

[Luca Geatti][Geatti], [Nicola Gigante][Gigante], [Angelo Montanari][Montanari], [Gabriele Venturato][Venturato]  
Past Matters: Supporting LTL+Past in the BLACK Satisfiability Checker.   
In: *Proceedings of the 28th International Symposium on Temporal Representation and Reasoning*,
[TIME 2021][TIME], pages 8:1-8:17,  
DOI: 10.4230/LIPIcs.TIME.2021.8  
[Full Text][TIMEPaper], [DBLP][TIMEDBLP]

```
@inproceedings{DBLP:conf/time/GeattiGMV21,
  author    = {Luca Geatti and
               Nicola Gigante and
               Angelo Montanari and
               Gabriele Venturato},
  editor    = {Carlo Combi and
               Johann Eder and
               Mark Reynolds},
  title     = {Past Matters: Supporting LTL+Past in the {BLACK} Satisfiability Checker},
  booktitle = {Proceedings of th e28th International Symposium on Temporal Representation 
               and Reasoning},
  series    = {LIPIcs},
  volume    = {206},
  pages     = {8:1--8:17},
  publisher = {Schloss Dagstuhl - Leibniz-Zentrum f{\"{u}}r Informatik},
  year      = {2021},
  doi       = {10.4230/LIPIcs.TIME.2021.8}
}
```

[Reynolds]: https://arxiv.org/abs/1609.04102
[CMake]: https://cmake.org
[zlib]: https://zlib.net/
[hopscotch]: https://github.com/Tessil/hopscotch-map
[CMS]: https://github.com/msoos/cryptominisat
[MiniSAT]: http://minisat.se/
[Z3]: https://github.com/Z3Prover/z3
[MathSAT]: http://mathsat.fbk.eu
[Homebrew]: https://brew.sh
[Geatti]: https://users.dimi.uniud.it/~luca.geatti
[Gigante]: https://users.dimi.uniud.it/~nicola.gigante
[Montanari]: https://users.dimi.uniud.it/~angelo.montanari
[Venturato]: https://gabventurato.github.io/
[Tableaux]: https://tableaux2019.org/
[TIME]: https://conference2.aau.at/event/61/
[TableauxPaper]: https://users.dimi.uniud.it/~nicola.gigante/papers/GeattiGM19.pdf
[TableauxDBLP]: https://dblp.org/rec/conf/tableaux/GeattiGM19.html
[TIMEPaper]: https://drops.dagstuhl.de/opus/volltexte/2021/14784/
[TIMEDBLP]: https://dblp.org/rec/conf/time/GeattiGMV21.html
[Home]: https://github.com/black-sat/black/wiki/Home 
[Installation]: https://github.com/black-sat/black/wiki/Installation 
[Usage]: https://github.com/black-sat/black/wiki/Usage 
