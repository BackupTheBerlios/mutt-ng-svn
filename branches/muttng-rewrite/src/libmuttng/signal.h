/* doxygen documentation {{{ */
/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_sig Signal handling
 * @{
 *
 * This is a very simple signal implementation I stole from:
 * <a href="http://lists.trolltech.com/qt-interest/1997-07/msg00158.html
 * ">http://lists.trolltech.com/qt-interest/1997-07/msg00158.html</a>.
 *
 * @section libmuttng_sig_decl Signal declaration
 *
 *   Declaring a signal is as easy as:
 *
 *   <pre>
 * SignalX<X0,X1,...> SignalFunction;</pre>
 *
 *   ...whereby @c X is a number, 1 or 2 currently. As the implementation
 *   here is very simple, the number specifies how many parameters
 *   will be passed down to handlers. For X=1 one needs to specify one
 *   argument, and two for X=2. An example we use is this:
 *
 *   <pre>
 * Signal2<Event::context,option_t*> sigOptChange;</pre>
 *
 *   ...as a member of the Event class. Any function taking a context and
 *   a pointer to option_t as argument can now bind to the handler.
 *
 * @section libmuttng_sig_bind Binding/Connecting
 *
 *   @c signal.h provides an overloaded function called @c
 *   connectSignal() which takes three arguments:
 *
 *     -# the signal
 *     -# a pointer to the object the callback is a member of
 *        (in the vast majority of cases this is @c this)
 *     -# a pointer to the actual callback member function.
 *
 *   All handlers for signals must return @c bool.
 *
 *   For example, for the Tool class we catch changes of options to
 *   adjust debug level internally. The function @c
 *   Tool::catchOptChange() takes the context and the pointer to the
 *   option changed as arguments. Binding is done via:
 *
 *   <pre>
 * connectSignal (event->sigOptChange, this, &Tool::catchOptChange);</pre>
 *
 *   Due to overloading, this always is the same independent from what
 *   argument a function takes and how many there are.
 *
 * @section libmuttng_sig_unbind Unbinding/Disconnecting
 *
 *   ...is currently not supported and not really needed (yet?).
 *
 * @section libmuttng_sig_emit Emitting
 *
 *   The various SignalX classes have a method called @c emit() which
 *   takes the arguments the signal was declared with. It returns
 *   @c bool only and executes the registered callbacks while none of
 *   them failed. The value returned specifies whether all callbacks
 *   succeeded.
 *
 *   For example, in @c set_command.cpp a change of an option is
 *   detected and emitted like this:
 *
 *   <pre>
 * event->sigOptChange.emit (event->getContext(), &Options[i]);</pre>
 *
 *   @b NOTE: For functions passing a context, we still have bad
 *   modularization meaning the event handler doesn't intersect the emit
 *   process to ensure the context passed is the current. This means
 *   that routines emitting a signal with an Event::context should
 *   always use @c Event::getContext() to retrieve the current.
 */
/* }}} */
/**
 * @file libmuttng/signal.h
 * @author Tero Pulkkinen <terop@modeemi.cs.tut.fi>
 * @author Esa Pulkkinen
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Signal handling
 */
#ifndef LIBMUTTNG_SIGNAL_H
#define LIBMUTTNG_SIGNAL_H

/* templates for one argument callbacks {{{ */

/**
 * Interface for callback with 1 argument.
 * @param a1 First argument type.
 */
template<class a1>
class Func1Interface {
  public:
    /** array of function interfaces */
    Func1Interface<a1>* next;
    /** initialize array */
    Func1Interface() : next(0) {}
    /**
     * the actual function call
     * @param x Argument passed to signal's emit function.
     * @return Whether call succeeded.
     */
    virtual bool call(a1 x) = 0;
    /** destructor */
    virtual ~Func1Interface() {}
};

/**
 * Specialization of Func1Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param a1 First argument type.
 */
template<class T, class a1>
class Func1 : public Func1Interface<a1> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(a1);
  private:
    /** object the callback is a member of */
    T *obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param f Callback function.
     */
    Func1 (T* obj_, callback f) : obj(obj_),func(f) {}
    bool call(a1 x) { return ((obj->*func)(x)); }
};

/**
 * Signal handler for allowing connects with one argument.
 * @param a1 First argument type.
 */
template<class a1>
class Signal1 {
  public:
    /** array typedef of callbacks registered */
    typedef Func1Interface<a1>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /** loop-prevention: whether we're running */
    bool running;
  public:
    /** constructor */
    Signal1 () { first=0; running = false; }
    /** delete a signal */
    ~Signal1 () {
      for(callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i=k;
      }
    }
    /**
     * register a new callback
     * @param item Callback.
     */
    void insert(callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @param x First argument.
     * @return Whether all calls succeeded.
     */
    bool emit (a1 x) {
      bool ret = false;
      if (!running) {
        running = true;
        for (callbacks i = first; i; i = i->next)
          if (!(ret = i->call (x))) {
            break;
          }
      } else {
      }
      running = false;
      return (ret);
    }
};

/* }}} */

/* templates for two argument callbacks {{{ */

/**
 * Interface for callback with 1 argument.
 * @param a1 First argument type.
 * @param a2 Second argument type.
 */
template<class a1, class a2>
class Func2Interface {
  public:
    /** array of function interfaces */
    Func2Interface<a1,a2>* next;
    /** initialize array */
    Func2Interface() : next(0) {}
    /**
     * the actual function call
     * @param x Argument passed to signal's emit function.
     * @param y Argument passed to signal's emit function.
     * @return Whether call succeeded.
     */
    virtual bool call(a1 x, a2 y) = 0;
    /** destructor */
    virtual ~Func2Interface() {}
};

/**
 * Specialization of Func1Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param a1 First argument type.
 */
template<class T, class a1, class a2>
class Func2 : public Func2Interface<a1,a2> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(a1,a2);
  private:
    /** object the callback is a member of */
    T *obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param f Callback function.
     */
    Func2 (T* obj_, callback f) : obj(obj_),func(f) {}
    bool call(a1 x, a2 y) { return ((obj->*func)(x,y)); }
};

/**
 * Signal handler for allowing connects with one argument.
 * @param a1 First argument type.
 */
template<class a1, class a2>
class Signal2 {
  public:
    /** array typedef of callbacks registered */
    typedef Func2Interface<a1,a2>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /** loop-prevention: whether we're running */
    bool running;
  public:
    /** constructor */
    Signal2 () { first=0; running = false; }
    /** delete a signal */
    ~Signal2 () {
      for(callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i=k;
      }
    }
    /**
     * register a new callback
     * @param item Callback.
     */
    void insert(callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @param x First argument.
     * @param y Second argument.
     * @return Whether all calls succeeded.
     */
    bool emit (a1 x, a2 y) {
      bool ret = false;
      if (!running) {
        running = true;
        for (callbacks i = first; i; i = i->next)
          if (!(ret = i->call (x,y))) {
            break;
          }
      } else {
      }
      running = false;
      return (ret);
    }
};

/* }}} */

/* connect functions {{{ */

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 */
template<class T, class a1>
void connectSignal (Signal1<a1>& signal, T* object, bool (T::*callback)(a1)) {
  signal.insert (new Func1<T,a1>(object,callback));
}

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 */
template<class T, class a1, class a2>
void connectSignal (Signal2<a1,a2>& signal, T* object, bool (T::*callback)(a1,a2)) {
  signal.insert (new Func2<T,a1,a2>(object,callback));
}

/* }}} */

#endif /* !LIBMUTTNG_SIGNAL_H */

/** @} */
