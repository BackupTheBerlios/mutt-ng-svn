/**
@file manual.h
@brief (AUTO) manual
*/
/**
@page page_manual Manual

    @section sect_introduction Introduction
    
    @section sect_building Building and installation
    @subsection sect_build-requirements Build Requirements
          
      For building mutt-ng, you need to have the following software installed
      and working:
    <ul>
    <li>GNU make. Others won't work.</li>
    <li>A C and C++ compiler such as part of the GNU Compiler Collection.</li>
    
      </ul>
    
    @subsection sect_build-config Build Configuration
    
      In order to make the build process work at all, you need to create
      a file called <code> GNUmakefile.config.mine</code> in the top-level
      directory, i.e. in <code> mutt-ng/branches/muttng-rewrite</code>.
    
      There you can set various options. Most important are the following:
    <ul>
    <li><code> CCSTYLE=(gcc|suncc)</code>. Wether to assume use of GNU or Sun
        C/C++ compilers. Default: <code> gcc</code>.</li>
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
    The following types of configuration options are supported:<ul>
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
      
    
    @section sect_hacking Hacking
    @subsection sect_hacking-style Coding style
    Documentation
    
        To keep the code documented and consistent, this section lists
        some rules to keep.
      
        In general: document all files! Specify a doxygen header with at
        least the @@file and @@brief tags. For
        headers, the brief tag looks like:
      <pre>
@@brief Interface: ...</pre>and<pre>
@@brief Implementation: ...</pre>for source files.Header files
    
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
      Misc.
    
        For debugging, make the code print debug info by semantic, not
        just any number.
      
    
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
    