/**
@file manual.h
@brief (AUTO) manual
*/
/**
@page page_manual Mutt-ng Manual

    @section sect_introduction Introduction
    @subsection sect_introduction-overview Overview
    <b>Mutt-ng</b> is a small but very
      powerful text-based MIME mail client.  Mutt-ng is highly
      configurable, and is well suited to the mail power user with
      advanced features like key bindings, keyboard macros, mail
      threading, regular expression searches and a powerful pattern
      matching language for selecting groups of messages.
    

    
      This documentation additionally contains documentation to
      <b>Mutt-NG</b>, a fork from Mutt
      with the goal to fix all the little annoyances of Mutt, to
      integrate all the Mutt patches that are floating around in the
      web, and to add other new features. Features specific to Mutt-ng
      will be discussed in an extra section. Don't be confused when
      most of the documentation talk about Mutt and not Mutt-ng,
      Mutt-ng contains all Mutt features, plus many more.
    

    
    @subsection sect_introduction-homepage Mutt-ng Home Page
    
      The official hompepage of mutt-ng is: <a href="http://www.muttng.org/">http://www.muttng.org/</a>

    
      While the rewrite is maintained in a branch, the
      homepage for it is: <a href="http://www.muttng.org/rewrite/">http://www.muttng.org/rewrite/</a>.
    

    
    @subsection sect_introduction-mailinglists Mailing Lists
    
      The following mailing lists are available:
    

    <ul>
    <li><a href="mailto:mutt-ng-users@lists.berlios.de">&lt;mutt-ng-users@lists.berlios.de&gt;</a>: This is
        where the mutt-ng user support happens.</li>
    <li><a href="mailto:mutt-ng-devel@lists.berlios.de">&lt;mutt-ng-devel@lists.berlios.de&gt;</a>: The
        development mailing list for mutt-ng</li>
    
      </ul>
    
    @subsection sect_introduction-distrib-sites Software Distribution Sites
    
      So far, there are no official releases of Mutt-ng, but you can
      download daily snapshots from <a href="http://mutt-ng.berlios.de/snapshots/">http://mutt-ng.berlios.de/snapshots/</a>
      for the ``old'' version.
    

    
      The code of the rewrite branch is currently only available via
      subversion:
    

    <pre>
$ svn checkout http://svn.berlios.de/svnroot/repos/mutt-ng/branches/muttng-rewrite</pre>
    @subsection sect_introduction-irc IRC
    
      Visit channel <tt>#muttng</tt> on <tt>irc.freenode.net</tt>
      (see <a href="http://www.freenode.net/">http://www.freenode.net/</a>) to chat with other people
      interested in Mutt-ng.  
    

    
    @subsection sect_introduction-weblog Weblog
    
      If you want to read fresh news about the latest development in
      Mutt-ng, and get informed about stuff like interesting,
      Mutt-ng-related articles and packages for your favorite
      distribution, you can read and/or subscribe to our development
      weblog at <a href="http://mutt-ng.supersized.org/">http://mutt-ng.supersized.org/</a>.
    

    
    @subsection sect_introduction-copyright Copyright
    
      Mutt is Copyright (C) 1996-2000 Michael R. Elkins
      <a href="mailto:me@cs.hmc.edu">&lt;me@cs.hmc.edu&gt;</a> and others.
    

    
      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.
    

    
      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
    

    
      You should have received a copy of the GNU General Public License
      along with this program; if not, write to the Free Software
      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    

    
    
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
    <li>Optionally, for encrypted connections, either OpenSSL or GNUTLS is
        required.</li>
    
      </ul>
    
      Everything should build if these requirements are met. However, as Perl scripts are
      used to generate some parts of the source code automatically which are already
      under version control, Perl isn't mandatory. On the other hand it can't
      do any harm if it's installed.
    

    
      For building muttng's homepage as well as the manual, the following tools must be
      installed and setup properly:
    

    <ul>
    <li>XSL processor. Currently only <tt>xsltproc</tt> and <tt>xalan</tt> are supported
        whereby <tt>xalan</tt> has problems with XInclude still to be resolved.</li>
    <li>DocBook stylesheets for version 4.3.</li>
    <li>A LaTeX system including <tt>pdflatex(1)</tt>, <tt>latex(1)</tt> and
        <tt>makeindex(1)</tt></li>
    .
    
      </ul>
    
      In addition, for building the source code's documentation,
      <tt>doxygen(1)</tt> (see <a href="http://www.doxygen.org/">http://www.doxygen.org/</a>) is required.
    

    
    @subsection sect_build-config Build Configuration
    
      In order to make the build process work at all, you need to create
      a file called <tt>GNUmakefile.config.mine</tt> in the top-level
      directory, i.e. in <tt>mutt-ng/branches/muttng-rewrite</tt>.
    

    
      There you can set various options. Most important are the following:
    

    <ul>
    <li><tt>CCSTYLE=(gcc|suncc)</tt>. Wether to assume use of GNU or Sun's
        Workshop Pro C/C++ compilers. Default: <tt>gcc</tt>.</li>
    <li><tt>XSLPROC=(xsltproc|xalanj)</tt>. Whether to use <tt>xsltproc(1)</tt>
        or Xalan Java for as XSL processor. Default: <tt>xsltproc</tt>.</li>
    <li><tt>DEBUG=(0|1).</tt> Whether to pass <tt>-g</tt> to the
        compilers/linkers. This is useful for running a debuger. This is not
        the same as the feature to make mutt-ng print debug messages (which
        currently cannot be turned off). Default: <tt>0</tt>.</li>
    <li><tt>UNITPPDIR=/path</tt>. Where Unit++ (see
        <a href="http://unitpp.sf.net/">http://unitpp.sf.net/</a>) for
        running the unit tests is installed. Default: <tt>/usr</tt>.</li>
    <li><tt>OPENSSLDIR=/path</tt>. Where OpenSSL
        is installed. This is required for encrypted connections.
        If none is given, OpenSSL won't be used. Default: none.</li>
    
      </ul>
    
    @subsection sect_build-app Building applications
    
      From the <tt>src</tt> subdirectory, the following are important targets:
    

    <ul>
    <li><tt>depend.</tt> Please always run "make depend" first.</li>
    <li><tt>all.</tt> Build everything.</li>
    <li><tt>test.</tt> Build an run all unit tests.</li>
    <li><tt>srcdoc.</tt> Run <tt>doxygen(1)</tt> to generate the docs from source.
        The output will be placed in the <tt>doxygen</tt> directory. <b>NOTE:</b> you
        need the graphviz package, too. If not, look through the file
        <tt>src/Doxyfile</tt> and there for a line called
        <tt>HAVE_DOT</tt>. Change the <tt>YES</tt> into <tt>NO.</tt></li>
    
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
      <tt>set,</tt><tt>unset,</tt><tt>reset,</tt><tt>toggle</tt> and <tt>query</tt>.
    

    
      The <tt>set</tt> command sets an option to a value. If no particular
      value is given for <tt>quad-option</tt> and <tt>bool</tt> types, <tt>yes</tt> is
      assumed.
    

    
      The <tt>unset</tt> command unsets a value to a neutral value. The
      neutral values depend on the option's type:
    

    <ul>
    <li><tt>boolean</tt> and <tt>quad-option</tt>: <tt>no</tt></li>
    <li><tt>number</tt>; <tt>0</tt></li>
    <li><tt>string</tt> and <tt>url</tt>: the empty string/url ""</li>
    
      </ul>
    
      The <tt>reset</tt> command changes a value to its default.
    

       
      The <tt>toggle</tt> command inverts a <tt>bool</tt> or <tt>quad-option</tt> value
      and is not allowed for other option types.
    

    
      The <tt>query</tt> command displays the current value for any option.
    

    
    @subsection sect_config-types Configuration options
    The following types of configuration options are supported:

    <ul>
    <li><b><tt>boolean</tt></b>: A boolean option. Possible values are:
        <tt>yes</tt>, <tt>no</tt>.</li>
    <li><b><tt>number</tt></b>: A numeric option. Most of the options are
        positive-only numbers but some allow negative values. The
        documentation for the options list these and, in addition,
        mutt-ng will perform strict checks and report error for
        invalid ranges.</li>
    <li><b><tt>string</tt></b>: A string. As for numbers, most options allow any
        value but some allow only for a few magic words given in the
        option documentation. Mutt-ng will perform strict checks and
        print errors for invalid values, i.e. non-magic words.</li>
    <li><b><tt>quad-option</tt></b>: A "question" with the feature to already
        answer it and thus skip any prompts with a given value.
        Possible values are: <tt>yes,</tt><tt>no,</tt><tt>ask-yes</tt> and <tt>ask-no.</tt>
        When an option is used and it's either <tt>ask-yes</tt> and <tt>ask-no,</tt>
        a prompt will query for a result highlighting the given
        choice, <tt>yes</tt> or <tt>no</tt> respectively. With settings of <tt>yes</tt>
        or <tt>no,</tt> the question is assumed to be answered already.</li>
    <li><b><tt>url</tt></b>: A URL for specifying folders. Basically, the
        following syntax is allowed:
        <pre>
proto[s]://[username[:password]@]host[:port]/path</pre>
        where valid protocols are: <tt>imap,</tt><tt>nntp,</tt><tt>pop,</tt><tt>smtp</tt>
        and <tt>file.</tt> The <tt>file</tt> protocol is treated specially as it
        doesn't allow the secure pointer <tt>[s]</tt> and only contains a
        path. For compatibility reasons, if an URL doesn't contain a
        protocoll, <tt>file</tt> is assumed.</li>
    
      </ul>
    
    @subsection sect_config-incompat Incompatible changes
    
      Compared to mutt and mutt-ng, there're some incompatible changes:
    

    <ul>
    <li>The <tt>set ?foo</tt> syntax isn't allowed any longer. For
        this purpose, the <tt>query</tt> command is to be used instead:
        <tt>query foo</tt></li>
    <li>The <tt>set nofoo</tt> syntax isn't allowed any
        longer. For this purpose, use <tt>unset foo</tt> instead.</li>
    <li>The <tt>set invfoo</tt> syntax isn't allowed any
        longer. For this purpose, use <tt>toggle foo</tt> instead.</li>
    <li>The <tt>set &foo</tt> syntax isn't allowed any
        longer. For this purpose, use <tt>reset foo</tt> instead.</li>
    
      </ul>
    
    
    @section sect_reference Reference
    @subsection sect_funcref Functions
    @subsubsection screen_generic Screen: generic
    - <b><tt>&lt;page-up&gt;</tt></b> (default binding: '@c pgup', group: Movement): 
      Page up
    - <b><tt>&lt;show-version&gt;</tt></b> (default binding: '@c V', group: Generic): 
      Show version information
    @subsubsection screen_help Screen: help
    @subsubsection screen_index Screen: index
    - <b><tt>&lt;break-thread&gt;</tt></b> (default binding: '@c #', group: Threading): 
      Break thread at current message
    - <b><tt>&lt;edit-message&gt;</tt></b> (default binding: '@c e', group: Editing): 
      Edit current message
    - <b><tt>&lt;group-reply&gt;</tt></b> (default binding: '@c g', group: Reply/Forward): 
      Group-reply to current message
    - <b><tt>&lt;help&gt;</tt></b> (default binding: '@c h', group: Generic): 
      Show help screen
    - <b><tt>&lt;link-thread&gt;</tt></b> (default binding: '@c &', group: Threading): 
      Link tagged to current message in thread
    - <b><tt>&lt;list-reply&gt;</tt></b> (default binding: '@c L', group: Reply/Forward): 
      List-reply to current message
    - <b><tt>&lt;reply&gt;</tt></b> (default binding: '@c r', group: Reply/Forward): 
      Reply to current message
    @subsubsection screen_pager Screen: pager
    - <b><tt>&lt;edit-message&gt;</tt></b> (default binding: '@c e', group: Editing): 
      Edit current message
    - <b><tt>&lt;group-reply&gt;</tt></b> (default binding: '@c g', group: Reply/Forward): 
      Group-reply to current message
    - <b><tt>&lt;help&gt;</tt></b> (default binding: '@c h', group: Generic): 
      Show help screen
    - <b><tt>&lt;list-reply&gt;</tt></b> (default binding: '@c L', group: Reply/Forward): 
      List-reply to current message
    - <b><tt>&lt;reply&gt;</tt></b> (default binding: '@c r', group: Reply/Forward): 
      Reply to current message
    
    @subsection sect_varref Variables
    @subsubsection option_abort_unmodified $abort_unmodified
      Type: @c quad-option<br>
      Initial value: '@c yes'<br>
      Change signaled: no
    
      If set to <tt>yes</tt>, composition will automatically abort after
      editing the message body if no changes are made to the file (this
      check only happens after the <em>first</em> edit of the file).
      When set to <tt>no</tt>, composition will never be aborted.
    @subsubsection option_allow_8bit $allow_8bit
      Type: @c boolean<br>
      Initial value: '@c yes'<br>
      Change signaled: no
    
      Controls whether 8-bit data is converted to 7-bit using either
      <tt>quoted-printable</tt> or <tt>base64</tt> encoding when
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
      indication are assumed to be in <tt>us-ascii</tt>.
      

    
      For example, Japanese users might prefer this:
      <pre>
      set assumed_charset="iso-2022-jp:euc-jp:shift_jis:utf-8"
      </pre>

    
      However, only the first content is valid for the message body.
      This variable is valid only if @ref option_strict_mime is <tt>unset</tt>.
      

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
      <tt>.[scope].[pid].[id].log</tt>, whereby:
      <ul>
    <li><tt>[scope]</tt> is an identifier for where the output came.
          One file will be created for <tt>libmuttng</tt> and one for the
          tool using it such as <tt>muttng(1)</tt> or <tt>muttgn-conf(1)</tt></li>
    <li><tt>[pid]</tt> is the current process ID</li>
    <li><tt>[id]</tt> is the debug sequence number. For the first debug
          file of the current session it'll be 1 and increased for
          subsequent enabling/disabling of debug output via this variable</li>
    
      </ul>
    

    @subsubsection option_folder $folder
      Type: @c url<br>
      Initial value: '@c file:///tmp/Mail'<br>
      Change signaled: yes
    
      Specifies the default location of your mailboxes.
      

    
      A <tt>+</tt> or <tt>=</tt> at the beginning of a pathname will be expanded
      to the value of this variable.
      

    
      Note that if you change this variable from
      the default value you need to make sure that the assignment occurs
      <em>before</em> you use <tt>+</tt> or <tt>=</tt> for any other variables
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
      command to create a <tt>Bcc:</tt> header field with your email address in it.)
      

    
      The value of @ref option_record is overridden by the @ref option_force_name
      and @ref option_save_name variables, and the @ref command_fcc-hook command.
      

    @subsubsection option_umask $umask
      Type: @c number<br>
      Initial value: '@c 0077'<br>
      Change signaled: no
    
      This variable specifies the <em>octal</em> permissions for
      <tt>umask(2)</tt>. See <tt>chmod(1)</tt> for possible
      value.
      

    
    
    @section sect_devguide Developer's Guide
    @subsection sect_devguide-tour Developer Tour
    
      This section is brief a introduction describing how things are
      organized.
    

    @subsubsection sect_devguide-source-organisation Source code
    
        The source code is organized into the following three layers:
      

    <ul>
    <li><em>core</em> in <tt>src/core/</tt>. This is the base
          library written in C containing very basic functionality. This
          includes a generic hash table, a generic list, a safe
          growing-buffer implementation, lightweight type conversions,
          etc. This is considered legacy and contains all
          implementations which do <em>not</em> deal with e-mail (and
          related topics) or the user interface(s) (and related topics.)
          This is eventually to be separated into an souvereign project,
          maybe. The API already is designed a little towards
          <tt>libowfat</tt> so that a future migratation will be not so
          painful. The migration isn't done as the first step in the
          rewrite process since we're focusing on the core (logic)
          design and not on efficient sanity wrappers for system
          functions.</li>
    <li><em>libmuttng</em> in <tt>src/libmuttng/</tt>. This is the core
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
    <li><em>muttng</em> in <tt>src/muttng/</tt>. Built on top of the
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
        These include DocBook as well as <tt>doxygen(1)</tt>-based
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
          developer manual, is placed in <tt>doc/common/</tt> which contains
          other language-independent parts. For every language supported, upon
          change these will be copied to the language directories to have them
          under version control, too, on the one hand and to ease (the release)
          build process for users.</li>
    
      </ul>
    
    @subsubsection sect_devguide-build-organisation Build system
    
        The build system is completely GNU make-based so that use of the autotools
        (such as <tt>autoconf(1)</tt>) is avoided. To avoid clashes and misleading
        error messages with other flavors of <tt>make(1)</tt>, all Makefiles are named
        <tt>GNUmakefile</tt> so that other make tools don't even find a Makefile.
      

    
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
        least the <tt>@@file</tt> and <tt>@@brief</tt> tags. For
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
        the <tt>src/</tt> directory,
        <tt>libmuttng/foo/bar.h</tt> in this case.
      

    
    @subsubsection sect_hacking-style-misc Misc.
    
        For debugging, make the code print debug info by semantic, not
        just any number.
      

    
    
    @subsection sect_devguide-build Build system
    @subsubsection sect_devguide-build-config Configuration: GNUmakefile.config and GNUmakefile.config.mine
    
        The user configuration Makefile is <tt>GNUmakefile.config.mine</tt>
        in the top-level source directory. It may contain any make logic.
      

    
        Any Makefile including it must include <tt>GNUmakefile.config</tt>
        directly afterwards like in @ref sample-libmuttng-build-config "include
          example" .
      

    @anchor sample-libmuttng-build-config
    @verbinclude makefile_config
    <tt>GNUmakefile.config</tt> interprets and completes internal options
        set in the custom file:
      

    <ul>
    <li>for the compilers, it sets up
          @ref table-make-env1 "variables for compilers and flags" 
          depending on <tt>CCSTYLE</tt></li>
    <li>for optional libraries, it sets up
          <tt>CFLAGS_[lib]</tt>, <tt>CXXFLAGS_[lib]</tt> and
          <tt>LDFLAGS_[lib]</tt> with compiler and linker flags for
          use with library <tt>[lib]</tt>. A
          @ref table-make-env2 "full listing"  of
          libararies is available.</li>
    <li>it checks whether the dependency file <tt>./.depend</tt> exists and
          includes it</li>
    <li>for <tt>DEBUG=1</tt>, it sets up <tt>CFLAGS</tt>, <tt>CXXFLAGS</tt>
          and <tt>LDFLAGS</tt> correctly</li>
    <li>it defines the operating system and sets compiler flags
          accordingly (e.g. <tt>-DFREEBSD</tt> for FreeBSD, <tt>-DSUNOS</tt>
          for solaris, etc.) The operating system value is set as
          <tt>$(MYOS)</tt></li>
    <li>it sets up compiler and linker flags to contain the required
          paths for Unit++ derived from the configured <tt>$(UNITPPDIR)</tt></li>
    <li>it searches <tt>$PATH</tt> for the paths of
          @ref table-make-tools "tools required" .</li>
    <li>according to the XSL processor chosen via <tt>$(XSLPROC)</tt>,
          it defines the following two functions: <tt>doxslt_s</tt> and
          <tt>doxslt_m</tt>. The first does transformation to a single
          output file while the latter does transformation to multiple output files
          (splitting these up is required for Xalan Java whose <tt>-out</tt>
          option cannot be set to a directory.) These functions take the following
          mandatory arguments (ordered):
          <ul>
    <li>XML source file</li>
    <li>XSL stylesheet file</li>
    <li>XML output file</li>
    <li>output language passed to processors as the value for the
              <tt>l10n.gentext.default.language</tt> parameter. This is bad
              modularization as this parameter only has an effect on DocBook
              but passing to other transformations shouldn't do harm.</li>
    
      </ul>
    </li>
    
      </ul>
    @anchor table-make-env1
    @htmlonly
    <p class="title">Common make compile variables</p>
      <table class="ordinary" rowsep="1" summary="Common make compile variables">
    <thead><tr><td>Variable</td><td>Meaning</td></tr></thead><tbody><tr><td><tt>CC</tt></td><td>C compiler</td></tr><tr><td><tt>CXX</tt></td><td>C++ compiler</td></tr><tr><td><tt>CFLAGS</tt></td><td>C compiler flags</td></tr><tr><td><tt>CXXFLAGS</tt></td><td>C++ compiler flags</td></tr><tr><td><tt>DEPFLAGS</tt></td><td>C/C++ compiler flags for dependencies</td></tr><tr><td><tt>AR</tt></td><td>path to <tt>ar(1)</tt></td></tr><tr><td><tt>RANLIB</tt></td><td>path to <tt>ranlib(1)</tt></td></tr></tbody>
      </table>
      @endhtmlonly
    

    @anchor table-make-env2
    @htmlonly
    <p class="title">Library-specific make compile variables</p>
      <table class="ordinary" rowsep="1" summary="Library-specific make compile variables">
    <thead><tr><td>Libarary</td><td>suffix</td><td>Variables</td></tr></thead><tbody><tr><td>OpenSSL</td><td><tt>SSL</tt></td><td><tt>CFLAGS_SSL</tt>, <tt>CXXFLAGS_SSL</tt>, <tt>LDFLAGS_SSL</tt></td></tr><tr><td>GNUTLS</td><td><tt>SSL</tt></td><td><tt>CFLAGS_SSL</tt>, <tt>CXXFLAGS_SSL</tt>, <tt>LDFLAGS_SSL</tt></td></tr></tbody>
      </table>
      @endhtmlonly
    

    @anchor table-make-tools
    @htmlonly
    <p class="title">Tools</p>
      <table class="ordinary" rowsep="1" summary="Tools">
    <thead><tr><td>Tool</td><td>Make variable</td></tr></thead><tbody><tr><td><tt>doxygen(1)</tt></td><td><tt>DOXYGEN</tt></td></tr><tr><td><tt>tidy(1)</tt></td><td><tt>TIDY</tt></td></tr><tr><td><tt>xgettext(1)</tt></td><td><tt>XGETTEXT</tt></td></tr><tr><td><tt>msgmerge(1)</tt></td><td><tt>MSGMERGE</tt></td></tr><tr><td><tt>msgfmt(1)</tt></td><td><tt>MSGFMT</tt></td></tr><tr><td><tt>pdflatex(1)</tt></td><td><tt>PDFLATEX</tt></td></tr><tr><td><tt>latex(1)</tt></td><td><tt>LATEX</tt></td></tr><tr><td><tt>makeindex(1)</tt></td><td><tt>MAKEINDEX</tt></td></tr></tbody>
      </table>
      @endhtmlonly
    

    
        In any Makefile, targets can be setup depending on whether a tool is present
        or not via, see @ref sample-libmuttng-build-conditional "conditional
          example" .
      

    @anchor sample-libmuttng-build-conditional
    @verbinclude makefile_conditional
    
        This only runs <tt>doxygen(1)</tt> it it's found in <tt>$PATH</tt>. If, in addition,
        <tt>tidy(1)</tt> is found too, it'll also be called. If doxygen isn't present,
        the <tt>srcdoc</tt> rule does nothing.
      

    
    @subsubsection sect_devguide-build-whereis Path search: GNUmakefile.whereis
    
        The include Makefile <tt>GNUmakefile.whereis</tt> defines a
        function called <tt>whereis</tt>. This searches through the
        environment variable <tt>$PATH</tt> and returns the full path
        found or the given default fallback.
      

    
        The arguments for <tt>whereis</tt> are in order:
      

    <ul>
    <li>name of binary to find</li>
    <li>fallback</li>
    
      </ul>
    
        For example, the full path to <tt>vi(1)</tt> can be obtained via
        the call shown in the @ref sample-libmuttng-build-whereis-call1 "first whereis call example" .
      

    @anchor sample-libmuttng-build-whereis-call1
    @verbinclude makefile_whereis_call1
    
        If the binary is found, <tt>GREAT_EDITOR</tt> will contain its path and
        will be empty otherwise. A check for it could be implemented as shown in the
        @ref sample-libmuttng-build-whereis-use "whereis usage example" .
      

    @anchor sample-libmuttng-build-whereis-use
    @verbinclude makefile_whereis_use
    
        An example of a non-empty default is show in the
        @ref sample-libmuttng-build-whereis-call2 "second whereis call example" 
        which will search for the <tt>vim(1)</tt> binary: if it's found,
        <tt>GREAT_EDITOR</tt> contains its path but just <tt>vi</tt> otherwise.
      

    @anchor sample-libmuttng-build-whereis-call2
    @verbinclude makefile_whereis_call2
    
    @subsubsection sect_devguide-build-subdirs Directory traversal: GNUmakefile.subdirs
    
        The file <tt>GNUmakefile.subdirs</tt> defines targets to
        be called recursively. To make use of it, define a variable
        named <tt>SUBDIRS</tt> at the beginning of a Makefile. Then,
        the following targets can be used on these directories:
      

    <ul>
    <li><tt>all</tt>: <tt>subdir</tt></li>
    <li><tt>clean</tt>: <tt>subdirclean</tt></li>
    <li><tt>doc</tt>: <tt>subdirdoc</tt></li>
    <li><tt>depend</tt>: <tt>subdirdepend</tt></li>
    <li><tt>test</tt>: <tt>subdirtest</tt></li>
    
      </ul>
    
        For example, to have all these targets for the subdirectories
        <tt>foo</tt> and <tt>bar</tt>, use the basic structure as shown
        in the @ref sample-libmuttng-build-structure "general makefile structure" 
        listing.
      

    @anchor sample-libmuttng-build-structure
    @verbinclude makefile_structure
    
    @subsubsection sect_devguide-build-compile Compilation: GNUmakefile.compile
    
        The files <tt>GNUmakefile.compile_c</tt> and <tt>GNUmakefile.compile_cpp</tt>
        contain everything necessary to compile sources either using a C or C++ compiler
        respectively.
      

    
        Use is as easy as show in the @ref sample-libmuttng-build-compile "compilation makefile example" .
      

    @anchor sample-libmuttng-build-compile
    @verbinclude makefile_compile
    
        This will compile the files
        <tt>foo.c</tt> and <tt>bar.c</tt> into the archive <tt>libfoobar.a</tt> using
        the C compiler.
      

    
        For C++ sources, the files must be named <tt>foo.cpp</tt> and <tt>bar.cpp</tt>
        and instead of <tt>GNUmakefile.compile_c</tt> the file
        <tt>GNUmakefile.compile_cpp</tt> must be included.
      

    
        Either of these also define the following targets:
      

    <ul>
    <li><tt>clean</tt>: remove any temporary, object and archive files</li>
    <li><tt>depend</tt>: run the C/C++ compiler and put the dependency
          tracking info for <tt>make(1)</tt> into <tt>./.depend</tt></li>
    
      </ul>
    
    @subsubsection sect_devguide-build-adding Adding subdirectories
    
        When adding directories somewhere, please make sure the Makefile
        includes at least the files previously shown in the 
        @ref sample-libmuttng-build-config "include example" .
      

    
        Also, it must define the following targets so that they work
        over the complete tree:
      

    <ul>
    <li><tt>all</tt>: perform building all</li>
    <li><tt>clean</tt>: perform cleanup so that only files for a release remain</li>
    <li><tt>test</tt>: perform all automated tests</li>
    <li><tt>depend</tt>: perform all preparation required for
          <tt>all</tt> target</li>
    
      </ul>
    
    
    @subsection sect_devguide-docs Documentation
    @subsubsection sect_devguide-docs-overview Organisation
    
        The documentation includes the user manual, the developer's guide and the
        (new) mutt-ng homepage. All is written in a custom XML dialect (describes later
        in this chapter) and transformed into the following formats:
      

    <ul>
    <li>DocBook: this meta format is used to transform everything into
          fancy HTML</li>
    <li>Doxygen: this format makes doxygen pick up the documentation to
          integrate it into the source's docs</li>
    <li>LaTex: this format is used to generate PDF and PostScript output</li>
    
      </ul>
    
        As far as possible, documentation is automatically generated (better: stripped)
        from the source code to avoid out-of-sync problems. Details of auto-generation
        are described later in this chapter.
      

    
        The documentation is multi-lingual. Details are described later in this
        chapter.
      

    
    @subsubsection sect_devguide-docs-layout Directory layout
    
        The <tt>doc/</tt> directory contains the following subdirectories:
      

    <ul>
    <li><tt>common/</tt>: it contains all language-independent fractions of any
          part of the docs</li>
    <li><tt>css/</tt>: it contains HTML CSS stylesheets for use with doxygen- and
          DocBook-generated HTML output</li>
    <li><tt>xsl/</tt>: it contains the XSL stylesheets used for transformations</li>
    <li><tt>manual_[lang]/</tt>: it contains all input and output files for
          the manual in a specific language</li>
    <li><tt>homepage_[lang]/</tt>: it contains all input and output files for
          the homepage in a specific language</li>
    <li><tt>examples/</tt>: it contains all examples contained in the manual
          (such as programming or configuration examples.)</li>
    <li><tt>tex/</tt>: all files related to LaTeX-processing</li>
    
      </ul>
    
        To have all files correctly setup when getting a working copy or making a
        release, some files are identical and copied from a ``master'' source to
        several directories. This is important to mention as a change made to a copy
        will get lost, changes should be made to the master only. The master documents
        and its copies are:
      

    <ul>
    <li><tt>css/muttng.css</tt>: the main CSS stylesheet it's copied to all
          <tt>manual_*</tt> and all <tt>homepage_*</tt> directories</li>
    <li><tt>common/manual-full.xml</tt> and <tt>common/manual-user.xml</tt>:
          the actual manual files are maintained like this to have a common structure
          for all translations. These are copied to all <tt>manual_*</tt> directories.
        </li>
    
      </ul>
    
        To ensure that at least the programming examples are up-to-date and compile,
        they're not only fragments but complete examples that can be compiled, linked
        and, if needed, even executed.
      

    
    @subsubsection sect_devguide-docs-dialect XML Dialect
    
        This section describes the custom XML dialect used for writing all of
        the documentation (including descriptions for variables and everything
        else.)
      

    @paragraph sect_devguide-docs-dialect-structure Document structure
          The manual is wrapped in a <tt>manual</tt> tag. It contains the following two
          tags:
        

    <ul>
    <li><tt>head</tt>: This describes a header for the document. Its title
            is enclosed in a <tt>title</tt> tag, the list of all authors in an
            <tt>authors</tt> tag. Within the latter, the <tt>author</tt> tag
            lists any number of authors with the following tags:
            <tt>surname</tt>, <tt>firstname</tt> and <tt>email</tt>.</li>
    <li><tt>content</tt>: This describes the content for the document, that
            is, one or more <tt>chapter</tt> tags.</li>
    
      </ul>
    
    @paragraph sect_devguide-docs-dialect-sectioning Chapters and sections
          Within the mentioned <tt>chapter</tt> tags, the following tags are to
          be used for grouping text: <tt>section</tt>, <tt>subsection</tt>
          and <tt>subsubsection</tt>.
          All sectioning tags <em>must</em> have an <tt>id</tt> attribute with
          the name of the section. For nesting them, please specify prefixes to
          avoid clashes. For example, within a chapter with <tt>id="intro"</tt>
          and there for a section mentioning the mailing lists, use
          <tt>id="intro-mailing"</tt>. As all output formats we use have a flat
          ``labeling'' or ``anchor'' namespace, we create our namespaces like
          this.
        

    
    @paragraph sect_devguide-docs-dialect-text Text        
          All texts are to be grouped within <tt>p</tt> tags (``p'' as in
          paragraph.)
        

    
          For ordinary text, please use the following to fill the manual with
          semantics rather than flat text or any layout:
        

    <ul>
    <li><em>special semantics</em>. There will be much more, but currently
            the following tags are to be used for specifying a special semantic
            for a word (mainly these are needed for auto-indexing so that
            one can actually find something in the documentation):
            <ul>
    <li><tt>val</tt>: when refering to (possible/default/...) values
                for variables.</li>
    <li><tt>hdr</tt>: when refering to a commonly used header</li>
    <li><tt>enc</tt>: when refering to transport encodings (such
                as <tt>quoted-printable</tt> or <tt>us-ascii</tt></li>
    <li><tt>env</tt>: when refering to environment variables</li>
    
      </ul>
    </li>
    <li><em>listings</em>. To specify lists or listings, the following tags
            are available:
            <ul>
    <li><tt>ul</tt>: a non-numbered itemized list</li>
    <li><tt>li</tt>: an item of any of these lists</li>
    
      </ul>
    </li>
    <li><em>misc.</em>: The following are not the optimal solution as they
            imply layout semantics already but here we go:
            <ul>
    <li><tt>tt</tt>: typewriter font</li>
    <li><tt>em</tt>: emphasise</li>
    <li><tt>b</tt>: bold font</li>
    
      </ul>
    </li>
    
      </ul>
    
    @paragraph sect_devguide-docs-dialect-referencing Referencing
          For specifying references, a distinction is made to what target
          a reference is made:
        

    <ul>
    <li>@ref tab-ref-tags "some tags"  don't
            accept attributes but the target specified as text</li>
    <li>for document-internal references, the 
            @ref tab-docref-attrs "docref tag is to
              be used" </li>
    <li>for references to manual pages, the @ref tab-man-attrs "man tag"  with an optional
            section attribute is to be used</li>
    
      </ul>
    @anchor tab-ref-tags
    @htmlonly
    <p class="title">reference tags</p>
      <table class="ordinary" rowsep="1" summary="reference tags">
    <thead><tr><td>Tag</td><td>Meaning</td></tr></thead><tbody><tr><td><tt>email</tt></td><td>an email address</td></tr><tr><td><tt>web</tt></td><td>a web address</td></tr><tr><td><tt>varref</tt></td><td>referencing a configuration variable</td></tr><tr><td><tt>cmdref</tt></td><td>referencing a configuration command</td></tr><tr><td><tt>funcref</tt></td><td>referencing a function</td></tr></tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-man-attrs
    @htmlonly
    <p class="title"><tt>man</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>man</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr></thead><tbody><tr><td><tt>sect</tt></td><td><tt>no</tt></td><td>Manual page section, default: 1</td></tr></tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-docref-attrs
    @htmlonly
    <p class="title"><tt>docref</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>docref</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr></thead><tbody><tr><td><tt>type</tt></td><td><tt>yes</tt></td><td>Type of link's end (e.g. ``table''.)</td></tr><tr><td><tt>href</tt></td><td><tt>yes</tt></td><td>Target (document-internal ID)</td></tr></tbody>
      </table>
      @endhtmlonly
    

    
    @paragraph sect_devguide-docs-dialect-variables Documenting variables
          Internally, variables are documented like this (this does <em>not</em> count for
          the description of a variable but the variable as a whole within the
          manual) (<em>note: this is auto-generated</em>):
        

    <ul>
    <li>all variables are wrapped within a <tt>descriptions</tt> tag</li>
    <li>within this, there's any number of <tt>variable</tt> tags containing
            the following attributes: <tt>name</tt> specifies the name of the
            variable, <tt>type</tt> specifies its type</li>
    <li>within a <tt>variable</tt> tag, the following tags will be used:
            <ul>
    <li><tt>init</tt>: initial or default value</li>
    <li><tt>sig</tt>: whether a change of it will cause a signal be emitted</li>
    <li><tt>descr</tt>: its description</li>
    
      </ul>
    </li>
    
      </ul>
    
    @paragraph sect_devguide-docs-dialect-functions Documenting functions
          Internally, functions are documented like this
          (<em>note: this is auto-generated</em>):
        

    <ul>
    <li>all functions are wrapped within a <tt>descriptions</tt> tag</li>
    <li>within this, there's any number of <tt>context</tt> tags containing
            the following attributes: <tt>name</tt> specifies the name of the
            context. As functions are grouped by context (that is: by menu) where
            they may have different bindings, each function may appear several
            times within different <tt>context</tt> tags.</li>
    <li>within the <tt>context</tt> tag, all functions it contains are
            given via <tt>function</tt> tags. Each <tt>function</tt> tag must contain the
            following attributes: <tt>name</tt> specifies the name a user
            may bind to it, <tt>default</tt> specifies the default key binding
            and <tt>group</tt> specifies to which semantic group the function belongs
            within the context as defined in <tt>src/muttng/event/GROUPS</tt>.</li>
    <li>the text within the <tt>function</tt> tag is just the functions's
            summary to appear in the help menus</li>
    
      </ul>
    
    @paragraph sect_devguide-docs-dialect-listings Embedding listrings
          Eventually there will be several types of listings each treated individually
          for the output.
        

    
          The following two tags can be used for specifying listings:
        

    <ul>
    <li><tt>listing</tt>: make listing from an external file</li>
    <li><tt>inlinelisting</tt>: make listing inline from given text</li>
    
      </ul>
    
          Both require these attributes:
        

    <ul>
    <li><tt>lang</tt> specifies the language. For most of the output this is irrelevant
            but for LaTeX output and the <tt>listings.sty</tt> package we have syntax-highlighting
            automatically. The following languages are supported:
            <ul>
    <li><tt>cpp</tt> for C++</li>
    <li><tt>make</tt> for GNU make</li>
    <li><tt>muttngrc</tt> for muttng's configuration syntax</li>
    
      </ul>
    </li>
    
      </ul>
    
          The <tt>listing</tt> tag also requires these attributes in addition:
        

    <ul>
    <li><tt>id</tt> specifies a document-internal ID to link to listings. As for
            any other ID attributes, we fake a namespace or hierarchy by prefixes. At least all
            sample listings must have <tt>sample-</tt> as prefix for this attribute.</li>
    <li><tt>title</tt> is the title to specify (if at all which depends on the output
            format.)</li>
    <li><tt>href</tt> specifies the filename in the <tt>doc/examples/</tt> directory.</li>
    
      </ul>
    
    @paragraph sect_devguide-docs-dialect-tables Tables
          Tables have to be specified within a
          @ref tab-tab-attrs "tab tag with
            proper attributes" . Within a table, there must be a header
          within a <tt>th</tt> tag and a body within a <tt>tb</tt> tag. Within each
          of them, there's any number of rows wrapped in <tt>tr</tt>. As a row consists
          of multiple columns, within <tt>tr</tt> there's any number of <tt>td</tt> tags
          for columns.
        

    
          The caption is specified via a <tt>cap</tt> tag within the table.
        

    @anchor tab-tab-attrs
    @htmlonly
    <p class="title"><tt>tab</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>tab</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr></thead><tbody><tr><td><tt>cols</tt></td><td><tt>yes</tt></td><td>Number of columns</td></tr><tr><td><tt>texstr</tt></td><td><tt>yes</tt></td><td>Layout string for LaTeX's <tt>longtable</tt></td></tr><tr><td><tt>id</tt></td><td><tt>yes</tt></td><td>Document-internal ID</td></tr></tbody>
      </table>
      @endhtmlonly
    

    
    
    @subsubsection sect_devguide-docs-auto Auto-generation
    
        The function and variable descriptions are generated
        to our custom XML dialect automatically.
      

    @paragraph sect_devguide-docs-auto-variables Variables
          For variables, <tt>src/muttng/config/config.pl</tt> is used. The
          documentation process for variables is the same as it was before:
        

    <ul>
    <li>all documentation is expected between the words <tt>START</tt>
            and <tt>END</tt> (commented) in <tt>set_command.cpp</tt></li>
    <li>the variable definition for the source is one line</li>
    <li><em>below</em> is a comment block whereby each line to be
            treated as part of the description for a variable starts with
            <em>two</em> asterisks. The tags are the same as for
            all other documentation.
          </li>
    
      </ul>
    
          The auto-generated files are:
        

    <ul>
    <li><tt>doc/manual_en/var_def.xml</tt>: contains a simple listing
            of all variables found. As English is expected to be always in sync
            with the source code, this file will be used to validate and/or
            check other translations for completeness</li>
    <li><tt>doc/manual_en/var_descr.xml</tt>: contains the full English
            documentation</li>
    
      </ul>
    
    @paragraph sect_devguide-docs-auto-functions Functions
          For functions, <tt>src/muttng/event/event.pl</tt> is used. It parses the
          following file in the <tt>event</tt> subdirectory for documentation:
          <tt>EVENTS</tt>. This has space or tab separated fields with the
          following meanings for documentation:
        

    <ul>
    <li>1st field: the context IDs defined in <tt>CONTEXTS</tt></li>
    <li>3rd field: the function's name</li>
    <li>4th field: the group IDs defined in <tt>GROUPS</tt></li>
    <li>5th field: the default keybinding</li>
    <li>rest of the line: short description</li>
    
      </ul>
    
          The auto-generated files are:
        

    <ul>
    <li><tt>doc/manual_en/func_def.xml</tt>: contains a simple listing
            of all functions found. As English is expected to be always in sync
            with the source code, this file will be used to validate and/or
            check other translations for completeness</li>
    <li><tt>doc/manual_en/func_descr.xml</tt>: contains the full English
            documentation</li>
    
      </ul>
    
    
    @subsubsection sect_devguide-docs-trans Translations
    
        Though the English documents are considered the ``master'' documents,
        they're only a treated as a translation, too. This section describes how to
        add and maintain a translation.
      

    @paragraph sect_devguide-docs-trans-files Mandatory files
          As mentioned when explaining the directory layout, the manual
          files are copied from a master document in the <tt>common/</tt> to a
          language directory. The manual files define the necessary structure which is
          technically implemented using XInclude. Thus, a language directory must
          contain a set of files:
        

    <ul>
    <li><tt>head.xml</tt>: it must contain a <tt>head</tt> tag as the document
            root and list all details for the manual: title and authors.</li>
    <li><tt>trans.xml</tt>: it must contain a set of translated special words described
            later in this document.</li>
    <li><tt>intro.xml</tt>: it must contain the introductionary chapter</li>
    <li><tt>build.xml</tt>: it must contain the chapter explaining the build
            process</li>
    <li><tt>config.xml</tt>: it must contain the configuration documentation</li>
    <li><tt>reference.xml</tt>: it must contain the reference chapter. This document
            must include the following two files: <tt>var_descr.xml</tt> for variable
            descriptions and <tt>func_descr.xml</tt> for functions. For the English
            documentation, both are automatically generated.</li>
    <li><tt>devguide.xml</tt>: it must contain the developer's guide</li>
    <li><tt>ack.xml</tt>: it must contain acknowledgements. This file must include
            the following files in the <tt>doc/common/</tt> directory:
            <ul>
    <li><tt>dev_muttng.xml</tt>: list of mutt-ng developers</li>
    <li><tt>ack_muttng.xml</tt>: list of people who contributed to mutt-ng</li>
    <li><tt>ack_mutt.xml</tt>: list of people who contributed to mutt</li>
    
      </ul>
    </li>
    
      </ul>
    
    @paragraph sect_devguide-docs-trans-add Adding a new translation
          When adding a new translation, the easiest way is to make a copy of the
          English translation: just copy all XML files except <tt>manual-docbook</tt>
          to the corresponding language directory and add them to version control.
        

    
          Afterwards, the following files should be translated first as they contain
          only a few words to translate:
        

    <ul>
    <li><tt>head.xml</tt></li>
    <li><tt>reference.xml</tt></li>
    <li><tt>ack.xml</tt> and</li>
    <li><tt>trans.xml</tt></li>
    
      </ul>
    
          For these, what to translate how should be pretty self-explanatory so that
          not much documentation is put here. In <tt>trans.xml</tt> the comments and
          English translations should be sufficient.
        

    
    
    @subsubsection sect_devguide-docs-notes Notes
    
        For LaTeX output escaping of several characters is required.
        With XSL this could be done using a recursively called function which
        has two major drawbacks: it's terribly slow as the manual is getting
        closer to being complete and it doesn't work based on semantics (for
        example, within a <tt>verbatim</tt> environment, we do not want to
        escape special characters but elsewhere we want.) To solve this,
        the file <tt>doc/tex/muttng.sty</tt> contains a macro named
        <tt>uglyesc</tt> which ``escapes''--or better: fakes escaping--using
        the <tt>listinline</tt> macro of the <tt>listings.sty</tt> package.
        As the <tt>uglyesc</tt> command switches to typewriter, most of the
        typewriter-layout is done with it rather than with LaTeX's own
        <tt>texttt</tt>. As a consequence, whenever using a word which may
        require escaping, use the <tt>tt/</tt> tag.
      

    
    
    @subsection sect_devguide-core Core
    @subsubsection sect_devguide-core-extending Extending the library
    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    @subsection sect_devguide-libmuttng Libmuttng
    @subsubsection sect_devguide-libmuttng-features Features
    
        Libmuttng is in the <tt>src/libmuttng/</tt> subdirectory
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
          <tt>pgp(1)</tt>/<tt>gpg(1)</tt> or via gpgme for crypto and support for
          the mailcap mechanism, using <tt>file(1)</tt> and the like. Decoding
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
      

    @paragraph sect_devguide-libmuttng-signal-declare Declaring
          Declaring a signal is as easy as:
        

    <pre>
SignalX<type of arg1,type of arg2,...,type of argX> signalname;</pre>
          whereby <tt>X</tt> is the number of arguments.
        

    
    @paragraph sect_devguide-libmuttng-signal-connect Connecting
          Connecting to a signal is easy, too. Each handler must fullfill
          the following two requirements:
        

    <ul>
    <li>it must return <tt>bool</tt> specifying whether it succeeded
            or not</li>
    <li>it must take exactly the number and type of arguments the
            signal was declared with</li>
    
      </ul>
    
          Due to overloading, connecting to a signal is always the same:
        

    <pre>
connectSignal (signal, object, Object::handler)</pre>
          ..whereby:
        

    <ul>
    <li><tt>signal</tt> is the signale declared elsewhere</li>
    <li><tt>object</tt> is an instance of an object containg a handler</li>
    <li><tt>Object::handler</tt> is the actual handler method. Here, <tt>Object</tt>
            is the <em>classname</em> of <tt>object</tt></li>
    
      </ul>
    
          In most cases, the object will be <tt>this</tt> though any method may connect
          everything it wants to every signal it wishes to.
        

    
    @paragraph sect_devguide-libmuttng-signal-emit Emitting
          Emitting a signal can be done by every method having access to
          the signal's declaration and works like this:
        

    <pre>
signal.emit (arguments);</pre>
          For a given signal, all handlers are executed in some order while
          each of them returns success. As soon as one handlers reports failure,
          the emit process will abort.
        

    
    @paragraph sect_devguide-libmuttng-signal-disconnect Disonnecting
          Disconnecting from a signal is highly recommended to take
          place in the object's destructor as a crash upon the next emit after
          destruction is likely. Though any object may connect as many handlers as it
          likes to a signal, it's currently only supported to unbind all
          handlers of an object <em>for a specific signal</em> at once like so:
        

    <pre>
disconnectSignals (signal, object);</pre>
          This must be done for every signal the object connected a(ny) handler(s)
          to. In most cases, object is simply <tt>this</tt>.
        

    
    @paragraph sect_devguide-libmuttng-signal-example Example
          The @ref sample-libmuttng-signal "Signal example"  shows how
          to handle signals: it contains a class <tt>Sig</tt> which contains nothing but
          the signal declaration and a class <tt>Handler</tt> which simply catches a signal
          and prints the argument passed through all handlers.
        

    @anchor sample-libmuttng-signal
    @verbinclude libmuttng_signal.cpp
    
    
    @subsubsection sect_devguide-libmuttng-url URL handling
    
        The syntax for supported URLs is:
      

    <pre>
proto[s]://[username[:password]@]host[:port][/path]</pre>
        whereby <tt>proto</tt> is any of the following protocolls:
        <tt>imap[s]</tt>, <tt>pop[s]</tt>, <tt>nntp[s]</tt>, <tt>smtp[s]</tt>
        or <tt>file</tt>.
      

    
        The string <tt>[s]</tt> denotes a secure connection to be used for those protocolls
        supporting it.
      

    
        LibMuttng supports parsing a string into such an URL and <em>always</em>
        fully qualifies the path, i.e. it will always have a leading slash (if no path
        is contained in the string, the path is just the slash.)
      

    
        Please see @ref sample-libmuttng-url "the example provided" 
        for use and error handling.
      

    @anchor sample-libmuttng-url
    @verbinclude libmuttng_url.cpp
    
    @subsubsection sect_devguide-libmuttng-connection Connection handling
    
    @subsubsection sect_devguide-libmuttng-mailbox Mailbox handling
    
        Libmuttng contains transparent support for the following
        types of mailboxes: IMAP, POP3, NNTP, Maildir, MH, MBOX
        and MMDF. Also, caching and filtering (for local mailboxes
        only) is transparently hidden behind the general Mailbox
        class interface.
      

    
        Though any client using libmuttng can access the different
        mailbox types derived from Mailbox directly, it's recommended
        to use the generic interface only.
      

    
        As part of hiding implementations behind the common interface,
        all mailboxes can be accessed via an URL only.
      

    @paragraph sect_devguide-libmuttng-mailbox-create Creating a mailbox
          Creating a new instance of a mailbox based on the URL (other ways
          are not planned), use the Mailbox::fromUrl() function as shown
          @ref sample-libmuttng-mailbox-create "in the example" .
        

    @anchor sample-libmuttng-mailbox-create
    @verbinclude libmuttng_mailbox_create.cpp
    
    
    @subsubsection sect_devguide-libmuttng-auto Auto-generated code
    <em>Signal implementation.</em> As unfortunately we cannot overload
        templates by the number of arguments, we need to specify the same
        implementation for every argument count we need. Thus, this is done by
        a script: <tt>src/libmuttng/signal.pl</tt> prints the commented
        implementation to <tt>stdout</tt> so that the makefile puts it into
        <tt>muttng_signal.h</tt>. When making changes, modify <tt>muttng_signal.h</tt> to verify it
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
    