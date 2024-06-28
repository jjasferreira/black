//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_PIPES_PIPELINE_HPP
#define BLACK_PIPES_PIPELINE_HPP

#include <black/support>
#include <black/logic>

#include <vector>
#include <array>

namespace black::pipes {

  template<typename P, typename T, typename ...Args>
  struct make_pipe {
    constexpr make_pipe() = default;
    
    make_pipe(make_pipe const&) = default;
    make_pipe(make_pipe &&) = default;

    make_pipe &operator=(make_pipe const&) = default;
    make_pipe &operator=(make_pipe &&) = default;

    template<typename ...Args2>
      requires std::is_constructible_v<P, Args..., Args2...>
    typename T::pipeline operator()(Args2 ...args2) const {
      return [... args3 = std::move(args2)](Args ...args) 
        -> typename T::instance 
      { 
        return std::make_unique<P>( std::move(args)..., std::move(args3)... );
      };
    }
  };

  //!
  //! Class handling instances of tranform pipelines.
  //!
  //! A \ref transform can be created from any transform::pipeline and actually
  //! instantiates the pipeline components and sets them up to stream the
  //! transform.
  //!
  //! One can actually apply the transform by calling the \ref transform object
  //! on a given \ref module, obtaining the transformed one. The transformation
  //! is incremental: at the next call only the differences with the previous
  //! one will be streamed.
  //!
  //! Example:
  //!
  //! ```cpp
  //!    module mod = ...;
  //!    pipes::transform nothing = pipes::id() | pipes::id();
  //!
  //!    module mod2 = nothing(mod);
  //!    assert(mod == mod2);
  //!
  //!    mod.require(x > 0);
  //!    mod2 = nothing(mod); // only the last requirement is streamed here
  //!
  //!    assert(mod == mod2);
  //! ```
  //!
  //! \note Since transforms will generally carry complex state with them, \ref
  //! transform objects are *move-only*.
  //!
  class transform
  {
  public:
    class base;
    
    //! Type reprsenting an instantiated pipeline stage.
    using instance = std::unique_ptr<base>;

    //! Type representing transform pipeline stages before instantiation.
    using pipeline = std::function<instance(consumer *next)>;

    //! @name Constructor
    //! @{

    //! Constructs the transform from a transform::pipeline object.
    transform(pipeline p);

    //! @}

    transform(transform const&) = delete;
    transform(transform &&) = default;
    
    transform &operator=(transform const&) = delete;
    transform &operator=(transform &&) = default;

    //! Gets a pointer to the underlying instance of \ref transform::base.
    base *get() { return _instance.get(); }

    //! Applies the transform to the given module.
    logic::module operator()(logic::module mod);

  private:
    logic::module _last;
    logic::module _output;
    instance _instance;
  };

  //!
  //! Abstract base class for transform pipeline stages.
  //!
  //! A pipeline stage has to provide three elements:
  //! 1. a pointer to a \ref consumer objects ready to stream (the consumer()
  //!    function);
  //! 2. a way to translate \ref object instances from the world before the
  //!    transform to the world after the transform (the translate() function);
  //! 3. a way to map back values from models obtained by solvers after the
  //!    transform to terms that make sense before the transform.
  //!
  class transform::base {
  public:
    virtual ~base() = default;

    //! The consumer implementing the transform.
    virtual class consumer *consumer() = 0;

    //! The translation function for \ref object instances.
    virtual std::optional<logic::object> translate(logic::object) = 0;

    //! The back-mapping function for model terms.
    virtual logic::term undo(logic::term) = 0;
  };

  inline transform::transform(pipeline p) : _instance{p(&_output)} { }

  inline logic::module transform::operator()(logic::module mod) {
    mod.replay(_last, _instance->consumer());
    _last = std::move(mod);
    return _output;
  }

  //!
  //! Utility type to declare pipeline factory objects.
  //!
  //! Given a type deriving from transform::base, the result is an object usable
  //! as a factory for the corresponding transform::pipeline.
  //!
  //! For example, this is the implementation of the pipes::id() transform:
  //!
  //! ```cpp
  //! class id_t : public transform::base
  //! {
  //! public:
  //!   id_t(class consumer *next) : _next{next} { }
  //!     
  //!   virtual class consumer *consumer() override { return _next; }
  //! 
  //!   virtual std::optional<logic::object> translate(logic::object) override 
  //!   { 
  //!     return {};
  //!   }
  //! 
  //!   virtual logic::term undo(logic::term t) override { return t; }
  //! 
  //! private:
  //!   class consumer *_next;
  //! };
  //!
  //! inline constexpr auto id = make_transform<id_t>{};
  //! ```
  //!
  template<typename P>
  using make_transform = make_pipe<P, transform, consumer *>;

  //
  //
  //
  class solver
  {
  public:
    class base;
    using instance = std::unique_ptr<base>;
    using pipeline = std::function<instance()>;

    solver(pipeline p);

    solver(solver const&) = delete;
    solver(solver &&) = default;
    
    solver &operator=(solver const&) = delete;
    solver &operator=(solver &&) = default;

    base *get() { return _instance.get(); }

    void set_smt_logic(std::string const& logic);
    support::tribool check(logic::module mod);
    std::optional<logic::term> value(logic::object x);

  private:
    logic::module _last;
    instance _instance;
  };

  class solver::base {
  public:
    virtual ~base() = default;

    virtual void set_smt_logic(std::string const&logic) = 0;

    virtual pipes::consumer *consumer() = 0;

    virtual support::tribool check() = 0;

    virtual std::optional<logic::term> value(logic::object) = 0;
  };

  inline solver::solver(pipeline p) : _instance{p()} { }

  inline void solver::set_smt_logic(std::string const& logic) {
    _instance->set_smt_logic(logic);
  }

  inline support::tribool solver::check(logic::module mod) {
    mod.replay(_last, _instance->consumer());
    _last = std::move(mod);
    return _instance->check();
  }

  inline std::optional<logic::term> solver::value(logic::object x) {
    return _instance->value(x);
  }

  template<typename P>
  using make_solver = make_pipe<P, solver>;

}

#endif // BLACK_PIPES_PIPELINE_HPP
