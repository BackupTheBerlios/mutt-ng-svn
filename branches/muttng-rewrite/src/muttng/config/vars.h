/**
@file muttng/config/vars.h
@brief (AUTO) Option Reference
*/
/**
@page page_options Reference: Configuration Options
    @section option_abort_unmodified $abort_unmodified
      Type: @c quad-option, Initial value: '@c yes', Signal notification: no

    
      If set to @c yes, composition will automatically abort after
      editing the message body if no changes are made to the file (this
      check only happens after the <em>first</em> edit of the file).
      When set to @c no, composition will never be aborted.
    

    @section option_allow_8bit $allow_8bit
      Type: @c boolean, Initial value: '@c yes', Signal notification: no

    
      Controls whether 8-bit data is converted to 7-bit using either
      @c quoted-printable or @c base64 encoding when
      sending mail.
      

    @section option_assumed_charset $assumed_charset
      Type: @c string, Initial value: '@c us-ascii', Signal notification: no

    
      This variable is a colon-separated list of character encoding
      schemes for messages without character encoding indication.
      Header field values and message body content without character encoding
      indication would be assumed that they are written in one of this list.
      
      By default, all the header fields and message body without any charset
      indication are assumed to be in @c us-ascii.
      
      For example, Japanese users might prefer this:
      <pre>
      set assumed_charset="iso-2022-jp:euc-jp:shift_jis:utf-8"
      </pre>
      However, only the first content is valid for the message body.
      This variable is valid only if @ref option_strict_mime is @c unset.
      

    @section option_debug_level $debug_level
      Type: @c number, Initial value: '@c 0', Signal notification: yes

    
      This variable specifies the current debug level and, currently,
      must be in the range 0 to 5. The value 0 has the special meaning
      that no debug is to be generated. From a value of 1 to 5 the
      amount of debug info written increases drastically.
      
      Debug files will be written to the home directory by default and to
      the current if the home directory cannot be determinded.
      
      Debug files will have a name of the following format:
      .[scope].[pid].[id].log, whereby:
      - [scope] is an identifier for where the output came.
          One file will be created for libmuttng and one for the
          tool using it such as muttng or muttgn-conf
    - [pid] is the current process ID
    - [id] is the debug sequence number. For the first debug
          file of the current session it'll be 1 and increased for
          subsequent enabling/disabling of debug output via this variable
    

    @section option_folder $folder
      Type: @c url, Initial value: '@c file:///tmp/Mail', Signal notification: yes

    
      Specifies the default location of your mailboxes.
      
      A + or = at the beginning of a pathname will be expanded
      to the value of this variable.
      
      Note that if you change this variable from
      the default value you need to make sure that the assignment occurs
      <em>before</em> you use + or = for any other variables
      since expansion takes place during the set command.
      

    @section option_mbox $mbox
      Type: @c url, Initial value: '@c file:///tmp/mbox', Signal notification: no

    
      This specifies the folder into which read mail in your
      @ref option_spoolfile folder will be appended.
      

    @section option_postponed $postponed
      Type: @c url, Initial value: '@c file:///tmp/postponed', Signal notification: yes

    
      Mutt-ng allows you to indefinitely ``postpone sending a message'' which
      you are editing. When you choose to postpone a message, Mutt-ng saves it
      in the mailbox specified by this variable.
      
      Also see the @ref option_postpone variable.
      

    @section option_record $record
      Type: @c url, Initial value: '@c ', Signal notification: no

    
      This specifies the file into which your outgoing messages should be
      appended. (This is meant as the primary method for saving a copy of
      your messages, but another way to do this is using the my_hdr
      command to create a Bcc: header field with your email address in it.)
      
      The value of @ref option_record is overridden by the @ref option_force_name
      and @ref option_save_name variables, and the fcc-hook command.
      

    @section option_umask $umask
      Type: @c number, Initial value: '@c 0077', Signal notification: no

    
      This variable specifies the <em>octal</em> permissions for
      @c umask(2). See @c chmod(1) for possible
      value.
      

    
*/
    