/**
* @file muttng/config/vars.h
* @brief (AUTO) Option Reference
*/
/**
* @page page_options Option Reference
* 
* @section option_allow_8bit $allow_8bit
*   Type: <code>boolean</code>,  Initial value: <code>'yes'</code><br> <br>
*   <p>
*   Controls whether 8-bit data is converted to 7-bit using either
*   <tt>quoted-printable</tt> or <tt>base64</tt> encoding when sending mail.
*   </p>
* @section option_assumed_charset $assumed_charset
*   Type: <code>string</code>,  Initial value: <code>'us-ascii'</code><br> <br>
* @section option_debug_level $debug_level
*   Type: <code>number</code>,  Initial value: <code>'0'</code><br> <br>
*   <p>
*   This variable specifies the current debug level and, currently,
*   must be in the range 0 to 5. The value 0 has the special meaning
*   that no debug is to be generated. From a value of 1 to 5 the
*   amount of debug info written increases drastically.
*   </p>
*   <p>
*   Debug files will be written to the home directory by default and to
*   the current if the home directory cannot be determinded.
*   </p>
*   <p>
*   Debug files will have a name of the following format:
*   <tt>.[scope].[pid].[id].log</tt>, whereby:
*   <ul>
*     <li><tt>[scope]</tt> is an identifier for where the output came.
*       One file will be created for <tt>libmuttng</tt> and one for the
*       tool using it such as <man>muttng</man> or <man>muttgn-conf</man>
*       </li>
*     <li><tt>[pid]</tt> is the current process ID</li>
*     <li><tt>[id]</tt> is the debug sequence number. For the first debug
*       file of the current session it'll be 1 and increased for
*       subsequent enabling/disabling of debug output via this variable</li>
*   </ul>
*   </p>
* @section option_umask $umask
*   Type: <code>number</code>,  Initial value: <code>'0077'</code><br> <br>
*   <p>
*   This variable specifies the <em>octal</em> permissions for
*   <man sect="2">umask</man>. See <man>chmod</man> for possible
*   value.
*   </p>
*/
