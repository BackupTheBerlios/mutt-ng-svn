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
      matching language for selecting groups of messages to perform
      various of types of actions with.
    

    
      The project started off with the mutt code base and got extended with
      more usefull features written on our own as well as patches
      which were already available for mutt. However, by the time we more and
      more ran into the problem that we found the code (mostly) too complex,
      not well enough documented and hardly maintainable any longer when it came to
      tracking down underlying problems of bug reports we received. As the
      consequence, we started to redesign and rewrite everything from scratch.
    

    
      For the rewrite, the main design objectives are:
    

    <ul>
    <li><em>Logic separation of funtionality.</em> It turned out that there
        were too few abstraction layers in general but special treatment in just too
        many places. Also it seemed that the main design wasn't questioned at all
        but just got extended by more conditional parts in-place instead of
        migrating to better abstraction models. For the rewrite, we thus use the following
        layer model described later in detail: a <em>core</em> part
        containing very low level but not mail-related functionality, a
        <em>libmuttng</em> library with all mail-related logic and a
        <em>muttng</em> part with user interfaces just using the two lower layers
        as needed. Within each, we separate features and functionality as much
        as possible to move to a more modular system.</li>
    <li><em>Documentation.</em> For developers there virtually wasn't any documentation
        about the design decissions made, the general interaction between different
        parts nor which parts there were. The user-manual was good by means of
        quantity but not quality. For the rewrite, we provide much better documentation
        for users and for developers in as many languages as we find translators for.
        The manual will be completely rewritten, that is, better structured, better
        examples and more accessible for users new to mutt-ng. For developers there's
        better documentation, too. Functions are not only documented, within the different
        layers they're also grouped by their purpose including reasoning for
        the grouping structure. The developer's guide contains plenty documentation
        and ready-to-use examples which are included from compileable source code
        (no copy'n'paste is required, examples are verified automatically to compile
        and work.)</li>
    <li><em>Quality ensurance.</em> As the <em>libmuttng</em> as well as the
        <em>muttng</em> layer is written in C++, there're so-called unit tests which
        are run automatically to see if a certain part works. This greatly improves
        maintainability of the code base not only during the rewrite process but also
        when a redesign is required in the future (as this will certainly be the case, we
        already prepare for it.)</li>
    
      </ul>
    
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
    
    @subsection sect_introduction-download Download
    
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
    
      Muttng is Copyright (C) 2005-06 Andreas Krennmair
      <a href="mailto:ak@synflood.at">&lt;ak@synflood.at&gt;</a> and others.
    

    
      Mutt is Copyright (C) 1996-2000 Michael R. Elkins
      <a href="mailto:me@cs.hmc.edu">&lt;me@cs.hmc.edu&gt;</a> and others.
    

    
      Muttng is published as source code with a license
      depending on individual files. The licenses are: GNU General Public
      License, GNU Lesser General Public License and "public domain" (i.e. no
      specific licensing terms attached.)
    

    
      Copies of the GPL and LGPL can be found as files GPL and LGPL in the root
      directory of the source.
    

    
      The header of each file contains its specific licensing terms; a complete list
      can be found at the end of this document.
    

    
    
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
    <li>Perl 5.</li>
    <li>Optionally, for building and running the unit tests, Unit++ (see
        <a href="http://unitpp.sf.net/">http://unitpp.sf.net/</a>) must be installed.</li>
    <li>Optionally, for building localized versions of the applications,
        GNU gettext is required to be installed.</li>
    <li>Optionally, for encrypted connections, either OpenSSL or GNUTLS is
        required.</li>
    <li>Though technically optional it's higly recommended to have
        libiconv installed which is used for conversions between
        different characte sets/encodings.</li>
    <li>For supporting internationalized domain names, optionally libidn
        is required.</li>
    <li>For perl-comatible regular expressions ("PCRE") instead of
        POSIX-compatible, libpcre is required.</li>
    
      </ul>
    
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
    
      In the top-level directory, there's a file named <tt>configure.pl</tt>
      which configures the build process. Just run it with the preferred set
      of options, a full list is available when running it with the 
      <tt>--help</tt> option.
    

    
    @subsection sect_build-app Building applications
    
      Building the main applications should be as easy as the following steps:
    

    <ol>
    <li>run <tt>configure.pl</tt></li>
    <li>If it didn't stop with an error (what shouldn't happen), run
        <pre>
$ make depend</pre>
        in the top-level directory.</li>
    <li>If everything still succeeded, just run <tt>make</tt> to build
        everything.</li>
    
      </ol>
    
      If any of these steps fails, please notify the developers via
      <a href="mailto:mutt-ng-devel@lists.berlios.de">&lt;mutt-ng-devel@lists.berlios.de&gt;</a>.
    

    
    
    @section sect_configuration Configuration
    @subsection sect_config-syntax Syntax of configuration files
    @subsubsection sect_config-syntax-quote Quoting
    
        For flexibility, mutt-ng distincts between two types of quoted
        strings:
      

    <ol>
    <li>single-quoted</li>
    <li>double-quoted</li>
    
      </ol>
    
        The difference between these two is that the latter is expanded
        or completed during reading the configuration files while the
        first one is expanded during use (if at all.) This is the standard
        behavior with many other applications already starting the
        shell. Consider the following example:
      

    @anchor sample-config-quoting
    @verbatim
            
set folder = "$HOME/Maildir/inbox"
set folder = '$HOME/Maildir/inbox'
              @endverbatim
            
        Here, the first line would be expanded during reading it
        to the full path, i.e. the string <tt>$HOME</tt> is replaced
        with its value since the string is enclosed in double quotes.
        The first one will not be expanded so that it contains
        <tt>$HOME/Maildir/inbox</tt> during runtime. A more comprehensive
        listing of such supported expansions is given in the next section.
      

    
        When reading configuration files, exactly one level of
        backslashes is stripped. For example,
      

    @anchor sample-config-backslash
    @verbatim
            
set variable = "\"foo\\\"bar\""
              @endverbatim
            
        will be <tt>"foo\"bar"</tt> at runtime.
      

    
    @subsubsection sect_config-syntax-expand Expansion
    
        The following items are expanded or replace during
        reading configuration files:
      

    <ul>
    <li><tt>$NAME</tt> or <tt>${NAME}</tt>. These are interpreted
          as variables and can be either environment are configuration
          variables while environment variables take precedence. This
          means that if no environment variable <tt>NAME</tt> is found,
          the configuration variable <tt>NAME</tt> is tried. Such a 
          name may consist of letters, digits and underscore. If a
          variable value is to be embedded into a sequence of such
          characters the latter syntax with curly braces may be used
          so mutt-ng can clearly identify what the name is.</li>
    <li>if a backslash is found, several special tokens may
          appear, see @ref table-config-backslash "
            the table"  for a full listing</li>
    <li>if a charet symbol is found, several special tokens
          appear, see @ref table-config-charet "
            the table"  for a full listing</li>
    <li>if a sequence is enclosed in backticks (<tt>`</tt>),
          it's replaced by the output of the command enclosed</li>
    
      </ul>
    @anchor table-config-backslash
    @htmlonly
    <p class="title">Backslash expansion</p>
      <table class="ordinary" rowsep="1" summary="Backslash expansion">
    <thead><tr><td>Sequence</td><td>Replacement</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>cX</tt> or <tt>CX</tt></td><td><tt>CTRL+X</tt></td><td>control character</td></tr>
    <tr><td><tt>r</tt></td><td><tt>r</tt></td><td>carriage return</td></tr>
    <tr><td><tt>n</tt></td><td><tt>n</tt></td><td>newline</td></tr>
    <tr><td><tt>t</tt></td><td><tt>t</tt></td><td>tabulator</td></tr>
    <tr><td><tt>e</tt> or <tt>E</tt></td><td><tt>escape</tt></td><td>escape character</td></tr>
    <tr><td><tt>XYZ</tt></td><td>character</td><td>octal character <tt>XYZ</tt></td></tr>
    <tr><td><tt>xAB</tt></td><td>character</td><td>hex character <tt>0xAB</tt></td></tr>
    </tbody>
      </table>
      @endhtmlonly
    @anchor table-config-charet
    @htmlonly
    <p class="title">Charet expansion</p>
      <table class="ordinary" rowsep="1" summary="Charet expansion">
    <thead><tr><td>Sequence</td><td>Replacement</td><td>Meaning</td></tr>
    </thead><tbody><tr><td>two charets</td><td>charet</td><td>charet character</td></tr>
    <tr><td>bracket</td><td><tt>escape</tt></td><td>escape character</td></tr>
    <tr><td><tt>X</tt></td><td><tt>CTRL+X</tt></td><td>control character</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    
    
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
        protocoll, <tt>file</tt> is assumed. The host part is treated specially
        in some way and may contain domain names as well IP addresses. For
        specifying IPv6 addresses, it has to enclosed in <tt>[]</tt>. For
        example, a secure IMAP connection to IPv6 loopback on port
        <tt>4711</tt> for user <tt>joe</tt> with password <tt>secret</tt> is:
        <pre>
imaps://joe:secret@[::1]:4711/</pre>.
        For implementations such as KAME-derived ones <em>(as found on BSD systems)</em>,
        an interface ID may be specified the usual way by appending <tt>\%ID</tt>. Note that
        as <tt>\%</tt> is used to encode non-ASCII or other special characters in
        URLs, it has to be encoded like so (note the encoded <tt>\%25</tt>):
        <pre>
imaps://joe:secret@[fe80::1%25lo0]:4711/</pre></li>
    <li><b><tt>regular expression</tt></b>. A regular expression.</li>
    <li><b><tt>system</tt></b>. A read-only system variable. These can be only
        queried or used but not set or changed by the user but may change
        during runtime.</li>
    
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
        set their own options (and any other make construct) to avoid subversion conflicts
        if we change something.
      

    
        There's a common set of targets every Makefile in any directory must implement
        so that we have targets working for the whole tree.
      

    
    
    @subsection sect_devguide-principles Principles
    
      This section is more a religious one with some basic principles and
      rules we intent to keep.
    

    @subsubsection sect_devguide-principles-constant Constant values
    
        Unfortunately, there's not <em>the</em> requirement to fulfil in
        order to get most speed and the most leightweight implementation.
        Many different aspects contribute to it of which constant values
        is only one.
      

    
        "Constant values" means that whenever a value is known (i.e. constant)
        at the time of the code writing already, it shouldn't be computed
        at runtime but the information should be made available for use.
      

    
        As this sounds very abstract, a practical example is the core layer's
        string abstraction: as the various <tt>buffer_add_*</tt> functions internally
        ensure that <tt>buffer_t::len</tt> always represents the string's current
        length, use of <tt>strlen(3)</tt> can be avoided. As the buffer
        API allows to specify a length when adding strings, this should be used
        whenever possible to prevent the <tt>buffer_add_str()</tt> function
        from having to compute the length.
      

    
    @subsubsection sect_devguide-principles-memory Memory management
    
        A very important part is memory management which we do more or less
        in a not-so-standard way. We use the core layer's string abstraction
        in many different places.
      

    
        The reason for our approach is that we first of all want to stop using
        constant string buffers by providing expected upper size limits per
        scenario (which may fail.) Another is that we want to have the system perform
        as few memory management operations as possible. The buffer API is
        designed in a way which makes reuse of buffers as simple as possible
        for mainly two reasons:
      

    <ol>
    <li>When working with strings, we don't want to have to deal with
          memory allocation/deallocation in every single function/method we
          write. The ideal case is to have one buffer per purpose and object which
          is initialized at creation and cleaned up at destruction. We hope
          that this way it's easier to avoid and find memory leaks.</li>
    <li>The buffer implementation then only will have to grow the buffer when more
          memory is needed than currently was allocated. This is expected to reduce the
          use of <tt>malloc(3)</tt> family of calls.</li>
    
      </ol>
    
        Another important goal is keep memory handling as clean as possible.
        This practically means that all memory is to be cleaned up after use which
        includes leaving no memory reachable when an application exits. This not only
        removes many warnings issued by <tt>valgrind(1)</tt> but also helps to
        keep the code clean (or enables us at least to check for it.)
      

    
    
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
    @subsubsection sect_devguide-build-systemconfig System Configuration
    
        As said in the introductory part of this manual, the use of
        tools such as <tt>autoconf(1)</tt> or <tt>automake(1)</tt> is
        avoided by use of a custom system which includes the detection
        of system properties, too.
      

    
        The system basically works like this: there's a configuration script
        written in Perl named <tt>configure.pl</tt> which sets up a configuration
        Makefile <tt>GNUmakefile.config.mine</tt>, runs some more difficult tests
        to obtain system-dependent settings and writes out header files with the
        configuration.
      

    
        As the whole project is work-in-progress, there's not much auto-detection
        implemented in <tt>configure.pl</tt> so everything has to be specified.
      

    
        Every Makefile must include <tt>GNUmakefile.config</tt>
        directly afterwards like in @ref sample-libmuttng-build-config "include example" .
      

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
    <thead><tr><td>Variable</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>CC</tt></td><td>C compiler</td></tr>
    <tr><td><tt>CXX</tt></td><td>C++ compiler</td></tr>
    <tr><td><tt>CFLAGS</tt></td><td>C compiler flags</td></tr>
    <tr><td><tt>CXXFLAGS</tt></td><td>C++ compiler flags</td></tr>
    <tr><td><tt>DEPFLAGS</tt></td><td>C/C++ compiler flags for dependencies</td></tr>
    <tr><td><tt>AR</tt></td><td>path to <tt>ar(1)</tt></td></tr>
    <tr><td><tt>RANLIB</tt></td><td>path to <tt>ranlib(1)</tt></td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor table-make-env2
    @htmlonly
    <p class="title">Library-specific make compile variables</p>
      <table class="ordinary" rowsep="1" summary="Library-specific make compile variables">
    <thead><tr><td>Libarary</td><td>suffix</td><td>Variables</td></tr>
    </thead><tbody><tr><td>OpenSSL</td><td><tt>SSL</tt></td><td><tt>CFLAGS_SSL</tt>, <tt>CXXFLAGS_SSL</tt>, <tt>LDFLAGS_SSL</tt></td></tr>
    <tr><td>GNUTLS</td><td><tt>SSL</tt></td><td><tt>CFLAGS_SSL</tt>, <tt>CXXFLAGS_SSL</tt>, <tt>LDFLAGS_SSL</tt></td></tr>
    <tr><td>LibIconv</td><td><tt>LIBICONV</tt></td><td><tt>CFLAGS_LIBICONV</tt>, <tt>CXXFLAGS_LIBICONV</tt>, <tt>LDFLAGS_LIBICONV</tt></td></tr>
    <tr><td>Unit++</td><td><tt>UNITPP</tt></td><td><tt>CFLAGS_UNITPP</tt>, <tt>CXXFLAGS_UNITPP</tt>, <tt>LDFLAGS_UNITPP</tt></td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor table-make-tools
    @htmlonly
    <p class="title">Tools</p>
      <table class="ordinary" rowsep="1" summary="Tools">
    <thead><tr><td>Tool</td><td>Make variable</td></tr>
    </thead><tbody><tr><td><tt>doxygen(1)</tt></td><td><tt>DOXYGEN</tt></td></tr>
    <tr><td><tt>tidy(1)</tt></td><td><tt>TIDY</tt></td></tr>
    <tr><td><tt>xgettext(1)</tt></td><td><tt>XGETTEXT</tt></td></tr>
    <tr><td><tt>msgmerge(1)</tt></td><td><tt>MSGMERGE</tt></td></tr>
    <tr><td><tt>msgfmt(1)</tt></td><td><tt>MSGFMT</tt></td></tr>
    <tr><td><tt>pdflatex(1)</tt></td><td><tt>PDFLATEX</tt></td></tr>
    <tr><td><tt>latex(1)</tt></td><td><tt>LATEX</tt></td></tr>
    <tr><td><tt>makeindex(1)</tt></td><td><tt>MAKEINDEX</tt></td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    
        In every Makefile, targets can be setup depending on whether a tool is present
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
    <li><tt>ol</tt>: numbered itemized list</li>
    <li><tt>li</tt>: an item of any of these lists</li>
    
      </ul>
    </li>
    <li><em>misc.</em>: The following are not the optimal solution as they
            imply layout semantics already but here we go:
            <ul>
    <li><tt>tt</tt>: typewriter font</li>
    <li><tt>em</tt>: emphasise</li>
    <li><tt>b</tt>: bold font</li>
    <li><tt>footnote</tt>: a footnote</li>
    
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
    <thead><tr><td>Tag</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>email</tt></td><td>an email address</td></tr>
    <tr><td><tt>web</tt></td><td>a web address</td></tr>
    <tr><td><tt>varref</tt></td><td>referencing a configuration variable</td></tr>
    <tr><td><tt>cmdref</tt></td><td>referencing a configuration command</td></tr>
    <tr><td><tt>funcref</tt></td><td>referencing a function</td></tr>
    <tr><td><tt>rfc</tt></td><td>an RfC number</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-man-attrs
    @htmlonly
    <p class="title"><tt>man</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>man</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>sect</tt></td><td><tt>no</tt></td><td>Manual page section, default: 1</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-docref-attrs
    @htmlonly
    <p class="title"><tt>docref</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>docref</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>type</tt></td><td><tt>yes</tt></td><td>Type of link's end (e.g. ``table''.)</td></tr>
    <tr><td><tt>href</tt></td><td><tt>yes</tt></td><td>Target (document-internal ID)</td></tr>
    </tbody>
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
          There're the following two tags for specifying listings:
        

    <ol>
    <li>@ref tab-listing-attrs "listing" :
            if given a <tt>href</tt> attribute, it requests the source be found
            in the external file and given as the tag's text otherwise. For
            most output formats, it puts the code in a dedicated paragraph.</li>
    <li>@ref tab-inlinelisting-attrs "inlinelisting" :
            contrary to the <tt>listing</tt> tag it always expects the code
            as the tag's text and doesn't put the code in a new paragraph but
            inlines it into the current.</li>
    
      </ol>
    
          In order to enable syntax-highlighting or auto-generation of references,
          each listing must be declared with a language of which the code is of.
          See @ref tab-listing-languages "the given table" 
          with all possible values for the <tt>lang</tt> attribute.
        

    @anchor tab-listing-languages
    @htmlonly
    <p class="title"><tt>inlinelisting</tt>/<tt>listing</tt> languages</p>
      <table class="ordinary" rowsep="1" summary="<tt>inlinelisting</tt>/<tt>listing</tt> languages">
    <thead><tr><td><tt>lang</tt> attribute value</td><td>Language</td></tr>
    </thead><tbody><tr><td><tt>c</tt></td><td>C code</td></tr>
    <tr><td><tt>cpp</tt></td><td>C++ code</td></tr>
    <tr><td><tt>make</tt></td><td>GNU <tt>make(1)</tt> code</td></tr>
    <tr><td><tt>muttngrc</tt></td><td>mutt-ng configuration</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-inlinelisting-attrs
    @htmlonly
    <p class="title"><tt>inlinelisting</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>inlinelisting</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>lang</tt></td><td><tt>yes</tt></td><td>listing's language</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @anchor tab-listing-attrs
    @htmlonly
    <p class="title"><tt>listing</tt> tag attributes</p>
      <table class="ordinary" rowsep="1" summary="<tt>listing</tt> tag attributes">
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>id</tt></td><td><tt>yes</tt></td><td>document-internal ID</td></tr>
    <tr><td><tt>title</tt></td><td><tt>yes</tt></td><td>caption string</td></tr>
    <tr><td><tt>lang</tt></td><td><tt>yes</tt></td><td>listing's language</td></tr>
    <tr><td><tt>href</tt></td><td><tt>no</tt></td><td>external source file (if any)</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    
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
    <thead><tr><td>Attribute</td><td>Mandatory</td><td>Meaning</td></tr>
    </thead><tbody><tr><td><tt>cols</tt></td><td><tt>yes</tt></td><td>Number of columns</td></tr>
    <tr><td><tt>texstr</tt></td><td><tt>yes</tt></td><td>Layout string for LaTeX's <tt>longtable</tt></td></tr>
    <tr><td><tt>id</tt></td><td><tt>yes</tt></td><td>Document-internal ID</td></tr>
    </tbody>
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
    @subsubsection sect_devguide-core-features Features
    
        Core is in the <tt>src/core/</tt> subdirectory
        and aims to be a library with easily reusable and lightweight
        C routines for any application. Maybe this is eventually
        either split into a separate project or we migrate to another
        library providing similar features. Or we just leave it as it
        is at the moment.
      

    
        The API is designed in a way similar to libowfat
        (see <a href="http://www.fefe.de/libowfat/">http://www.fefe.de/libowfat/</a>) by
        Felix von Leitner so a possible migration to it will be easier
        if it happens at all.
      

    
        The <em>core</em> library contains the following features:
      

    <ul>
    <li><em>String handling.</em> As the upper layers of mutt-ng are
          C++ and we consider C++ strings to be too heavyweighted but
          C strings on the other hand too low level, it contains a comprise:
          a string abstraction with enough leightweight routines to
          get rid of <tt>snprintf(3)</tt> for composing strings.</li>
    <li><em>Data conversion.</em>. It contains features for converting
          data such as <tt>iconv(3)</tt>-based conversions,
          <tt>quoted-printable</tt> and <tt>base64</tt> support and
          the like.</li>
    <li><em>Wrappers.</em> For historic reasons, it contains many
          functions just wrapping functions which the standard C library
          should provide or which are provided but require too many
          individual special treatment (including error checking.)
          This includes safety wrappers for
          memory handling (checking the results returned by the system)
          as well as <tt>NULL</tt>-aware versions of the standard string
          functions (which isn't the case for all systems.)</li>
    <li><em>I/O.</em> It contains some utility functions to ease
          and secure access to (temporary) files.</li>
    <li><em>Data structures</em>. It contains a relatively leightweight
          array-based generic list implementation upon which a generic
          hash table is built.</li>
    
      </ul>
    
    @subsubsection sect_devguide-core-string String handling
    @paragraph sect_devguide-core-string-intro Introduction
          The string handling is built around a simple structure called
          <tt>buffer_t</tt> with lots of different functions for working
          with it: adding strings/buffers/numbers, comparisons, manipulation
          such as chomping, etc.
        

    
          The reason for implementing it is quite simple: the standard way
          of composing a destination string seems to be using
          <tt>snprintf(3)</tt>. But this approach has several drawbacks
          our string abstraction avoids:
        

    <ul>
    <li>Programmers are responsible on their own for memory management
            for at least the destination for <tt>snprintf(3)</tt> which
            in the past lead to defining a series of scenarios with upper limits
            of expected input. Though most of these were allocated on the stack rather
            than via <tt>malloc(3)</tt> and <tt>free(3)</tt>,
            it's bad to make assumptions about the amount of input on
            the one hand and on the other it's hard to track down bugs caused by
            such a failure in assumption. Our string abstraction automatically
            grows the buffer as needed <em>(using <tt>realloc(3)</tt>
              with a large enough default/increase size so that
              <tt>_POSIX_PATH_MAX</tt> fits in)</em>
            so we need not make any assumptions. Also, the actual code is
            unit tested and quite short so that several independent security
            checks should be possible.</li>
    <li>As profiling shows (see, for example,
            <a href="http://user.cs.tu-berlin.de/~pdmef/graphviz/#printf_vs_write">http://user.cs.tu-berlin.de/~pdmef/graphviz/#printf_vs_write</a>),
            the <tt>printf(3)</tt> family of functions may internally
            use <tt>malloc(3)</tt> which eliminates the positive speed
            effect of allocating storage on the stack temporarily.</li>
    <li>More of a philosophical nature is the fact that we consider it to
            be superflous to "encode" formatting information for
            <tt>printf(3)</tt>. Specifying a format string is a type
            of encoding the format of the requested result. <tt>printf(3)</tt>
            needs to decode it to compose the result. However, exactly this
            encoding and decoding is superflous as the format already is always
            constant during runtime. It only changes when the actual source
            changes. Even when composing the format string dynamically, right
            before the call the formatting information is constant.
          </li>
    
      </ul>
    
          Our approach has the following disadvantages:
        

    <ul>
    <li>it requires much more typing and thus leads to much more code</li>
    
      </ul>
    
    @paragraph sect_devguide-core-string-format Formatting output
          Basically, one needs to first declare a buffer as shown
          in the @ref sample-buffer-declare "example" . The
          <tt>buffer_init()</tt> function sets everything to zero so that no
          accidents happen.
        

    @anchor sample-buffer-declare
    @code
            
buffer_t buffer;
buffer_init(&buffer);
              @endcode
            
          See the file <tt>core/buffer.h</tt> for the functions.
        

    
          In @ref sample-buffer-format "the short example" 
          there's a small sample program given on how to format output: it simply prints out the number
          of arguments the program is called with and their values. A different version
          writing the output more often but copying less is
          @ref sample-buffer-format2 "shown in another example" .
        

    
          These examples show...
        

    <ul>
    <li>that there's no need to use <tt>printf(3)</tt> required to
            print formatted output.</li>
    <li>how to format numbers of different bases with and without
            zero padding using <tt>buffer_add_num()</tt> and
            <tt>buffer_add_num2()</tt></li>
    <li>why how and how to use the <tt>buffer_shrink()</tt> function:
            it helps avoiding copying the same content multiple times in
            situation where it's sufficient to do it only once</li>
    <li>that the buffer functions keep track of length and size information
            so that there's no need to compute these when needed</li>
    
      </ul>
    @anchor sample-buffer-format
    @include core_buffer_format.c
            @anchor sample-buffer-format2
    @include core_buffer_format2.c
            
    @paragraph sect_devguide-core-string-recode Encoding and decoding
          Currently, buffers can be encoded and decoded from and to the following formats:
        

    <ul>
    <li><tt>base64</tt></li>
    <li><tt>quoted-printable</tt></li>
    
      </ul>
    
          These not need much documentation, please see the doxygen part for usage details.
        

    
    @paragraph sect_devguide-core-string-charset Character Sets
          The string handling part of the core layer contains a very simple but suffiently
          convenient interface for convertion a buffer from any to any character set
          while being aware of many officially defined character set aliases.
        

    
          The implementation in <tt>conv.c</tt> is basically taken from the old mutt code base
          and contains wrappers around <tt>libiconv</tt> converting everything by force.
        

    
          The most important function is <tt>conv_iconv()</tt> converting a buffer in-place.
        

    
          Besides it, the following functions are to be used for handling character
          sets:
        

    <ul>
    <li><tt>conv_charset_normal()</tt>: in case a passed in character set is
            known to be an alias for another, the contents is replaced with the
            lowercase non-alias name</li>
    <li><tt>conv_charset_eq()</tt>: normalizes two given character sets and
            checks for equality</li>
    <li><tt>conv_charset_base64()</tt>: certain RfCs were written defining
            preferred character sets and transport encoding for various languages. To
            not have special treatment and checks all over the place, this is the main
            function to test whether <tt>base64</tt> encoding should be used. Otherwise
            the caller is free to choose.</li>
    <li><tt>conv_charset_list()</tt>: passes as many known character sets or
            aliases to a callback as the callback accepts them</li>
    
      </ul>
    
          The <tt>conv_iconv_version()</tt> appends the version of <tt>libiconv</tt> used
          to a destination buffer.
        

    
          The <tt>conv_init()</tt> and <tt>conv_cleanup()</tt> functions are not intended
          for use by clients; clients should use <tt>core_init()</tt> and <tt>core_cleanup()</tt>
          instead.
        

    
    @paragraph sect_devguide-core-string-token Tokenizer
    
    @subsubsection sect_devguide-core-list Generic list
    
    @subsubsection sect_devguide-core-hashing Generic hash table
    
        The core layer contains a generic hash table. "Generic" means that
        all kinds of data can be stored. This is achieved by making the
        storage type <tt>unsigned long</tt> so that simple types like
        <tt>int</tt> or <tt>char</tt> can be stored as well as all types
        of data referenced to by pointers which fit into <tt>unsigned long</tt>.
      

    
        This approach has the advantage that one hash table can be used
        for every type of data but the disadvantage that it makes C code
        harder to read (due to type casts) and requires some care (again:
        due to type casts.)
      

    @paragraph sect_devguide-core-hashing-create Table creation
          The function <tt>hash_new()</tt> function is to be used for
          creating a new table. The <tt>size</tt> argument should be
          twice as large as the expected maximum number of elements stored
          in the table to ensure low access times due to few collissions.
        

    
          It depends on the caller whether keys are stable during lifetime
          or not. If so, the <tt>dup_key</tt> can be set to <tt>0</tt> to
          store pointers to the keys only. If not, keys are copied when
          it's set to <tt>1</tt>. This cannot be changed during runtime.
        

    
          If one is not sure, the safe way is to copy keys which requires
          more memory and memory operations such as <tt>malloc(3)</tt>
          and <tt>free(3)</tt>. If keys change during runtime
          and the hash table didn't copy them, the behavior is completely
          undefined and strange effects may appear.
        

    
    @paragraph sect_devguide-core-hashing-destroy Table destruction
          For destruction, the <tt>hash_destroy</tt> function is to be
          used.
        

    
          It accepts and optional callback to free up memory consumed by
          entries it stored.
        

    
          This is optional since the table cannot know whether the entries
          are stored just pointers into original data or pointers to
          copies of the original data (in fact, it doesn't even know that
          it stores pointers.) For the latter case, upon destruction of
          a table, all the extra memory allocated must be released, of course.
        

    
    @paragraph sect_devguide-core-hashing-work Insertion, removal and lookup
          The following scenarios are supported when working with a table:
        

    <ul>
    <li>adding items</li>
    <li>removing items</li>
    <li>finding data associated with an item</li>
    <li>testing for existence</li>
    
      </ul>
    
          The last two points cannot be merged as they only have the same
          semantics in some cases but not all. For example, when a table is
          used to map keys to real data, these are equal: if not associated
          data is found when performing a lookup, the item doesn't exist (thus, the
          existence test is superflous.) But when a table is used to just
          manage keys without data (to see whether a property is defined or
          not, for example), there's no associated data so that the find
          operation would always fail. In such a case, the existence feature
          is to be used instead.
        

    
          All of these functions are available in two flavors: the real one
          expects a table, a key and a hash code for the key. The second one
          is just defined as a macro for the first one.
        

    
          The reason is that when the programmer knows he is going to perform
          a series of operations with the same key, it would waste resources to
          compute the hash code over and over for every single operation. Thus,
          the programmer should compute the key's hash code once and use the
          functions with the <tt>_hash</tt> suffix to avoid computation.
        

    
          Though the hash code computation is quite cheap and in the majority
          of cases it wouldn't mean a noticeable slowdown of operation, it
          doesn't make sense to compute constant and known data all the time.
        

    
    @paragraph sect_devguide-core-hashing-map Mapping functions to a table
          Besides the mention functions supported, there's one more powerful
          function called <tt>hash_map()</tt>.
        

    
          As the table's internal organization isn't promoted to the outside (i.e.
          a table is defined as <tt>void*</tt>) to achieve de-coupling and information
          hiding, the map function can be used to work with a table besided adding,
          removing or searching for items (though some of these may be done with 
          the map function, too.)
        

    
          The map function takes a pointer to a callback function as an
          argument and calls it for every single key-value pair the table
          contains in a undefined order if no sorting is requested.
          This doesn't sound spectacular but
          enables a client to count the number of pairs stored, compose a list
          of keys and/or values including custom selection patterns, etc.
        

    
          During runtime of the <tt>hash_map()</tt> function, the table will
          be locked (see next section.)
        

    
    @paragraph sect_devguide-core-hashing-locking Locking
          A table can be locked: while it's in that state, no insertions or
          removals will succeed. As a consistent and constant state is important
          for the <tt>hash_map()</tt> function, a table is locked during its run.
        

    
    @paragraph sect_devguide-core-hashing-example Example
          As uses of hashing may appear in lots of different fashions, it's
          difficult to provide a complete and useful example. The
          @ref sample-core-hashing "simple hashing example" 
          creates a hash table with the arguments it was called with. The actual command-line
          arguments serves as keys while their index number serves as value.
          Also it shows how to use the <tt>hash_map()</tt> function: in the
          example it's used for printing the items on <tt>stdout</tt>.
        

    @anchor sample-core-hashing
    @include core_hash.c
            
          What the example doesn't show is that <tt>hash_map()</tt> only
          executes the callback while it's told to. That means that the
          callback can make the mapping abort at any point it wishes to
          (because, for example, it already found what it was looking for.)
          A @ref sample-libmuttng-hashing "more complete example is provided for libmuttng"  in C++
          using a typesafe wrapper doing typecasts only though a C equivalent would
          be exactly the same.
        

    
    
    @subsubsection sect_devguide-core-intl Internationalization
    
        As usual, the support for internationalization is built on top of GNU gettext.
        The file <tt>intl.h</tt> defines the following two macros for use regardless
        whether the caller wishes to link in gettext or not:
      

    <ol>
    <li><tt>_(X)</tt>: the string <tt>X</tt> is to be translated. This is
          to be used in regular code.</li>
    <li><tt>N_(X)</tt>: is the same as the above but for use with in places
          with constant initialization such as static arrays or strings.</li>
    
      </ol>
    
        It's still inconsistent: as soon as the client knows its local character set,
        it should call <tt>intl_encoding()</tt>. This is planned to be merged into
        the <tt>core_init()</tt> call.
      

    
    @subsubsection sect_devguide-core-net Networking
    
        The core library has two features currently regarding networking:
      

    <ol>
    <li>it can parse <tt>/etc/resolv.conf</tt> for a systems (assumed) DNS domain
          name via <tt>net_dnsdomainname()</tt></li>
    <li>it supports IDN (international domain names)</li>
    
      </ol>
    @paragraph sect_devguide-core-net-idn IDN
          The IDN support is build upon <tt>libidn</tt> and only consists of
          two functions:
        

    <ol>
    <li>convert an international domain name from a local character set
            to the actual ASCII-DNS name</li>
    <li>convert from ASCII-DNS to a local character set</li>
    
      </ol>
    
          All valid international domain names can be converted to an ASCII-DNS
          equivalent.
        

    
          However, when converting from ASCII-DNS back to a local character
          set, characters may appear which cannot be represented in the target
          character set. In such cases the caller is free to choose whether loss
          is acceptable or not by passing the right flag to
          <tt>net_idn2local()</tt>.
        

    
          In case of errors encoutered, the decoded or encoded version is just
          the very same as the original.
        

    
    
    @subsubsection sect_devguide-core-rx Regular expressions
    
        The core library contains an abstraction layer for regular expressions
        allowing different backends and more convenient handling.
      

    
        "Convenience" means for instance to have a printable version of the
        pattern available at any time instead of only the compiled version.
      

    
        The only backend supported at the moment is the usual system's regular expression
        implementation <tt>regex(3)</tt> but support for
        <tt>pcre(3)</tt> will be built in, too.
      

    
    @subsubsection sect_devguide-core-signals Signal handling
    
    @subsubsection sect_devguide-core-commands Command handling
    
        The core library contains convenient functions for running external
        commands in different ways:
      

    <ol>
    <li>either "standalone" or</li>
    <li>as filter</li>
    
      </ol>
    
        The "standalone" way is via <tt>command_run()</tt> which internally
        executes the command using <tt>sh(1)</tt> to not have to mess with
        setting up a command-line preparing routing.
      

    
        The filter mode can be used to run command with pre-defined <tt>stdin</tt>,
        <tt>stdout</tt> and <tt>stderr</tt> file descriptors to, for example,
        make a command read input from a file and print the output to another.
        This can also be achieved using shell redirection in
        <tt>command_run()</tt> but callers may have the files opened
        already so <tt>command_filter()</tt> is a better alternative.
      

    
    @subsubsection sect_devguide-core-io Basic I/O
    
        The files <tt>io.c</tt> and <tt>io.h</tt> respectively define various
        filesystem- or I/O-related functions:
      

    <ul>
    <li><tt>io_open_read()</tt> is built on top of the command handling and
          can be used to abstract from a source of input. As known from
          muttng's source command, instead of from a file the output may come from
          a command being executed if the filename ends in <tt>|</tt>. This is the
          only function behind it.</li>
    <li><tt>io_tempfile()</tt> can be used to obtain opened temporary files
          more flexibly as it allowes for custom directories or name suggestions.</li>
    <li><tt>io_readline()</tt> can be used to read arbitrary long lines from
          input files while supporting linue continuations: if a line ends with
          a backslash, the it's concatenated with the next line(s) unless one line
          is finished.</li>
    
      </ul>
    
        Other functions defined in <tt>io.h</tt> mainly are sanity wrappers.
      

    
    @subsubsection sect_devguide-core-rx Regular expressions
    
        The core layer supports two types of regular expressions:
      

    <ol>
    <li>POSIX: it must be provided by the system</li>
    <li>PCRE: for perl-compatibility, libpcre must be installed</li>
    
      </ol>
    
        All details are hidden behind a structure named <tt>rx_t</tt> as defined
        in <tt>rx.h</tt>. Among the feature to always store a printable version
        of the pattern along with the compiled one, it also allows to specify whether
        a subject string should not match a pattern.
      

    
        The functions provided are:
      

    <ul>
    <li><tt>rc_compile()</tt> and <tt>rx_free()</tt>: compile a pattern and
          free all memory allocated for a <tt>rx_t</tt> structure</li>
    <li><tt>rc_match()</tt> and <tt>rc_exec()</tt>: see if a subject string
          matches the pattern. These two are identical except that for the latter
          the caller must provide storage for a <tt>rx_match_t</tt> array which is used to
          encode where matches are located. The first only tests whether it matches
          at all.</li>
    <li><tt>rc_eq()</tt>: a very weak way to compare to regular expressions by
          comparing their string representations. It's weak since the following two
          patterns are identical though their strings are different:
          <tt>[ab]{2}[ab]{2,}</tt> and <tt>[ab]{4,}</tt>.</li>
    <li><tt>rx_version()</tt>: in case a library is used for regular expression
          support, this obtains its version number.</li>
    
      </ul>
    
    @subsubsection sect_devguide-core-extending Extending the library
    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    @subsection sect_devguide-libmuttng Libmuttng
    @subsubsection sect_devguide-libmuttng-features Features
    
        Libmuttng is in the <tt>src/libmuttng/</tt> subdirectory
        and provides the following features:
      

    <ul>
    <li><em>utilities</em> helping to keep the code modular, clean
          and readable: some of the RfC parsers/routines for use
          in different parts, cheap but typesafe wrappers for the
          generic data structures provided by the core layer and more</li>
    <li><em>basic services</em> of interest for the whole library and
          application: signal handling and debug support as well a centralized
          interface for configuration options</li>
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
    
    @subsubsection sect_devguide-libmuttng-hashing Again: hashing
    
        As one of the major disadvantages of the core layer's generic
        hash table is the fact that typecasts make the C code harder
        to read, libmuttng contains a lightweight template-based
        wrapper to have a typesafe and more readable (in a C++ meaning)
        variant of a hash table.
      

    
        Due to the fact that it's just a wrapper adding no extra
        features or changing semantics, all what counts for the core
        layer's hash table counts for the wrapper in libmuttng, too.
      

    
        To show how readability is improved,
        @ref sample-libmuttng-hashing "a C++ example" 
        is provided.
      

    @anchor sample-libmuttng-hashing
    @include libmuttng_hash.cpp
            
    @subsubsection sect_devguide-libmuttng-signal Signal handling
    
        Libmuttng contains a very simple, easy to use and typesafe signal
        handling interface. In fact, "signal" is a little misleading as
        it's after all a callback interface but since it's used to signal
        a certain action, event or request to interested parts, we call
        it "signal." Though signals will be used within the library,
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
    @include libmuttng_signal.cpp
            
    
    @subsubsection sect_devguide-libmuttng-config Configuration handling
    
        Built on top of signal handling, libmuttng provides a simple mechanism
        for managing configuration variables via so-called ConfigManager class. It's
        the central interface and can be accessed in two ways with two different
        semantics:
      

    <ol>
    <li><em>Registration.</em> This way is intended for modules (even outside
          of libmuttng) wanting to have a configuration variable globally
          available.</li>
    <li><em>Queries.</em> This way is intended for parts of the library and
          clients which either parse or print variables while the use of a
          variable's value is a rare case which shouldn't happen at all but
          may do so.</li>
    
      </ol>
    @paragraph sect_devguide-libmuttng-config-register Registering configuration variables
          Registration is done via the mentioned ConfigManager class which
          only knows that there are options but neither that there are
          different types nor which types there are. As a consequence,
          the caller must provide the option allocated elsewhere.
        

    
          The way to create a new option is basically always the same: just
          create an object with proper parameters and call
          <tt>ConfigManager::reg()</tt> with it. The common set for all options
          include an option's name, an initial value and a pointer to the actual
          storage of the variable. Besides these, some options
          provide more advanced features embedded in the object creation:
        

    <ul>
    <li><tt>Numbers.</tt> A numeric option can be created using the
            IntOption class which provides two constructors. These allow for specifying
            that a variable has a range of valid values. The default is to "restrict"
            a value to the whole integer range.</li>
    <li><tt>Strings.</tt> Optionally when creating a string option, a regular
            expression pattern can be supplied against which to check all values.</li>
    
      </ul>
    
          Passing a pointer for the actual storage is an imporant part of the
          design goal behind this way of configuration management. The idea is to keep
          the scope of a variable as small as possible to achieve modularity, separation
          and information hiding, that is, only those source parts which need access
          to the variable's storage should see it on the one hand, but visibility to the outside
          of the library must be given on the other hand.
        

              
          This is done in a modular 
          and flexible way. De-coupling the different parts of the library and client(s)
          is further improved by the fact, that every option has a signal emitted when
          it changes so that every part can take action when an option of its choice changes
          (and as seen with the signal examples, such bindings can even be created temporarily.)
        

    
          The big advantage is that access from the "user space" with "only" average O(1)
          is fast enough due to hashing but still "real" O(1) for the parts utilizing it.
        

    
          In order to achieve our memory management design goals, the configuration
          manager cleans up all storage when it's told to, normally when the application
          is about to exit. The implication is that source parts registering a variable
          are strongly encouraged to only read its storage but not change it any way. The configuration
          manager handles allocation and deallocation of memory for the option's storage
          so it may produce conflicts when the creator tries to do so, too.
        

    
    @paragraph sect_devguide-libmuttng-config-query Querying configuration variables
          Summarized under the term "query", there's a number of features available
          for clients of the library. These include not only to query for a current
          value or change it, but also reset or unset, i.e. set it to a neutral value
          for a certain option (hiding what this specifically means.)
        

    
          For details, please see the following methods:
        

    <ul>
    <li><tt>Option::set()</tt>: change a value</li>
    <li><tt>Option::unset()</tt>: change to a neutral value</li>
    <li><tt>Option::toggle()</tt>: toggle or invert a value if supported</li>
    <li><tt>Option::query()</tt>: obtain current value</li>
    <li><tt>Option::getType()</tt>: get localized type string</li>
    <li><tt>Option::validity()</tt>: obtain any limitations for a specific variable</li>
    
      </ul>
    
    @paragraph sect_devguide-libmuttng-config-docs Documenting variables
          This is fully broken at the moment but will be fixed soon.
        

    
    
    @subsubsection sect_devguide-libmuttng-url URL handling
    
        The syntax for supported URLs is:
      

    <pre>
proto[s]://[username[:password]@]host[:port][/path]</pre>
        whereby <tt>proto</tt> is any of the following protocolls:
        <tt>imap[s]</tt>, <tt>pop[s]</tt>, <tt>nntp[s]</tt>, <tt>smtp[s]</tt>
        or <tt>file</tt>.
      

    
        The string <tt>[s]</tt> denotes a secure connection to be used for those protocolls
        supporting it.
      

    
        The host part is treated specially in some way and may contain domain names
        as well IP addresses. For specifying IPv6 addresses, it has to enclosed in
        <tt>[]</tt>. For example, a secure IMAP connection to IPv6 loopback on port
        <tt>4711</tt> for user <tt>joe</tt> with password <tt>secret</tt> is:
        <pre>
imaps://joe:secret@[::1]:4711/</pre>.
        For implementations such as KAME-derived ones <em>(as found on BSD systems)</em>,
        an interface ID may be specified the usual way by appending <tt>\%ID</tt>. Note that
        as <tt>\%</tt> is used to encode non-ASCII or other special characters in
        URLs, it has to be encoded like so (note the encoded <tt>\%25</tt>):
        <pre>
imaps://joe:secret@[fe80::1%25lo0]:4711/</pre>

    
        For further complication: the hostname, if it's not an IPv4 or IPv6 address,
        is expected to be an international domain encoded in the clients character set.
        It's parsed into punycode internally.
      

    
        LibMuttng supports parsing a string into such an URL and <em>always</em>
        fully qualifies the path, i.e. it will always have a leading slash (if no path
        is contained in the string, the path is just the slash.)
      

    
        Please see @ref sample-libmuttng-url "the example provided" 
        for use and error handling.
      

    @anchor sample-libmuttng-url
    @include libmuttng_url.cpp
            
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
      

    
        The implementation is split into two major parts via inheritance:
        local and remote folders:
      

    <ul>
    <li>local folders</li>
    <li>remote folders derived from the <tt>RemoteMailbox</tt> class
          get protected attributes for caching, a connection, as well
          as a send and a receive buffer. The underlying <tt>RemoteMailbox</tt>
          class is reposponsible for allocation and de-allocation of the
          buffers.</li>
    
      </ul>
    @paragraph sect_devguide-libmuttng-mailbox-create Creating a mailbox
          Creating a new instance of a mailbox based on the URL (other ways
          are not planned), use the Mailbox::fromURL() function as shown
          @ref sample-libmuttng-mailbox-create "in the example" .
        

    @anchor sample-libmuttng-mailbox-create
    @include libmuttng_mailbox_create.cpp
            
          Internally, when receiving a request this way, the Mailbox class
          distincts between the following two base types of mailboxes:
        

    <ol>
    <li>local or</li>
    <li>remote mailbox</li>
    
      </ol>
    
          For remote mailboxes, all details are to be handled by the RemoteMailbox class.
          Depending on the protocoll, it queries the final mailbox type for whether
          it already has a connection or not whereby it does not know why this step
          is done. If no existing connection was returned in this step, a new one
          is created. With either the fresh or the existing one the mailbox' constructor
          is called and a pointer to it returned from RemoteMailbox::fromURL(). For example,
          the NNTPMailbox class doesn't use a new connection for every new folder to
          a given host but reuses them as far as possible while POP3Mailbox uses a new
          connection for every single folder. It's left to the actual implementation to
          decide such details.
        

    
    
    @subsubsection sect_devguide-libmuttng-connection Connection handling
    
        Behind the generic connection implementation provided as the Connection
        class, the following implementing classes are transparently hidden behind
        it:
      

    <ul>
    <li><tt>PlainConnection</tt> class provides plaintext, i.e. un-encrypted
          transport</li>
    <li><tt>TLSConnection</tt> class provides SSL 3.0 and TLS 1.0 support
          with the <tt>gnutls</tt> library as backend</li>
    <li><tt>SSLConnection</tt> class provides SSL 3.0 support
          with the <tt>openssl</tt> library as backend</li>
    
      </ul>
    
        As are mailboxes, connections have to be created from an URL so that
        only for those URLs connections can be created for which a protocoll
        is known. Alternatively the caller can provide a custom <tt>url_t</tt>
        structure instead of using the default way which is to call
        <tt>url_from_string()</tt> for the parsing.
      

    
        For a caller, there're only the following functions intended for use:
      

    <ul>
    <li><tt>socketConnect()</tt> and <tt>socketDisconnect()</tt> do DNS
          lookups and open or close the socket.</li>
    <li><tt>writeLine()</tt> and <tt>readLine()</tt> write or read any data
          to/from a socket.</li>
    
      </ul>
    
        As part of the abstraction, the latter two hide the fact that lines written or
        read end in CRLF: <tt>writeLine()</tt> adds these on its own while
        <tt>readLine()</tt> strips them.
      

    
        The signals <tt>sigPreconnect</tt> and <tt>sigPostconnect</tt> are emitted
        right before a connection is made and right after it has been closed
        for clients to catch them. These can be used, for example, to establish
        and terminate dial-up connections.
      

    
        The implementation is portable and protocoll independent so that
        IPv6 is of course supported.
      

    
        To save resources, the Connection class maintains a list of connections created
        so far in order to recycle them. The decission is made based on whether
        <tt>url_eq()</tt> reports equality for two URLs or not. This is weak and will
        be fine-tuned.
      

    
    @subsubsection sect_devguide-libmuttng-nntp NNTP support
    
        Libmuttng supports the Network News Transfer Protocol as
        defined in <a href="http://www.faqs.org/rfcs/rfc977.html">RfC 977</a> as well as some extensions
        described in <a href="http://www.faqs.org/rfcs/rfc2980.html">RfC 2980</a>. <em>(Each time a connection
          is established, it's tested which extensions are
          supported by the server so that libmuttng works fine with
          a server only supporting <a href="http://www.faqs.org/rfcs/rfc977.html">RfC 977</a>)</em>

    @paragraph sect_devguigde-libmuttng-nntp-server Default server detection
          Libmuttng supports the following two ways to detect a
          default server by priority:
        

    <ol>
    <li>the <tt>$NNTPSERVER</tt> environment variable</li>
    <li>the following standard files containing the servername
            (comment line may start with <tt>#</tt>):
            <ol>
    <li><tt>/etc/nntpserver</tt></li>
    <li><tt>/etc/news/server</tt></li>
    
      </ol>
    </li>
    
      </ol>
    
    
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
    @subsubsection sect_devguide-muttng-tools Binaries
    
        Based on the core and libmuttng libraries, a number of clients or
        tools are built.
      

    
        All tools are derived from a single base class and support a set of
        @ref table-cli-common "common command-line options" .
      

    @anchor table-cli-common
    @htmlonly
    <p class="title">Common command-line options</p>
      <table class="ordinary" rowsep="1" summary="Common command-line options">
    <thead><tr><td>Switch</td><td>Argument</td><td>Meaning</td></tr>
    </thead><tbody><tr><td>-d</td><td>level</td><td>Set debug level</td></tr>
    <tr><td>-F</td><td>file</td><td>Read non-standard config file</td></tr>
    <tr><td>-h</td><td></td><td>Help screen</td></tr>
    <tr><td>-n</td><td></td><td>Bypass system config file</td></tr>
    <tr><td>-q</td><td></td><td>Quiet mode</td></tr>
    <tr><td>-v</td><td></td><td>Show version info</td></tr>
    <tr><td>-V</td><td></td><td>Show warranty and license</td></tr>
    </tbody>
      </table>
      @endhtmlonly
    

    @paragraph sect_devguide-muttng-tools-muttng muttng<tt>muttng(1)</tt> is the main user agent.
        

    
    @paragraph sect_devguide-muttng-tools-sync muttng-sync<tt>muttng-sync(1)</tt> is a tool capable of bi-directional syncing any
          two folders (as far as supported.)
        

    
    @paragraph sect_devguide-muttng-tools-mailx muttng-mailx<tt>muttng-mailx(1)</tt> is a true replacement for <tt>mailx(1)</tt>'s send mode.
        

    
    @paragraph sect_devguide-muttng-tools-query muttng-query<tt>muttng-query(1)</tt> is a tool to query folders and other items
          preferrably for use in scripts and other applications.
        

    
    @paragraph sect_devguide-muttng-tools-conf muttng-conf<tt>muttng-conf(1)</tt> is a powerful configuration diagnostics tool.
        

    
    
    @subsubsection sect_devguide-muttng-auto Auto-generated code
    
    @subsubsection sect_devguide-muttng-extending Extending
    
        When making extensions to the library and adding classes,
        please make sure to derive them from the Muttng base
        class to have debugging support and future extensions
        already in your class.
      

    
        When making any extensions are chaning an implementation,
        please make sure to run and maybe add/change/extend one
        of the unit tests to verify it's working.
      

    
    
    
    @section sect_licensing Licensing
    <ol>
    <li><b>GNU General Public License</b>:
      <tt>src/libmuttng/test/conn_tests.cpp</tt><tt>src/libmuttng/test/header_tests.h</tt><tt>src/libmuttng/test/conn_tests.h</tt><tt>src/libmuttng/test/test.cpp</tt><tt>src/libmuttng/test/header_tests.cpp</tt><tt>src/libmuttng/test/url_tests.cpp</tt><tt>src/libmuttng/test/url_tests.h</tt><tt>src/libmuttng/test/lib_tests.h</tt><tt>src/libmuttng/test/rfc2047_tests.cpp</tt><tt>src/libmuttng/test/rfc2047_tests.h</tt><tt>src/libmuttng/test/lib_tests.cpp</tt><tt>src/libmuttng/crypto/crypto_gpgme.cpp</tt><tt>src/libmuttng/crypto/classic_smime.h</tt><tt>src/libmuttng/crypto/crypto.cpp</tt><tt>src/libmuttng/crypto/classic_crypto.cpp</tt><tt>src/libmuttng/crypto/crypto_gpgme.h</tt><tt>src/libmuttng/crypto/crypto.h</tt><tt>src/libmuttng/crypto/classic_crypto.h</tt><tt>src/libmuttng/crypto/classic_pgp.cpp</tt><tt>src/libmuttng/crypto/classic_smime.cpp</tt><tt>src/libmuttng/crypto/classic_pgp.h</tt><tt>src/libmuttng/cache/cache_qdbm.cpp</tt><tt>src/libmuttng/cache/cache.cpp</tt><tt>src/libmuttng/cache/cache.h</tt><tt>src/libmuttng/cache/cache_qdbm.h</tt><tt>src/libmuttng/message/message.cpp</tt><tt>src/libmuttng/message/body.h</tt><tt>src/libmuttng/message/header.cpp</tt><tt>src/libmuttng/message/message.h</tt><tt>src/libmuttng/message/body.cpp</tt><tt>src/libmuttng/message/header.h</tt><tt>src/libmuttng/message/simple_header.h</tt><tt>src/libmuttng/message/simple_header.cpp</tt><tt>src/libmuttng/message/mailbox_header.h</tt><tt>src/libmuttng/message/mailbox_header.cpp</tt><tt>src/libmuttng/message/subject_header.cpp</tt><tt>src/libmuttng/message/subject_header.h</tt><tt>src/libmuttng/mailbox/mmdf_mailbox.cpp</tt><tt>src/libmuttng/mailbox/dir_mailbox.h</tt><tt>src/libmuttng/mailbox/maildir_mailbox.h</tt><tt>src/libmuttng/mailbox/file_mailbox.h</tt><tt>src/libmuttng/mailbox/nntp_mailbox.h</tt><tt>src/libmuttng/mailbox/mailbox.h</tt><tt>src/libmuttng/mailbox/imap_mailbox.cpp</tt><tt>src/libmuttng/mailbox/pop3_mailbox.h</tt><tt>src/libmuttng/mailbox/mmdf_mailbox.h</tt><tt>src/libmuttng/mailbox/local_mailbox.cpp</tt><tt>src/libmuttng/mailbox/imap_mailbox.h</tt><tt>src/libmuttng/mailbox/remote_mailbox.cpp</tt><tt>src/libmuttng/mailbox/local_mailbox.h</tt><tt>src/libmuttng/mailbox/remote_mailbox.h</tt><tt>src/libmuttng/mailbox/mh_mailbox.cpp</tt><tt>src/libmuttng/mailbox/mbox_mailbox.cpp</tt><tt>src/libmuttng/mailbox/mh_mailbox.h</tt><tt>src/libmuttng/mailbox/mbox_mailbox.h</tt><tt>src/libmuttng/mailbox/dir_mailbox.cpp</tt><tt>src/libmuttng/mailbox/file_mailbox.cpp</tt><tt>src/libmuttng/mailbox/nntp_mailbox.cpp</tt><tt>src/libmuttng/mailbox/mailbox.cpp</tt><tt>src/libmuttng/mailbox/maildir_mailbox.cpp</tt><tt>src/libmuttng/mailbox/pop3_mailbox.cpp</tt><tt>src/libmuttng/transport/ssl_connection.cpp</tt><tt>src/libmuttng/transport/connection.cpp</tt><tt>src/libmuttng/transport/plain_connection.cpp</tt><tt>src/libmuttng/transport/tls_connection.cpp</tt><tt>src/libmuttng/transport/ssl_connection.h</tt><tt>src/libmuttng/transport/connection.h</tt><tt>src/libmuttng/transport/tls_connection.h</tt><tt>src/libmuttng/transport/plain_connection.h</tt><tt>src/libmuttng/util/url.cpp</tt><tt>src/libmuttng/util/rfc2047.cpp</tt><tt>src/libmuttng/util/url.h</tt><tt>src/libmuttng/util/rfc2047.h</tt><tt>src/libmuttng/libmuttng.cpp</tt><tt>src/libmuttng/debug.cpp</tt><tt>src/libmuttng/libmuttng.h</tt><tt>src/libmuttng/version.h</tt><tt>src/libmuttng/debug.h</tt><tt>src/libmuttng/config/option.h</tt><tt>src/libmuttng/config/int_option.h</tt><tt>src/libmuttng/config/int_option.cpp</tt><tt>src/libmuttng/config/config_manager.h</tt><tt>src/libmuttng/config/option.cpp</tt><tt>src/libmuttng/config/string_option.h</tt><tt>src/libmuttng/config/config_manager.cpp</tt><tt>src/libmuttng/config/string_option.cpp</tt><tt>src/libmuttng/config/rx_option.h</tt><tt>src/libmuttng/config/bool_option.h</tt><tt>src/libmuttng/config/bool_option.cpp</tt><tt>src/libmuttng/config/url_option.h</tt><tt>src/libmuttng/config/url_option.cpp</tt><tt>src/libmuttng/config/rx_option.cpp</tt><tt>src/libmuttng/config/quad_option.h</tt><tt>src/libmuttng/config/quad_option.cpp</tt><tt>src/libmuttng/config/sys_option.h</tt><tt>src/libmuttng/config/sys_option.cpp</tt><tt>src/libmuttng/config/syn_option.h</tt><tt>src/libmuttng/config/syn_option.cpp</tt><tt>src/core/exit.h</tt><tt>src/core/mem.c</tt><tt>src/core/mem.h</tt><tt>src/core/str.c</tt><tt>src/core/exit.c</tt><tt>src/core/version.h</tt><tt>src/core/intl.h</tt><tt>src/core/str.h</tt><tt>src/core/io.c</tt><tt>src/core/conv.h</tt><tt>src/core/io.h</tt><tt>src/core/conv.c</tt><tt>src/core/array.h</tt><tt>src/core/test/buffer_tests.h</tt><tt>src/core/test/test.cpp</tt><tt>src/core/test/buffer_tests.cpp</tt><tt>src/core/test/hash_tests.cpp</tt><tt>src/core/test/hash_tests.h</tt><tt>src/core/test/list_tests.cpp</tt><tt>src/core/test/conv_tests.h</tt><tt>src/core/test/list_tests.h</tt><tt>src/core/test/io_tests.h</tt><tt>src/core/test/io_tests.cpp</tt><tt>src/core/test/conv_tests.cpp</tt><tt>src/core/test/net_tests.cpp</tt><tt>src/core/test/net_tests.h</tt><tt>src/core/test/qp_tests.cpp</tt><tt>src/core/test/base64_tests.cpp</tt><tt>src/core/test/base64_tests.h</tt><tt>src/core/test/qp_tests.h</tt><tt>src/core/test/rx_tests.h</tt><tt>src/core/test/rx_tests.cpp</tt><tt>src/core/rx.h</tt><tt>src/core/core.h</tt><tt>src/core/command.c</tt><tt>src/core/sigs.h</tt><tt>src/core/core.c</tt><tt>src/core/sigs.c</tt><tt>src/core/command.h</tt><tt>src/core/net.c</tt><tt>src/core/net.h</tt><tt>src/core/buffer_token.h</tt><tt>src/core/intl.c</tt><tt>src/core/rx_posix.c</tt><tt>src/core/buffer_token.c</tt><tt>src/core/buffer_qp.c</tt><tt>src/core/buffer_qp.h</tt><tt>src/core/buffer_base64.c</tt><tt>src/core/buffer_base64.h</tt><tt>src/muttng/tools/muttng_tool.h</tt><tt>src/muttng/tools/sync_tool.h</tt><tt>src/muttng/tools/conf_tool.cpp</tt><tt>src/muttng/tools/conf_tool.h</tt><tt>src/muttng/tools/tool.h</tt><tt>src/muttng/tools/main.cpp</tt><tt>src/muttng/tools/mailx_tool.cpp</tt><tt>src/muttng/tools/sync_tool.cpp</tt><tt>src/muttng/tools/muttng_tool.cpp</tt><tt>src/muttng/tools/mailx_tool.h</tt><tt>src/muttng/tools/tool.cpp</tt><tt>src/muttng/tools/query_tool.h</tt><tt>src/muttng/tools/query_tool.cpp</tt><tt>src/muttng/event/event.h</tt><tt>src/muttng/config/config.h</tt><tt>src/muttng/config/global_variables.h</tt><tt>src/muttng/config/config.cpp</tt><tt>src/muttng/ui/ui_plain.cpp</tt><tt>src/muttng/ui/ui_curses.cpp</tt><tt>src/muttng/ui/ui_text.cpp</tt><tt>src/muttng/ui/ui.h</tt><tt>src/muttng/ui/ui_plain.h</tt><tt>src/muttng/ui/ui_curses.h</tt><tt>src/muttng/ui/ui_text.h</tt><tt>src/muttng/ui/ui.cpp</tt><tt>src/muttng/muttng.cpp</tt><tt>src/muttng/muttng.h</tt></li>
    <li><b>GNU Lesser General Public License</b>:
      <tt>src/libmuttng/util/hash.h</tt><tt>src/core/buffer.h</tt><tt>src/core/hash.h</tt><tt>src/core/hash.c</tt><tt>src/core/list.h</tt><tt>src/core/list.c</tt><tt>src/core/buffer_format.c</tt><tt>src/core/buffer_base.c</tt><tt>src/core/buffer_base.h</tt><tt>src/core/buffer_format.h</tt><tt>src/core/rx_pcre.c</tt></li>
    <li><b>public domain</b>:
      <tt>src/libmuttng/test/signal_tests.cpp</tt><tt>src/libmuttng/test/signal_tests.h</tt><tt>src/libmuttng/muttng_signal.h</tt><tt>src/libmuttng/signal.pl</tt><tt>doc/examples/core_hash.c</tt><tt>doc/examples/libmuttng_signal.cpp</tt><tt>doc/examples/libmuttng_mailbox_create.cpp</tt><tt>doc/examples/libmuttng_url.cpp</tt><tt>doc/examples/libmuttng_hash.cpp</tt><tt>doc/examples/core_buffer_format.c</tt><tt>doc/examples/core_buffer_format2.c</tt><tt>contrib/html_head_number.pl</tt><tt>contrib/licensing.pl</tt></li>
    
      </ol>
    
    @section sect_history Mutt-ng History
    <ul>
    <li><em>2005/11/10</em> First line of code checked in for rewrite.</li>
    <li><em>2005/11/07</em> First checking of design ideas for rewrite.</li>
    <li><em>2005/01/07</em> Initial revision checked in.</li>
    
      </ul>
    
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
    