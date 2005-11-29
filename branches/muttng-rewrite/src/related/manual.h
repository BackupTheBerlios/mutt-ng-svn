/**
@file manual.h
@brief (AUTO) manual
*/
/**
@page page_manual Mutt-ng Manual

    @section sect_introduction Introduction
    
    @section sect_building Building and installation
    @subsection sect_build-systems Supported systems
    
      Mutt-ng is known to work on the following systems:
    

    <ul>
    <li>Linux: gcc</li>
    <li>FreeBSD: 5.x with gcc 3.4/4.0</li>
    <li>Solaris: 5.9 with gcc 3.4/Sun Workshop Pro 9.0</li>
    
      </ul>
    
    @subsection sect_build-requirements Build Requirements
          
      For building mutt-ng, you need to have the following software installed
      and working:
    

    <ul>
    <li>GNU make. Others won't work as the makefiles use GNU make-only constructs.</li>
    <li>A C and C++ compiler. Currently supported are the GNU Compiler Collection and
        Sun Microsystems' workshop pro compiler suite.</li>
    <li>Optionally, for building and running the unit tests, Unit++ (see
        <a href="http://unitpp.sf.net/">http://unitpp.sf.net/</a>) must be installed.</li>
    <li>Optionally, for building localized versions of the applications,
        GNU gettext is required to be installed.</li>
    
      </ul>
    
      Everything should build if these requirements are met. However, as Perl scripts are
      used to generate some parts of the source code automatically which are already
      under version control, Perl isn't mandatory. On the other hand it can't
      do any harm if it's installed.
    

    
      For building muttng's homepage as well as the manual, the following tools must be
      installed and setup properly:
    

    <ul>
    <li>XSL processor. Currently only <code> xsltproc</code> and <code> xalan</code> are supported
        whereby <code> xalan</code> has problems with XInclude still to be resolved.</li>
    <li>DocBook stylesheets for version 4.3.</li>
    
      </ul>
    
      In addition, for building the source code's documentation,
      @c doxygen(1) (see <a href="http://www.doxygen.org/">http://www.doxygen.org/</a>) is required.
    

    
    @subsection sect_build-config Build Configuration
    
      In order to make the build process work at all, you need to create
      a file called <code> GNUmakefile.config.mine</code> in the top-level
      directory, i.e. in <code> mutt-ng/branches/muttng-rewrite</code>.
    

    
      There you can set various options. Most important are the following:
    

    <ul>
    <li><code> CCSTYLE=(gcc|suncc)</code>. Wether to assume use of GNU or Sun's
        Workshop Pro C/C++ compilers. Default: <code> gcc</code>.</li>
    <li><code> XSLPROC=(xsltproc|xalanj)</code>. Whether to use @c xsltproc(1)
        or Xalan Java for as XSL processor. Default: <code> xsltproc</code>.</li>
    <li><code> DEBUG=(0|1).</code> Whether to pass <code> -g</code> to the
        compilers/linkers. This is useful for running a debuger. This is not
        the same as the feature to make mutt-ng print debug messages (which
        currently cannot be turned off). Default: <code> 0</code>.</li>
    <li><code> UNITPPDIR=/path</code>. Where Unit++ (see
        <a href="http://unitpp.sf.net/">http://unitpp.sf.net/</a>) for
        running the unit tests is installed. Default: <code> /usr</code>.</li>
    
      </ul>
    
    @subsection sect_build-app Building applications
    
      From the <code> src</code> subdirectory, the following are important targets:
    

    <ul>
    <li><code> depend.</code> Please always run "make depend" first.</li>
    <li><code> all.</code> Build everything.</li>
    <li><code> test.</code> Build an run all unit tests.</li>
    <li><code> srcdoc.</code> Run @c doxygen(1) to generate the docs from source.
        The output will be placed in the <code> doxygen</code> directory. <b> NOTE:</b> you
        need the graphviz package, too. If not, look through the file
        <code> src/Doxyfile</code> and there for a line called
        <code> HAVE_DOT</code>. Change the <code> YES</code> into <code> NO.</code></li>
    
      </ul>
    
      Building the main applications should be as easy as the following:
    

    <pre>
$ cd ./src && make depend && make</pre>
      If any build process fails, please notify the developers via
      <a href="mailto:mutt-ng-devel@lists.berlios.de">&lt;mutt-ng-devel@lists.berlios.de&gt;</a>.
    

    
    
    @section sect_configuration Configuration
    @subsection sect_config-syntax Syntax of configuration files
    
    @subsection sect_config-commands Configuration commands
    
      The following commands are available for dealing with options:
      <code> set,</code><code> unset,</code><code> reset,</code><code> toggle</code> and <code> query</code>.
    

    
      The <code> set</code> command sets an option to a value. If no particular
      value is given for <code> quad-option</code> and <code> bool</code> types, <code> yes</code> is
      assumed.
    

    
      The <code> unset</code> command unsets a value to a neutral value. The
      neutral values depend on the option's type:
    

    <ul>
    <li><code> boolean</code> and <code> quad-option</code>: <code> no</code></li>
    <li><code> number</code>; <code> 0</code></li>
    <li><code> string</code> and <code> url</code>: the empty string/url ""</li>
    
      </ul>
    
      The <code> reset</code> command changes a value to its default.
    

       
      The <code> toggle</code> command inverts a <code> bool</code> or <code> quad-option</code> value
      and is not allowed for other option types.
    

    
      The <code> query</code> command displays the current value for any option.
    

    
    @subsection sect_config-types Configuration options
    The following types of configuration options are supported:

    <ul>
    <li><b> <code> boolean</code></b>: A boolean option. Possible values are:
        <code> yes</code>, <code> no</code>.</li>
    <li><b> <code> number</code></b>: A numeric option. Most of the options are
        positive-only numbers but some allow negative values. The
        documentation for the options list these and, in addition,
        mutt-ng will perform strict checks and report error for
        invalid ranges.</li>
    <li><b> <code> string</code></b>: A string. As for numbers, most options allow any
        value but some allow only for a few magic words given in the
        option documentation. Mutt-ng will perform strict checks and
        print errors for invalid values, i.e. non-magic words.</li>
    <li><b> <code> quad-option</code></b>: A "question" with the feature to already
        answer it and thus skip any prompts with a given value.
        Possible values are: <code> yes,</code><code> no,</code><code> ask-yes</code> and <code> ask-no.</code>
        When an option is used and it's either <code> ask-yes</code> and <code> ask-no,</code>
        a prompt will query for a result highlighting the given
        choice, <code> yes</code> or <code> no</code> respectively. With settings of <code> yes</code>
        or <code> no,</code> the question is assumed to be answered already.</li>
    <li><b> <code> url</code></b>: A URL for specifying folders. Basically, the
        following syntax is allowed:
        <pre>
proto[s]://[username[:password]@]host[:port]/path</pre>
        where valid protocols are: <code> imap,</code><code> nntp,</code><code> pop,</code><code> smtp</code>
        and <code> file.</code> The <code> file</code> protocol is treated specially as it
        doesn't allow the secure pointer <code> [s]</code> and only contains a
        path. For compatibility reasons, if an URL doesn't contain a
        protocoll, <code> file</code> is assumed.</li>
    
      </ul>
    
    @subsection sect_config-incompat Incompatible changes
    
      Compared to mutt and mutt-ng, there're some incompatible changes:
    

    <ul>
    <li>The set <b> ?</b>foo syntax isn't allowed any longer. For
        this purpose, the <code> query</code> command is to be used instead:
        query foo</li>
    <li>The set <b> no</b>foo syntax isn't allowed any
        longer. For this purpose, use unset foo instead.</li>
    <li>The set <b> inv</b>foo syntax isn't allowed any
        longer. For this purpose, use toggle foo instead.</li>
    <li>The set <b> &</b>foo syntax isn't allowed any
        longer. For this purpose, use reset foo instead.</li>
    
      </ul>
    
    
    @section sect_reference Reference
    @subsection sect_funcref Functions
    @subsubsection screen_generic Screen: generic
    - <b><code>&lt;page-up&gt;</code></b> (default binding: '@c pgup', group: Movement): 
      Page up
    - <b><code>&lt;show-version&gt;</code></b> (default binding: '@c V', group: Generic): 
      Show version information
    @subsubsection screen_help Screen: help
    @subsubsection screen_index Screen: index
    - <b><code>&lt;break-thread&gt;</code></b> (default binding: '@c #', group: Threading): 
      Break thread at current message
    - <b><code>&lt;edit-message&gt;</code></b> (default binding: '@c e', group: Editing): 
      Edit current message
    - <b><code>&lt;group-reply&gt;</code></b> (default binding: '@c g', group: Reply/Forward): 
      Group-reply to current message
    - <b><code>&lt;help&gt;</code></b> (default binding: '@c h', group: Generic): 
      Show help screen
    - <b><code>&lt;link-thread&gt;</code></b> (default binding: '@c &', group: Threading): 
      Link tagged to current message in thread
    - <b><code>&lt;list-reply&gt;</code></b> (default binding: '@c L', group: Reply/Forward): 
      List-reply to current message
    - <b><code>&lt;reply&gt;</code></b> (default binding: '@c r', group: Reply/Forward): 
      Reply to current message
    @subsubsection screen_pager Screen: pager
    - <b><code>&lt;edit-message&gt;</code></b> (default binding: '@c e', group: Editing): 
      Edit current message
    - <b><code>&lt;group-reply&gt;</code></b> (default binding: '@c g', group: Reply/Forward): 
      Group-reply to current message
    - <b><code>&lt;help&gt;</code></b> (default binding: '@c h', group: Generic): 
      Show help screen
    - <b><code>&lt;list-reply&gt;</code></b> (default binding: '@c L', group: Reply/Forward): 
      List-reply to current message
    - <b><code>&lt;reply&gt;</code></b> (default binding: '@c r', group: Reply/Forward): 
      Reply to current message
    
    @subsection sect_varref Variables
    @subsubsection option_abort_unmodified $abort_unmodified
      Type: @c quad-option<br>
      Initial value: '@c yes'<br>
      Change signaled: no
    
      If set to <code> yes</code>, composition will automatically abort after
      editing the message body if no changes are made to the file (this
      check only happens after the <em>first</em> edit of the file).
      When set to <code> no</code>, composition will never be aborted.
    @subsubsection option_allow_8bit $allow_8bit
      Type: @c boolean<br>
      Initial value: '@c yes'<br>
      Change signaled: no
    
      Controls whether 8-bit data is converted to 7-bit using either
      <code> quoted-printable</code> or <code> base64</code> encoding when
      sending mail.
      

    @subsubsection option_assumed_charset $assumed_charset
      Type: @c string<br>
      Initial value: '@c us-ascii'<br>
      Change signaled: no
    
      This variable is a colon-separated list of character encoding
      schemes for messages without character encoding indication.
      Header field values and message body content without character encoding
      indication would be assumed that they are written in one of this list.
      

    
      By default, all the header fields and message body without any charset
      indication are assumed to be in <code> us-ascii</code>.
      

    
      For example, Japanese users might prefer this:
      <pre>
      set assumed_charset="iso-2022-jp:euc-jp:shift_jis:utf-8"
      </pre>

    
      However, only the first content is valid for the message body.
      This variable is valid only if @ref option_strict_mime is <code> unset</code>.
      

    @subsubsection option_debug_level $debug_level
      Type: @c number<br>
      Initial value: '@c 0'<br>
      Change signaled: yes
    
      This variable specifies the current debug level and, currently,
      must be in the range 0 to 5. The value 0 has the special meaning
      that no debug is to be generated. From a value of 1 to 5 the
      amount of debug info written increases drastically.
      

    
      Debug files will be written to the home directory by default and to
      the current if the home directory cannot be determinded.
      

    
      Debug files will have a name of the following format:
      <code> .[scope].[pid].[id].log</code>, whereby:
      <ul>
    <li><code> [scope]</code> is an identifier for where the output came.
          One file will be created for <code> libmuttng</code> and one for the
          tool using it such as @c muttng(1) or @c muttgn-conf(1)</li>
    <li><code> [pid]</code> is the current process ID</li>
    <li><code> [id]</code> is the debug sequence number. For the first debug
          file of the current session it'll be 1 and increased for
          subsequent enabling/disabling of debug output via this variable</li>
    
      </ul>
    

    @subsubsection option_folder $folder
      Type: @c url<br>
      Initial value: '@c file:///tmp/Mail'<br>
      Change signaled: yes
    
      Specifies the default location of your mailboxes.
      

    
      A <code> +</code> or <code> =</code> at the beginning of a pathname will be expanded
      to the value of this variable.
      

    
      Note that if you change this variable from
      the default value you need to make sure that the assignment occurs
      <em>before</em> you use <code> +</code> or <code> =</code> for any other variables
      since expansion takes place during the @ref command_set command.
      

    @subsubsection option_mbox $mbox
      Type: @c url<br>
      Initial value: '@c file:///tmp/mbox'<br>
      Change signaled: no
    
      This specifies the folder into which read mail in your
      @ref option_spoolfile folder will be appended.
      

    @subsubsection option_postponed $postponed
      Type: @c url<br>
      Initial value: '@c file:///tmp/postponed'<br>
      Change signaled: yes
    
      Mutt-ng allows you to indefinitely ``@ref func_postpone sending a message'' which
      you are editing. When you choose to postpone a message, Mutt-ng saves it
      in the mailbox specified by this variable.
      

    
      Also see the @ref option_postpone variable.
      

    @subsubsection option_record $record
      Type: @c url<br>
      Initial value: '@c '<br>
      Change signaled: no
    
      This specifies the file into which your outgoing messages should be
      appended. (This is meant as the primary method for saving a copy of
      your messages, but another way to do this is using the @ref command_my_hdr
      command to create a <code> Bcc:</code> header field with your email address in it.)
      

    
      The value of @ref option_record is overridden by the @ref option_force_name
      and @ref option_save_name variables, and the @ref command_fcc-hook command.
      

    @subsubsection option_umask $umask
      Type: @c number<br>
      Initial value: '@c 0077'<br>
      Change signaled: no
    
      This variable specifies the <em>octal</em> permissions for
      @c umask(2). See @c chmod(1) for possible
      value.
      

    
    
    @section sect_devguide Developer's Guide
    @subsection sect_devguide-tour Developer Tour
    
      This section is brief a introduction describing how things are
      organized.
    

    @subsubsection sect_devguide-source-organisation Source code
    
        The source code is organized into the following three layers:
      

    <ul>
    <li><em>core</em> in <code> src/core/</code>. This is the base
          library written in C containing very basic functionality. This
          includes a generic hash table, a generic list, a safe
          growing-buffer implementation, lightweight type conversions,
          etc. This is considered legacy and contains all
          implementations which do <em>not</em> deal with e-mail (and
          related topics) or the user interface(s) (and related topics.)
          This is eventually to be separated into an souvereign project,
          maybe. The API already is designed a little towards
          <code> libowfat</code> so that a future migratation will be not so
          painful. The migration isn't done as the first step in the
          rewrite process since we're focusing on the core (logic)
          design and not on efficient sanity wrappers for system
          functions.</li>
    <li><em>libmuttng</em> in <code> src/libmuttng/</code>. This is the core
          part of the whole project as it contains most of the mail logic. It
          will provide one simple interface to mail storage independent from
          ``details'' like whether the actual storage is IMAP, POP3, NNTP or
          local storage methods such as Maildir or MBOX, whether there's
          support for a local cache or not, etc. Even the fact that some
          storage or transport methods may use network connections (thus
          involving encryption and authentication features, for example), will
          be be hidden from the application's point of view to achieve maximum
          portability and transparency. Also, it will contain transparent
          MIME-handling including digital encryption and signing, content types
          and handlers, etc. Besides these two major features, it'll provide
          utility classes and functions not at all related to mail but useful
          for the library and application implementation like a centralized
          debug output interface and generic signal handling.</li>
    <li><em>muttng</em> in <code> src/muttng/</code>. Built on top of the
          <em>core</em> and <em>libmuttng</em> layers, this group represents
          user interfaces and everything related. This includes support for
          reading configuration files, presenting data provided by the two
          underlying layers, etc. It will provide a convenient way to write new
          binaries with few code having access to all mail-related tasks. It
          will be split into two major parts: the actual user interface on side
          and all common logic (configuration, mail logic, etc.) on the other
          so that user interfaces only have to implement presenting data and
          nothing more.</li>
    
      </ul>
    
        A very important design goal to keep by any means is the
        relationship between these three layers:
      

    <ul>
    <li><em>core</em> must not depend on or make any kind of
          assumptions about either <em>libmuttng</em> or
          <em>muttng</em>. It may only rely on the standard C
          library.</li>
    <li><em>libmuttng</em> must not depend on or make any kind of
          assumptions about <em>muttng</em> but may utilize
          <em>core</em> and yet not make assumptions about how it
          internally works.</li>
    <li><em>muttng</em> may use any library it finds useful but, as
          all parts, must not make assumptions about internal
          implementation details or choices.</li>
    
      </ul>
    
        The reason for this separation is to have good information
        hiding and a modular structure not only within these parts but
        also between them. As our main intention is to write a
        mail <em>user</em> agent, we move as much mail logic as possible
        out to libmuttng which then will be split into a separate
        project as core will be, too. In the end, mutt-ng is supposed to
        be a flexible client just using powerful libraries without
        implementing much logic itself.
      

    
    @subsubsection sect_devguide-doc-organisation Documentation
    
        All documentation in all places is written in a custom XML
        dialect to be translated via XSL into all other formats we need.
        These include DocBook as well as @c doxygen(1)-based
        documentation. Language-independent parts are put in a separate
        directory and automated mechanisms for validity and completeness
        checks will be provided to ensure correct translated documentation.
      

    
        The basic organization is this:
      

    <ul>
    <li>The ``master'' documentation is English. Where appropriate (see
          below), it'll be auto-generated and validated from the source code to
          avoid out-of-sync problems as far as possible.</li>
    <li>New is the goal to have all parts split over several files rather
          than only one big file. All files have to be valid XML and are checked
          for validity and completenes (where appropriate.)</li>
    <li>The basic manual structure for both, the user-only manual and this
          developer manual, is placed in <code> doc/common/</code> which contains
          other language-independent parts. For every language supported, upon
          change these will be copied to the language directories to have them
          under version control, too, on the one hand and to ease (the release)
          build process for users.</li>
    
      </ul>
    
    @subsubsection sect_devguide-build-organisation Build system
    
        The build system is completely GNU make-based so that use of the autotools
        (such as @c autoconf(1)) is avoided. To avoid clashes and misleading
        error messages with other flavors of @c make(1), all Makefiles are named
        <code> GNUmakefile</code> so that other make tools don't even find a Makefile.
      

    
        In the top-level source directory there're Makefile written for inclusion:
        they contain common logic, targets and rules as well as functions.
      

    
        These include Makefiles have to be included in all other Makefiles as necessary.
      

    
        All Makefiles are under version control except a custom one where users can
        set their own options (and any other make construct.) to avoid subversion conflicts
        if we change something.
      

    
        There's a common set of targets every Makefile in any directory must implement
        so that we have targets working for the whole tree.
      

    
    
    @subsection sect_devguide-style Coding style
    @subsubsection sect_hacking-style-doc Documentation
    
        To keep the code documented and consistent, this section lists
        some rules to keep.
      

    
        In general: document all files! Specify a doxygen header with at
        least the @@file and @@brief tags. For
        headers, the brief tag looks like:
      

    <pre>
@@brief Interface: ...</pre>and

    <pre>
@@brief Implementation: ...</pre>for source files.

    
    @subsubsection sect_hacking-style-hdr Header files
    
        For each source file, there is a header file listing all
        publicly accessable protos for the functions in the source
        file. All functions not listed in the header are to be declared
        static.
      

    
        Each header file is wrapped within the following preprocessor
        magic:
      

    <pre>
#ifndef LIBMUTTNG_FOO_BAR_H
#define LIBMUTTNG_FOO_BAR_H
...
#endif</pre>
        whereby the identifier is constructed from the filename under
        the src/ directory,
        libmuttng/foo/bar.h in this case.
      

    
    @subsubsection sect_hacking-style-misc Misc.
    
        For debugging, make the code print debug info by semantic, not
        just any number.
      

    
    
    @subsection sect_devguide-build Build system
    @subsubsection sect_devguide-build-config Configuration: GNUmakefile.config and GNUmakefile.config.mine
    
        The user configuration Makefile is <code> GNUmakefile.config.mine</code>
        in the top-level source directory. It may contain any make logic.
      

    
        Any Makefile including it must include <code> GNUmakefile.config</code>
        directly afterwards like so:
      

    <pre>
include [path]/GNUmakefile.config.mine
include [path]/GNUmakefile.config</pre><code> GNUmakefile.config</code> interprets and completes internal options
        set in the custom file:
      

    <ul>
    <li>for the compilers, it sets up the following variables depending
          on <code> CCSTYLE</code>:
          <ul>
    <li><code> CC</code>: C compiler</li>
    <li><code> CXX</code>: C++ compiler</li>
    <li><code> CFLAGS</code>: C compiler flags</li>
    <li><code> CXXFLAGS</code>: C++ compiler flags</li>
    <li><code> DEPFLAGS</code>: C/C++ compiler flags to obtain include dependencies
              for make's dependency tracking</li>
    <li><code> AR</code>: path to @c ar(1)</li>
    <li><code> RANLIB</code>: path to @c ranlib(1)</li>
    
      </ul>
    </li>
    <li>it checks whether the dependency file <code> ./.depend</code> exists and
          includes it</li>
    <li>for <code> DEBUG=1</code>, it sets up <code> CFLAGS</code>, <code> CXXFLAGS</code>
          and <code> LDFLAGS</code> correctly</li>
    <li>it defines the operating system and sets compiler flags
          accordingly (e.g. <code> -DFREEBSD</code> for FreeBSD, <code> -DSUNOS</code>
          for solaris, etc.) The operating system value is set as
          <code> $(MYOS)</code></li>
    <li>it sets up compiler and linker flags to contain the required
          paths for Unit++ derived from the configured <code> $(UNITPPDIR)</code></li>
    <li>it searches for the following tools in <code> $PATH</code>:
          <ul>
    <li>@c doxygen(1): <code> $(DOXYGEN)</code></li>
    <li>@c tidy(1): <code> $(TIDY)</code></li>
    <li>@c xgettext(1): <code> $(XGETTEXT)</code></li>
    <li>@c msgmerge(1): <code> $(MSGMERGE)</code></li>
    <li>@c msgfmt(1): <code> $(MSGFMT)</code></li>
    
      </ul>
    </li>
    <li>according to the XSL processor chosen via <code> $(XSLPROC)</code>,
          it defines the following two functions: <code> doxslt_s</code> and
          <code> doxslt_m</code>. The first does transformation to a single
          output file while the latter does transformation to multiple output files
          (splitting these up is required for Xalan Java whose <code> -out</code>
          option cannot be set to a directory.) These functions take the following
          mandatory arguments (ordered):
          <ul>
    <li>XML source file</li>
    <li>XSL stylesheet file</li>
    <li>XML output file</li>
    <li>output language passed to processors as the value for the
              <code> l10n.gentext.default.language</code> parameter. This is bad
              modularization as this parameter only has an effect on DocBook
              but passing to other transformations shouldn't do harm.</li>
    
      </ul>
    </li>
    
      </ul>
    
        In any Makefile, targets can be setup depending on whether a tool is present
        or not via, for example:
      

    <pre>
srcdoc:
ifneq ($(DOXYGEN),)
        $(DOXYGEN)
ifneq ($(TIDY),)
        $(TIDY) [options] [files]
endif
        @ true</pre>
        This only runs @c doxygen(1) it it's found in <code> $PATH</code>. If, in addition,
        @c tidy(1) is found too, it'll also be called. If doxygen isn't present,
        the <code> srcdoc</code> rule does nothing.
      

    
    @subsubsection sect_devguide-build-whereis Path search: GNUmakefile.whereis
    
        The include Makefile <code> GNUmakefile.whereis</code> defines a
        function called <code> whereis</code>. This searches through the
        environment variable <code> $PATH</code> and returns the full path
        found or the given default fallback.
      

    
        The arguments for <code> whereis</code> are in order:
      

    <ul>
    <li>name of binary to find</li>
    <li>fallback</li>
    
      </ul>
    
        For example, the full path to @c vi(1) can be obtained via:
      

    <pre>
GREAT_EDITOR := $(call whereis,vi,)</pre>
        If the binary is found, <code> GREAT_EDITOR</code> will contain its path and
        will be empty otherwise. A check for it could be:
      

    <pre>
somerule:
ifneq ($(GREAT_EDITOR),)
        $(GREAT_EDITOR) [options]
endif</pre>
        An example of a non-empty default is:
      

    <pre>
GREAT_EDITOR := $(call whereis,vim,vi)</pre>
        ...which will search for the @c vim(1) binary: if it's found,
        <code> GREAT_EDITOR</code> contains its path but just <code> vi</code> otherwise.
      

    
    @subsubsection sect_devguide-build-subdirs Directory traversal: GNUmakefile.subdirs
    
        The file <code> GNUmakefile.subdirs</code> defines targets to
        be called recursively. To make use of it, define a variable
        named <code> SUBDIRS</code> at the beginning of a Makefile. Then,
        the following targets can be used on these directories:
      

    <ul>
    <li><code> all</code>: <code> subdir</code></li>
    <li><code> clean</code>: <code> subdirclean</code></li>
    <li><code> doc</code>: <code> subdirdoc</code></li>
    <li><code> depend</code>: <code> subdirdepend</code></li>
    <li><code> test</code>: <code> subdirtest</code></li>
    
      </ul>
    
        For example, to have all these targets for the subdirectories
        <code> foo</code> and <code> bar</code>, use:
      

    <pre>
SUBDIRS=foo bar

all: subdirs
        [commands for local "all" target]
clean: subdirclean
        [commands for local "clean" target]
doc: subdirdoc
        [commands for local "doc" target]
depend: subdirdepend
        [commands for local "depend" target]
test: subdirtest
        [commands for local "test" target]</pre>
          ...whereby the local commands are optional.
        

    
    @subsubsection sect_devguide-build-compile Compilation: GNUmakefile.compile_c and GNUmakefile.compile_cpp
    
        The files <code> GNUmakefile.compile_c</code> and <code> GNUmakefile.compile_cpp</code>
        contain everything necessary to compile sources either using a C or C++ compiler
        respectively.
      

    
        Use is as easy as:
      

    <pre>
FILES := foo bar
LIB := libfoobar.a

all: $(LIB)

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config
include $(CURDIR)/../../GNUmakefile.compile_c</pre>
        This will compile the files
        <code> foo.c</code> and <code> bar.c</code> into the archive <code> libfoobar.a</code> using
        the C compiler.
      

    
        For C++ sources, the files must be named <code> foo.cpp</code> and <code> bar.cpp</code>
        and instead of <code> GNUmakefile.compile_c</code> the file
        <code> GNUmakefile.compile_cpp</code> must be included.
      

    
        Either of these also define the following targets:
      

    <ul>
    <li><code> clean</code>: remove any temporary, object and archive files</li>
    <li><code> depend</code>: run the C/C++ compiler and put the dependency
          tracking info for @c make(1) into <code> ./.depend</code></li>
    
      </ul>
    
    @subsubsection sect_devguide-build-adding Adding subdirectories
    
        When adding directories somewhere, please make sure the Makefile
        includes at least the following:
      

    <pre>
include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config</pre>
        Also, it must define the following targets so that they work
        over the complete tree:
      

    <ul>
    <li><code> all</code>: perform building all</li>
    <li><code> clean</code>: perform cleanup so that only files for a release remain</li>
    <li><code> test</code>: perform all automated tests</li>
    <li><code> depend</code>: perform all preparation required for
          <code> all</code> target</li>
    
      </ul>
    
    
    @subsection sect_devguide-docs Documentation
    @subsubsection sect_devguide-docs-dialect XML Dialect
     This section describes the custom XML dialect used for writing all of
        the documentation (including descriptions for variables and everything
        else.)

    
        The manual is wrapped in a <code> manual</code> tag. It contains the following two
        tags:
      

    <ul>
    <li><code> head</code>: This describes a header for the document. Its title
          is enclosed in a <code> title</code> tag, the list of all authors in an
          <code> authors</code> tag. Within the latter, the <code> author</code> tag
          lists any number of authors with the following tags:
          <code> surname</code>, <code> firstname</code> and <code> email</code>.</li>
    <li><code> content</code>: This describes the content for the document, that
          is, one or more <code> chapter</code> tags.</li>
    
      </ul>
    
        Within the mentioned <code> chapter</code> tags, the following tags are to
        be used for grouping text: <code> section</code> and <code> subsection</code>.
        All sectioning tags <em>must</em> have an <code> id</code> attribute with
        the name of the section. For nesting them, please specify prefixes to
        avoid clashes. For example, within a chapter with <code> id="intro"</code>
        and there for a section mentioning the mailing lists, use
        <code> id="intro-mailing"</code>. As all output formats we use have a flat
        ``labeling'' or ``anchor'' namespace, we create our namespaces like
        this.
      

            
        All texts are to be grouped within <code> p</code> tags (``p'' as in
        paragraph.)
      

    
        For ordinary text, please use the following to fill the manual with
        semantics rather than flat text or any layout:
      

    <ul>
    <li><em>(cross-)referencing</em>. For making any type of references, the followin
          tags are to be used:
          <ul>
    <li><code> email</code>: an email address</li>
    <li><code> web</code>: a web address</li>
    <li><code> varref</code>: referencing a configuration variable</li>
    <li><code> cmdref</code>: referencing a configuration command</li>
    <li><code> funcref</code>: referencing a function</li>
    <li><code> man</code>: referencing a manual page. Optionally,
              it may contain a <code> sect</code> attribute to specify the
              section. If none given, 1 will be used by default.</li>
    
      </ul>
    </li>
    <li><em>special semantics</em>. There will be much more, but currently
          the following tags are to be used for specifying a special semantic
          for a word (mainly these are needed for auto-indexing so that
          one can actually find something in the documentation):
          <ul>
    <li><code> val</code>: when refering to (possible/default/...) values
              for variables.</li>
    <li><code> hdr</code>: when refering to a commonly used header</li>
    <li><code> enc</code>: when refering to transport encodings (such
              as <code> quoted-printable</code> or <code> us-ascii</code></li>
    <li><code> env</code>: when refering to environment variables</li>
    
      </ul>
    </li>
    <li><em>listings</em>. To specify lists or listings, the following tags
          are available:
          <ul>
    <li><code> ul</code>: a non-numbered itemized list</li>
    <li><code> li</code>: an item of any of these lists</li>
    
      </ul>
    </li>
    <li><em>misc.</em>: The following are not the optimal solution as they
          imply layout semantics already but here we go:
          <ul>
    <li><code> tt</code>: typewriter font</li>
    <li><code> em</code>: emphasise</li>
    <li><code> b</code>: bold font</li>
    
      </ul>
    </li>
    
      </ul>
    
        Internally, variables are documented like this (this does <em>not</em> count for
        the description of a variable but the variable as a whole within the
        manual) (<em>note: this is auto-generated</em>):
      

    <ul>
    <li>all variables are wrapped within a <code> descriptions</code> tag</li>
    <li>within this, there's any number of <code> variable</code> tags containing
          the following attributes: <code> name</code> specifies the name of the
          variable, <code> type</code> specifies its type</li>
    <li>within a <code> variable</code> tag, the following tags will be used:
          <ul>
    <li><code> init</code>: initial or default value</li>
    <li><code> sig</code>: whether a change of it will cause a signal be emitted</li>
    <li><code> descr</code>: its description</li>
    
      </ul>
    </li>
    
      </ul>
    
        Internally, functions are documented like this
        (<em>note: this is auto-generated</em>):
      

    <ul>
    <li>all functions are wrapped within a <code> descriptions</code> tag</li>
    <li>within this, there's any number of <code> context</code> tags containing
          the following attributes: <code> name</code> specifies the name of the
          context. As functions are grouped by context (that is: by menu) where
          they may have different bindings, each function may appear several
          times within different <code> context</code> tags.</li>
    <li>within the <code> context</code> tag, all functions it contains are
          given via <code> function</code> tags. Each <code> function</code> tag must contain the
          following attributes: <code> name</code> specifies the name a user
          may bind to it, <code> default</code> specifies the default key binding
          and <code> group</code> specifies to which semantic group the function belongs
          within the context.</li>
    <li>the text within the <code> function</code> tag is just the functions's
          (English) summary to appear in the help menus</li>
    
      </ul>
    
    @subsubsection sect_devguide-docs-auto Auto-generation
    
        The function and variable descriptions are generated
        to our custom XML dialect automatically.
      

    
        For variables, <code> src/muttng/config/config.pl</code> is used. The
        documentation process for variables is the same as it was before:
      

    <ul>
    <li>all documentation is expected between the words <code> START</code>
          and <code> END</code> (commented) in set_command.cpp</li>
    <li>the variable definition for the source is one line</li>
    <li><em>below</em> is a comment block whereby each line to be
          treated as part of the description for a variable starts with
          <em>two</em> asterisks. The tags are the same as for
          all other documentation.
        </li>
    
      </ul>
    
        For functions, <code> src/muttng/event/event.pl</code> is used. It parses the
        following file in the subdirectory for documentation: <code> EVENTS</code>. This
        has space or tab separated fields with the following meanings for documentation:
      

    <ul>
    <li>1st field: the context IDs defined in <code> CONTEXTS</code></li>
    <li>3rd field: the function's name</li>
    <li>4th field: the group IDs defined in <code> GROUPS</code></li>
    <li>5th field: the default keybinding</li>
    <li>rest of the line: short description</li>
    
      </ul>
    
    
    @subsection sect_devguide-core Core
    @subsubsection sect_devguide-core-extending Extending the library
    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    @subsection sect_devguide-libmuttng Libmuttng
    @subsubsection sect_devguide-libmuttng-features Features
    
        Libmuttng is in the <code> src/libmuttng/</code> subdirectory
        and provides the following features:
      

    <ul>
    <li><em>basic services</em> of interest for the whole library and
          application: signal handling and debug support.</li>
    <li><em>mailbox support</em> includes: IMAP, POP3, NNTP, Maildir, MH,
          MBOX and MMDF. The Mailbox abstraction layer works transparently and is
          URL-driven, that is, the client doesn't need to care about and that
          there are different storage formats</li>
    <li><em>caching</em> is transparent for some mailbox types: IMAP, NNTP,
          Maildir and MH. Again, as the Mailbox layer is abstract, so is the
          caching layer which means that clients don't take note of caching
          existing.</li>
    <li><em>MIME and crypto</em> support via either local utilities such as
          @c pgp(1)/@c gpg(1) or via gpgme for crypto and support for
          the mailcap mechanism, using @c file(1) and the like. Decoding
          will be transparent for client, too.</li>
    
      </ul>
    
    @subsubsection sect_devguide-libmuttng-signal Signal handling
    
        Libmuttng contains a very simple, easy to use and typesafe signal
        handling interface. Though signals will be used within the library,
        too, it's use is not limited to connecting handlers in the client
        to events in the library but may also be used to pass signals
        only within the client.
      

    
        We started off with the implementation from:
        <a href="http://lists.trolltech.com/qt-interest/1997-07/msg00158.html">http://lists.trolltech.com/qt-interest/1997-07/msg00158.html</a>
        and extended/modified it to fit our needs.
      

    <em>Declaring</em> a signal is as easy as:
      

    <pre>
SignalX<type of arg1,type of arg2,...,type of argX> signalname;</pre>
        whereby <code> X</code> is the number of arguments. An example may be:
      

    <pre>
Signal1<Mailbox*> mailboxHasNewMail;</pre>
        saying that all handlers will get one argument being a pointer to a
        Mailbox class instance.
      

    <em>Connecting</em> to a signal is easy, too. Each handler must fullfill
        the following two requirements:
      

    <ul>
    <li>it must return <code> bool</code> specifying whether it succeeded
          or not</li>
    <li>it must take exactly the number and type of arguments the
          signal was declared with</li>
    
      </ul>
    
        For the above example, given a class <code> foo</code> with a method
        <code> bool foo::bar(Mailbox* mailbox)</code>, connecting to the
        signal is as easy as:
      

    <pre>
connectSignal (someObject.mailboxHasNewMail, this, &foo::bar);</pre><em>Emitting</em> a signal can be done by every method having access to
        the signal's declaration and works like this:
      

    <pre>
this.mailboxHasNewMail.emit (this);</pre>
        ...assuming the signal is defined in a Mailbox class.
      

    
        For a given signal, all handlers are executed in some order while
        each of them returns success. As soon as one handlers reports failure,
        the emit process will abort.
      

    <em>Disconnecting</em> from a signal is highly recommended to take
        place in the object's destructor as a crash upon the next emit after
        destruction is likely. Though any object may connect as many handlers as it
        likes to a signal, it's currently only supported to unbind all
        handlers of an object <em>for a specific signal</em> at once like so:
      

    <pre>
disconnectSignals (someObject.mailboxHasNewMail, this);</pre>
        This must be done for every signal the object connected a(ny) handler(s)
        to.
      

    
    @subsubsection sect_devguide-libmuttng-auto Auto-generated code
    <em>Signal implementation.</em> As unfortunately we cannot overload
        templates by the number of arguments, we need to specify the same
        implementation for every argument count we need. Thus, this is done by
        a script: <code> src/libmuttng/signal.pl</code> prints the commented
        implementation to <code> stdout</code> so that the makefile puts it into
        <code> muttng_signal.h</code>. When making changes, modify muttng_signal.h to verify it
        works (plus maybe add a unit test) and adjust signal.pl to print the
        changed code.
      

    
    @subsubsection sect_devguide-libmuttng-extending Extending the library
    
        When making extensions to the library and adding classes,
        please make sure to derive them from the Libmuttng base
        class to have debugging support and future extensions
        already in your class.
      

    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    @subsection sect_devguide-muttng Muttng
    @subsubsection sect_devguide-muttng-auto Auto-generated code
    
    @subsubsection sect_devguide-muttng-extending Extending
    
        When making extensions to the library and adding classes,
        please make sure to derive them from the Muttng base
        class to have debugging support and future extensions
        already in your class.
      

    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    
    @section sect_acknowledgements Acknowledgements
    
    Mutt-ng is developed by the following people:
    <ul>
    <li>Andreas Krennmair <a href="mailto:ak@synflood.at">&lt;ak@synflood.at&gt;</a></li>
    <li>Nico Golde <a href="mailto:nico@ngolde.de">&lt;nico@ngolde.de&gt;</a></li>
    <li>Rocco Rutte <a href="mailto:pdmef@cs.tu-berlin.de">&lt;pdmef@cs.tu-berlin.de&gt;</a></li>
    
      </ul>
    

    
    The following people have been very helpful to the
    development of mutt-ng:
    <ul>
    <li>Christian Gall <a href="mailto:cg@cgall.de">&lt;cg@cgall.de&gt;</a></li>
    <li>Iain Lea <a href="mailto:iain@bricbrac.de">&lt;iain@bricbrac.de&gt;</a></li>
    <li>Andreas Kneib <a href="mailto:akneib@gmx.net">&lt;akneib@gmx.net&gt;</a></li>
    <li>Felix Meinhold <a href="mailto:felix.meinhold@gmx.net">&lt;felix.meinhold@gmx.net&gt;</a></li>
    <li>Carsten Schoelzki <a href="mailto:cjs@weisshuhn.de">&lt;cjs@weisshuhn.de&gt;</a></li>
    <li>Elimar Riesebieter <a href="mailto:riesebie@lxtec.de">&lt;riesebie@lxtec.de&gt;</a></li>
    
      </ul>
    

    
    The following people have been very helpful to the
    development of mutt:
    <ul>
    <li>Kari Hurtta <a href="mailto:kari.hurtta@fmi.fi">&lt;kari.hurtta@fmi.fi&gt;</a></li>
    <li>Vikas Agnihotri <a href="mailto:vikasa@writeme.com">&lt;vikasa@writeme.com&gt;</a></li>
    <li>Francois Berjon <a href="mailto: Francois.Berjon@aar.alcatel-alsthom.fr">&lt; Francois.Berjon@aar.alcatel-alsthom.fr&gt;</a></li>
    <li>Aric Blumer <a href="mailto:aric@fore.com">&lt;aric@fore.com&gt;</a></li>
    <li>John Capo <a href="mailto:jc@irbs.com">&lt;jc@irbs.com&gt;</a></li>
    <li>David Champion <a href="mailto:dgc@uchicago.edu">&lt;dgc@uchicago.edu&gt;</a></li>
    <li>Brendan Cully <a href="mailto:brendan@kublai.com">&lt;brendan@kublai.com&gt;</a></li>
    <li>Liviu Daia <a href="mailto:daia@stoilow.imar.ro">&lt;daia@stoilow.imar.ro&gt;</a></li>
    <li>Thomas E. Dickey <a href="mailto:dickey@herndon4.his.com">&lt;dickey@herndon4.his.com&gt;</a></li>
    <li>David DeSimone <a href="mailto:fox@convex.hp.com">&lt;fox@convex.hp.com&gt;</a></li>
    <li>Nickolay N. Dudorov <a href="mailto:nnd@wint.itfs.nsk.su">&lt;nnd@wint.itfs.nsk.su&gt;</a></li>
    <li>Ruslan Ermilov <a href="mailto:ru@freebsd.org">&lt;ru@freebsd.org&gt;</a></li>
    <li>Edmund Grimley Evans <a href="mailto:edmundo@rano.org">&lt;edmundo@rano.org&gt;</a></li>
    <li>Michael Finken <a href="mailto:finken@conware.de">&lt;finken@conware.de&gt;</a></li>
    <li>Sven Guckes <a href="mailto:guckes@math.fu-berlin.de">&lt;guckes@math.fu-berlin.de&gt;</a></li>
    <li>Lars Hecking <a href="mailto:lhecking@nmrc.ie">&lt;lhecking@nmrc.ie&gt;</a></li>
    <li>Mark Holloman <a href="mailto:holloman@nando.net">&lt;holloman@nando.net&gt;</a></li>
    <li>Andreas Holzmann <a href="mailto:holzmann@fmi.uni-passau.de">&lt;holzmann@fmi.uni-passau.de&gt;</a></li>
    <li>Marco d'Itri <a href="mailto:md@linux.it">&lt;md@linux.it&gt;</a></li>
    <li>Björn Jacke <a href="mailto:bjacke@suse.com">&lt;bjacke@suse.com&gt;</a></li>
    <li>Byrial Jensen <a href="mailto:byrial@image.dk">&lt;byrial@image.dk&gt;</a></li>
    <li>David Jeske <a href="mailto:jeske@igcom.net">&lt;jeske@igcom.net&gt;</a></li>
    <li>Christophe Kalt <a href="mailto:kalt@hugo.int-evry.fr">&lt;kalt@hugo.int-evry.fr&gt;</a></li>
    <li>Tommi Komulainen <a href="mailto:Tommi.Komulainen@iki.fi">&lt;Tommi.Komulainen@iki.fi&gt;</a></li>
    <li>Felix von Leitner (a.k.a ``Fefe'') <a href="mailto:leitner@math.fu-berlin.de">&lt;leitner@math.fu-berlin.de&gt;</a></li>
    <li>Brandon Long <a href="mailto:blong@fiction.net">&lt;blong@fiction.net&gt;</a></li>
    <li>Jimmy Mäkeä <a href="mailto:jmy@flashback.net">&lt;jmy@flashback.net&gt;</a></li>
    <li>Lars Marowsky-Bree <a href="mailto:lmb@pointer.in-minden.de">&lt;lmb@pointer.in-minden.de&gt;</a></li>
    <li>Thomas ``Mike'' Michlmayr <a href="mailto:mike@cosy.sbg.ac.at">&lt;mike@cosy.sbg.ac.at&gt;</a></li>
    <li>Andrew W. Nosenko <a href="mailto:awn@bcs.zp.ua">&lt;awn@bcs.zp.ua&gt;</a></li>
    <li>David O'Brien <a href="mailto:obrien@Nuxi.cs.ucdavis.edu">&lt;obrien@Nuxi.cs.ucdavis.edu&gt;</a></li>
    <li>Clint Olsen <a href="mailto:olsenc@ichips.intel.com">&lt;olsenc@ichips.intel.com&gt;</a></li>
    <li>Park Myeong Seok <a href="mailto:pms@romance.kaist.ac.kr">&lt;pms@romance.kaist.ac.kr&gt;</a></li>
    <li>Thomas Parmelan <a href="mailto:tom@ankh.fr.eu.org">&lt;tom@ankh.fr.eu.org&gt;</a></li>
    <li>Ollivier Robert <a href="mailto:roberto@keltia.freenix.fr">&lt;roberto@keltia.freenix.fr&gt;</a></li>
    <li>Thomas Roessler <a href="mailto:roessler@does-not-exist.org">&lt;roessler@does-not-exist.org&gt;</a></li>
    <li>Roland Rosenfeld <a href="mailto:roland@spinnaker.de">&lt;roland@spinnaker.de&gt;</a></li>
    <li>TAKIZAWA Takashi <a href="mailto:taki@luna.email.ne.jp">&lt;taki@luna.email.ne.jp&gt;</a></li>
    <li>Allain Thivillon <a href="mailto:Allain.Thivillon@alma.fr">&lt;Allain.Thivillon@alma.fr&gt;</a></li>
    <li>Gero Treuner <a href="mailto:gero@70t.de">&lt;gero@70t.de&gt;</a></li>
    <li>Vsevolod Volkov <a href="mailto:vvv@lucky.net">&lt;vvv@lucky.net&gt;</a></li>
    <li>Ken Weinert <a href="mailto:kenw@ihs.com">&lt;kenw@ihs.com&gt;</a></li>
    
      </ul>
    

    
    
*/
    