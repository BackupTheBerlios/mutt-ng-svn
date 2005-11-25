/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_sig Signal handling
 * @{
 */
/**
 * @file libmuttng/signal.h
 * @author Tero Pulkkinen <terop@modeemi.cs.tut.fi>
 * @author Esa Pulkkinen
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief (AUTO) Interface+Implementation: Signal handling
 */
#ifndef LIBMUTTNG_SIGNAL_H
#define LIBMUTTNG_SIGNAL_H

/* templates for 1 argument(s) {{{ */

/**
 * Interface for callback with 1 argument(s).
 * @param A1 Type of argument 1.
 */
template<class A1>
class Func1Interface {
  public:
    /** array of function interfaces */
    Func1Interface<A1>* next;
    /** constructor: init array */
    Func1Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a) = 0;
    /** destructor */
    virtual ~Func1Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func1Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 */
template<class T,class A1>
class Func1 : public Func1Interface<A1> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func1 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a) { return ((obj->*func)(a)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 1 argument(s).
 * @param A1 Type of argument 1.
 */
template <class A1>
class Signal1 {
  public:
    /** array typedef of callbacks registered */
    typedef Func1Interface<A1>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal1 () { first=0; running = false; }
    /** delete a signal */
    ~Signal1 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1>
void connectSignal (Signal1<A1>& signal,
                    T* object, bool (T::*callback)(A1 a)) {
  signal.insert (new Func1<T,A1>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1>
void disconnectSignals (Signal1<A1>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 2 argument(s) {{{ */

/**
 * Interface for callback with 2 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 */
template<class A1,class A2>
class Func2Interface {
  public:
    /** array of function interfaces */
    Func2Interface<A1,A2>* next;
    /** constructor: init array */
    Func2Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b) = 0;
    /** destructor */
    virtual ~Func2Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func2Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 */
template<class T,class A1,class A2>
class Func2 : public Func2Interface<A1,A2> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func2 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b) { return ((obj->*func)(a,b)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 2 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 */
template <class A1,class A2>
class Signal2 {
  public:
    /** array typedef of callbacks registered */
    typedef Func2Interface<A1,A2>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal2 () { first=0; running = false; }
    /** delete a signal */
    ~Signal2 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2>
void connectSignal (Signal2<A1,A2>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b)) {
  signal.insert (new Func2<T,A1,A2>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2>
void disconnectSignals (Signal2<A1,A2>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 3 argument(s) {{{ */

/**
 * Interface for callback with 3 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 */
template<class A1,class A2,class A3>
class Func3Interface {
  public:
    /** array of function interfaces */
    Func3Interface<A1,A2,A3>* next;
    /** constructor: init array */
    Func3Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b,A3 c) = 0;
    /** destructor */
    virtual ~Func3Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func3Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 */
template<class T,class A1,class A2,class A3>
class Func3 : public Func3Interface<A1,A2,A3> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b,A3 c);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func3 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b,A3 c) { return ((obj->*func)(a,b,c)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 3 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 */
template <class A1,class A2,class A3>
class Signal3 {
  public:
    /** array typedef of callbacks registered */
    typedef Func3Interface<A1,A2,A3>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal3 () { first=0; running = false; }
    /** delete a signal */
    ~Signal3 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b,A3 c) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b,c)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2,class A3>
void connectSignal (Signal3<A1,A2,A3>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b,A3 c)) {
  signal.insert (new Func3<T,A1,A2,A3>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2,class A3>
void disconnectSignals (Signal3<A1,A2,A3>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 4 argument(s) {{{ */

/**
 * Interface for callback with 4 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 */
template<class A1,class A2,class A3,class A4>
class Func4Interface {
  public:
    /** array of function interfaces */
    Func4Interface<A1,A2,A3,A4>* next;
    /** constructor: init array */
    Func4Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b,A3 c,A4 d) = 0;
    /** destructor */
    virtual ~Func4Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func4Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 */
template<class T,class A1,class A2,class A3,class A4>
class Func4 : public Func4Interface<A1,A2,A3,A4> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b,A3 c,A4 d);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func4 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b,A3 c,A4 d) { return ((obj->*func)(a,b,c,d)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 4 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 */
template <class A1,class A2,class A3,class A4>
class Signal4 {
  public:
    /** array typedef of callbacks registered */
    typedef Func4Interface<A1,A2,A3,A4>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal4 () { first=0; running = false; }
    /** delete a signal */
    ~Signal4 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b,A3 c,A4 d) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b,c,d)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2,class A3,class A4>
void connectSignal (Signal4<A1,A2,A3,A4>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b,A3 c,A4 d)) {
  signal.insert (new Func4<T,A1,A2,A3,A4>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2,class A3,class A4>
void disconnectSignals (Signal4<A1,A2,A3,A4>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 5 argument(s) {{{ */

/**
 * Interface for callback with 5 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 */
template<class A1,class A2,class A3,class A4,class A5>
class Func5Interface {
  public:
    /** array of function interfaces */
    Func5Interface<A1,A2,A3,A4,A5>* next;
    /** constructor: init array */
    Func5Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b,A3 c,A4 d,A5 e) = 0;
    /** destructor */
    virtual ~Func5Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func5Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 */
template<class T,class A1,class A2,class A3,class A4,class A5>
class Func5 : public Func5Interface<A1,A2,A3,A4,A5> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func5 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b,A3 c,A4 d,A5 e) { return ((obj->*func)(a,b,c,d,e)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 5 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 */
template <class A1,class A2,class A3,class A4,class A5>
class Signal5 {
  public:
    /** array typedef of callbacks registered */
    typedef Func5Interface<A1,A2,A3,A4,A5>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal5 () { first=0; running = false; }
    /** delete a signal */
    ~Signal5 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b,A3 c,A4 d,A5 e) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b,c,d,e)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5>
void connectSignal (Signal5<A1,A2,A3,A4,A5>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e)) {
  signal.insert (new Func5<T,A1,A2,A3,A4,A5>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5>
void disconnectSignals (Signal5<A1,A2,A3,A4,A5>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 6 argument(s) {{{ */

/**
 * Interface for callback with 6 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 */
template<class A1,class A2,class A3,class A4,class A5,class A6>
class Func6Interface {
  public:
    /** array of function interfaces */
    Func6Interface<A1,A2,A3,A4,A5,A6>* next;
    /** constructor: init array */
    Func6Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @param f Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b,A3 c,A4 d,A5 e,A6 f) = 0;
    /** destructor */
    virtual ~Func6Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func6Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6>
class Func6 : public Func6Interface<A1,A2,A3,A4,A5,A6> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func6 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f) { return ((obj->*func)(a,b,c,d,e,f)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 6 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 */
template <class A1,class A2,class A3,class A4,class A5,class A6>
class Signal6 {
  public:
    /** array typedef of callbacks registered */
    typedef Func6Interface<A1,A2,A3,A4,A5,A6>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal6 () { first=0; running = false; }
    /** delete a signal */
    ~Signal6 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @param f Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b,A3 c,A4 d,A5 e,A6 f) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b,c,d,e,f)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6>
void connectSignal (Signal6<A1,A2,A3,A4,A5,A6>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f)) {
  signal.insert (new Func6<T,A1,A2,A3,A4,A5,A6>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6>
void disconnectSignals (Signal6<A1,A2,A3,A4,A5,A6>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */

/* templates for 7 argument(s) {{{ */

/**
 * Interface for callback with 7 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 * @param A7 Type of argument 7.
 */
template<class A1,class A2,class A3,class A4,class A5,class A6,class A7>
class Func7Interface {
  public:
    /** array of function interfaces */
    Func7Interface<A1,A2,A3,A4,A5,A6,A7>* next;
    /** constructor: init array */
    Func7Interface() : next(0) {}
    /**
     * the actual function call.
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @param f Argument passed to signal's emit(). 
     * @param g Argument passed to signal's emit(). 
     * @return Whether callback succeeded.
     */
    virtual bool call (A1 a,A2 b,A3 c,A4 d,A5 e,A6 f,A7 g) = 0;
    /** destructor */
    virtual ~Func7Interface () {}
    /**
     * get pointer to object the callback is a member of
     * @c void* is okay since we compare pointers only. Hopefully
     * they're constant during lifetime of an object (otherwise we
     * couldn't call the function anyway.)
     * @return Pointer.
     */
     virtual void* getObjPtr() = 0;
};

/**
 * Specialization of Func7Interface knowing about the object type the
 * callback is a member of. This is required to find the object to call
 * a member of.
 * @param T Type of class.
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 * @param A7 Type of argument 7.
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6,class A7>
class Func7 : public Func7Interface<A1,A2,A3,A4,A5,A6,A7> {
  public:
    /** for readability: typedef of function pointer */
    typedef bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f,A7 g);
  private:
    /** object the callback is a member of */
    T* obj;
    /** the callback function */
    callback func;
  public:
    /**
     * Constructor.
     * @param obj_ Object
     * @param func_ Callback function.
     */
    Func7 (T* obj_, callback func_) : obj(obj_),func(func_) {}
    bool call(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f,A7 g) { return ((obj->*func)(a,b,c,d,e,f,g)); }
    void* getObjPtr () { return ((void*) obj); }
};

/**
 * Signal handler for allowing connects with 7 argument(s).
 * @param A1 Type of argument 1.
 * @param A2 Type of argument 2.
 * @param A3 Type of argument 3.
 * @param A4 Type of argument 4.
 * @param A5 Type of argument 5.
 * @param A6 Type of argument 6.
 * @param A7 Type of argument 7.
 */
template <class A1,class A2,class A3,class A4,class A5,class A6,class A7>
class Signal7 {
  public:
    /** array typedef of callbacks registered */
    typedef Func7Interface<A1,A2,A3,A4,A5,A6,A7>* callbacks;
  private:
    /** list of callbacks */
    callbacks first;
    /**
     * for loop prevention: whether we're running.
     * @test signal_tests::test_signal_loop().
     */
    bool running;
  public:
    /** constructor */
    Signal7 () { first=0; running = false; }
    /** delete a signal */
    ~Signal7 () {
      for (callbacks i = first; i != 0 ; ) {
        callbacks k = i->next;
        delete i;
        i = k;
      }
    }
    /**
     * Register a new callback.
     * @param item Callback.
     * @test signal_tests::test_signal_connect().
     */
    void insert (callbacks item) {
      item->next = first;
      first = item;
    }
    /**
     * Remove all bindings for given object from signal.
     * @param object Object.
     * @test signal_tests::test_signal_connect().
     */
    template<class T> void removeAll (T* object) {
      callbacks last = first;
      callbacks i = first;
      while (i) {
        if ((void*) object == i->getObjPtr()) {
          if (first == i) {
            first = i->next;
            delete i;
            i = first;
          } else {
            last->next = i->next;
            delete i;
            i = last->next;
          }
        } else {
          last = i;
          i = i->next;
        }
      }
    }
    /**
     * Emit a signal, ie call all connected functions.
     * @test signal_tests::test_signal_connect().
     * @test signal_tests::test_signal_loop().
     * @test signal_tests::test_signal_disconnect().
     * @param a Argument passed to signal's emit(). 
     * @param b Argument passed to signal's emit(). 
     * @param c Argument passed to signal's emit(). 
     * @param d Argument passed to signal's emit(). 
     * @param e Argument passed to signal's emit(). 
     * @param f Argument passed to signal's emit(). 
     * @param g Argument passed to signal's emit(). 
     * @return Whether all calls succeeded.
     */
    bool emit (A1 a,A2 b,A3 c,A4 d,A5 e,A6 f,A7 g) {
      if (running)
        return (false);
      running = true;
      for (callbacks i = first; i; i = i->next)
        if (!i->call (a,b,c,d,e,f,g)) {
          running = false;
          return (false);
        }
      running = false;
      return (true);
    }
};

/**
 * Connect a function to a signal.
 * @param signal Target signal to connect to.
 * @param object The object the handler is a member of.
 * @param callback The callback function itself.
 * @test signal_tests::test_signal_connect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6,class A7>
void connectSignal (Signal7<A1,A2,A3,A4,A5,A6,A7>& signal,
                    T* object, bool (T::*callback)(A1 a,A2 b,A3 c,A4 d,A5 e,A6 f,A7 g)) {
  signal.insert (new Func7<T,A1,A2,A3,A4,A5,A6,A7>(object,callback));
}

/**
 * Remove all bindings of an object from a signal.
 * @param signal Signal.
 * @param object Object.
 * @test signal_tests::test_signal_disconnect().
 */
template<class T,class A1,class A2,class A3,class A4,class A5,class A6,class A7>
void disconnectSignals (Signal7<A1,A2,A3,A4,A5,A6,A7>& signal, T* object) {
  signal.removeAll (object);
}

/* }}} */


#endif /* !LIBMUTTNG_SIGNAL_H */
/** @} */
