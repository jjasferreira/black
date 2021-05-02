//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <black/frontend/io.hpp>
#include <black/frontend/model.hpp>

#include <black/logic/parser.hpp>

namespace black::frontend {

  static 
  void collect_atoms(black::formula f, std::unordered_set<black::atom> &atoms) {
    using namespace black;
    f.match(
      [&](atom a) {
        atoms.insert(a);
      },
      [&](unary, formula f1) {
        collect_atoms(f1, atoms);
      },
      [&](binary, formula f1, formula f2) {
        collect_atoms(f1, atoms);
        collect_atoms(f2, atoms);
      }
    );
  }

  void print_model(black::solver &solver, black::formula f) {
    black_assert(solver.model().has_value());

    io::message("Model:", solver.model()->size());

    std::unordered_set<black::atom> atoms;
    collect_atoms(f, atoms);
    
    size_t size = solver.model()->size();
    size_t width = log10(size) + 1;
    for(size_t t = 0; t < size; ++t) {
      io::print(verbosity::message, "- t = {:>{}}: {{", t, width);
      bool first = true;
      for(atom a : atoms) {
        tribool v = solver.model()->value(a, t);
        const char *comma = first ? "" : ", ";
        if(v == true) {
          io::print(verbosity::message, "{}{}", comma, to_string(a));
          first = false;
        } else if(v == false) {
          io::print(verbosity::message, "￢{}{}", comma, to_string(a));
          first = false;
        }
      }
      io::print(verbosity::message, "}}");
      if(solver.model()->loop() == t)
        io::print(verbosity::message, " <- loops here");
      io::print(verbosity::message, "\n");
    }
  
  }

}