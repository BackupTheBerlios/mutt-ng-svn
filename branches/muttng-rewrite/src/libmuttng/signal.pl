#!/usr/bin/perl -w

use strict;

my $max = 7;
my @params = ("", "a", "b", "c", "d", "e", "f", "g");

my $head =
"/**\n" .
" * \@ingroup libmuttng\n" .
" * \@addtogroup libmuttng_sig Signal handling\n" .
" * \@{\n" .
" * For implementation details and an example, please see\n" .
" * \@ref sect_devguide-libmuttng-signal.\n" .
" */\n" .
"/**\n" .
" * \@file libmuttng/muttng_signal.h\n" .
" * \@author Tero Pulkkinen <terop\@modeemi.cs.tut.fi>\n" .
" * \@author Esa Pulkkinen\n" .
" * \@author Rocco Rutte <pdmef\@cs.tu-berlin.de>\n" .
" * \@brief (AUTO) Interface+Implementation: Signal handling\n" .
" */\n" .
"#ifndef LIBMUTTNG_SIGNAL_H\n" .
"#define LIBMUTTNG_SIGNAL_H\n\n";

my $tail = 
"\n" .
"#endif /* !LIBMUTTNG_SIGNAL_H */\n" .
"/** \@} */\n";

my $sep = "";

my $template_funcXint = "";
my $template_funcXint2 = "";
my $template_funcXint3 = "";
my $template_funcXint4 = "";
my $template_funcXcalldoc = "";
my $call_args = "";

print $head;

for (my $i = 1; $i <= $max; $i++) {

  $template_funcXint .=  $sep."class A$i";
  $template_funcXint2 .= $sep."A$i";
  $template_funcXint3 .= $sep."A$i $params[$i]";
  $template_funcXint4 .= " * \@param A$i Type of argument $i.\n";
  $template_funcXcalldoc .=
    "     * \@param $params[$i] Argument passed to signal's emit(). \n";
  $call_args .= $sep.$params[$i];

  print "/* templates for $i argument(s) {{{ */\n\n";

  print "/**\n";
  print " * Interface for callback with $i argument(s).\n";
  print $template_funcXint4;
  print " */\n";
  print "template<$template_funcXint>\n";
  print "class Func${i}Interface {\n";
  print "  public:\n";
  print "    /** array of function interfaces */\n";
  print "    Func${i}Interface<$template_funcXint2>* next;\n";
  print "    /** constructor: init array */\n";
  print "    Func${i}Interface() : next(0) {}\n";
  print "    /**\n";
  print "     * the actual function call.\n";
  print $template_funcXcalldoc;
  print "     * \@return Whether callback succeeded.\n";
  print "     */\n";
  print "    virtual bool call ($template_funcXint3) = 0;\n";
  print "    /** destructor */\n";
  print "    virtual ~Func${i}Interface () {}\n";
  print "    /**\n";
  print "     * get pointer to object the callback is a member of\n";
  print "     * \@c void* is okay since we compare pointers only. Hopefully\n";
  print "     * they're constant during lifetime of an object (otherwise we\n";
  print "     * couldn't call the function anyway.)\n";
  print "     * \@return Pointer.\n";
  print "     */\n";
  print "     virtual void* getObjPtr() = 0;\n";
  print "};\n\n";

  print "/**\n";
  print " * Specialization of Func${i}Interface knowing about the object type the\n";
  print " * callback is a member of. This is required to find the object to call\n";
  print " * a member of.\n";
  print " * \@param T Type of class.\n";
  print $template_funcXint4;
  print " */\n";
  print "template<class T,$template_funcXint>\n";
  print "class Func${i} : public Func${i}Interface<$template_funcXint2> {\n";
  print "  public:\n";
  print "    /** for readability: typedef of function pointer */\n";
  print "    typedef bool (T::*callback)($template_funcXint3);\n";
  print "  private:\n";
  print "    /** object the callback is a member of */\n";
  print "    T* obj;\n";
  print "    /** the callback function */\n";
  print "    callback func;\n";
  print "  public:\n";
  print "    /**\n";
  print "     * Constructor.\n";
  print "     * \@param obj_ Object\n";
  print "     * \@param func_ Callback function.\n";
  print "     */\n";
  print "    Func${i} (T* obj_, callback func_) : obj(obj_),func(func_) {}\n";
  print "    bool call($template_funcXint3) { return ((obj->*func)($call_args)); }\n";
  print "    void* getObjPtr () { return ((void*) obj); }\n";
  print "};\n\n";

  print "/**\n";
  print " * Signal handler for allowing connects with $i argument(s).\n";
  print $template_funcXint4;
  print " */\n";
  print "template <$template_funcXint>\n";
  print "class Signal$i {\n";
  print "  public:\n";
  print "    /** array typedef of callbacks registered */\n";
  print "    typedef Func${i}Interface<$template_funcXint2>* callbacks;\n";
  print "  private:\n";
  print "    /** list of callbacks */\n";
  print "    callbacks first;\n";
  print "    /**\n";
  print "     * for loop prevention: whether we're running.\n";
  print "     * \@test signal_tests::test_signal_loop().\n";
  print "     */\n";
  print "    bool running;\n";
  print "  public:\n";
  print "    /** constructor */\n";
  print "    Signal$i () { first=0; running = false; }\n";
  print "    /** delete a signal */\n";
  print "    ~Signal$i () {\n";
  print "      for (callbacks i = first; i != 0 ; ) {\n";
  print "        callbacks k = i->next;\n";
  print "        delete i;\n";
  print "        i = k;\n";
  print "      }\n";
  print "    }\n";
  print "    /**\n";
  print "     * Register a new callback.\n";
  print "     * \@param item Callback.\n";
  print "     * \@test signal_tests::test_signal_connect().\n";
  print "     */\n";
  print "    void insert (callbacks item) {\n";
  print "      item->next = first;\n";
  print "      first = item;\n";
  print "    }\n";
  print "    /**\n";
  print "     * Remove all bindings for given object from signal.\n";
  print "     * \@param object Object.\n";
  print "     * \@test signal_tests::test_signal_connect().\n";
  print "     */\n";
  print "    template<class T> void removeAll (T* object) {\n";
  print "      callbacks last = first;\n";
  print "      callbacks i = first;\n"; 
  print "      while (i) {\n"; 
  print "        if ((void*) object == i->getObjPtr()) {\n";
  print "          if (first == i) {\n";
  print "            first = i->next;\n";
  print "            delete i;\n";
  print "            i = first;\n"; 
  print "          } else {\n";
  print "            last->next = i->next;\n";
  print "            delete i;\n";
  print "            i = last->next;\n";
  print "          }\n";
  print "        } else {\n";
  print "          last = i;\n";
  print "          i = i->next;\n";
  print "        }\n";
  print "      }\n";
  print "    }\n";
  print "    /**\n";
  print "     * Emit a signal, ie call all connected functions.\n";
  print "     * \@test signal_tests::test_signal_connect().\n";
  print "     * \@test signal_tests::test_signal_loop().\n";
  print "     * \@test signal_tests::test_signal_disconnect().\n";
  print $template_funcXcalldoc;
  print "     * \@return Whether all calls succeeded.\n";
  print "     */\n";
  print "    bool emit ($template_funcXint3) {\n";
  print "      if (running)\n";
  print "        return (false);\n";
  print "      running = true;\n";
  print "      for (callbacks i = first; i; i = i->next)\n";
  print "        if (!i->call ($call_args)) {\n";
  print "          running = false;\n";
  print "          return (false);\n";
  print "        }\n";
  print "      running = false;\n";
  print "      return (true);\n";
  print "    }\n";
  print "};\n\n";

  print "/**\n";
  print " * Connect a function to a signal.\n";
  print " * \@param signal Target signal to connect to.\n";
  print " * \@param object The object the handler is a member of.\n";
  print " * \@param callback The callback function itself.\n";
  print " * \@test signal_tests::test_signal_connect().\n";
  print " */\n";
  print "template<class T,$template_funcXint>\n";
  print "void connectSignal (Signal${i}<$template_funcXint2>& signal,\n";
  print "                    T* object, bool (T::*callback)($template_funcXint3)) {\n";
  print "  signal.insert (new Func${i}<T,$template_funcXint2>(object,callback));\n";
  print "}\n\n";

  print "/**\n";
  print " * Remove all bindings of an object from a signal.\n";
  print " * \@param signal Signal.\n";
  print " * \@param object Object.\n";
  print " * \@test signal_tests::test_signal_disconnect().\n";
  print " */\n";
  print "template<class T,$template_funcXint>\n";
  print "void disconnectSignals (Signal$i<$template_funcXint2>& signal, T* object) {\n";
  print "  signal.removeAll (object);\n";
  print "}\n\n";

  print "/* }}} */\n\n";

  $sep = ",";
}

print $tail;
