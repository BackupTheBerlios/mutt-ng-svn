/**
 * @page page_coding Coding Style
 *
 *   @section coding_doc Documentation
 *
 *     To keep the code documented and consistent, this section lists
 *     some rules to keep.
 *
 *     In general: document all files! Specify a doxygen header with at
 *     least the <code>@@file</code> and <code>@@brief</code> tags. For
 *     headers, the brief tag looks like:
 *
 *     <pre>
 * @@brief Interface: ...</pre>
 *
 *     and
 *
 *     <pre>
 * @@brief Implementation: ...</pre>
 *
 *     for source files.
 *
 *     @subsection coding_doc_hdr Header Files
 *
 *       For each source file, there is a header file listing all
 *       publicly accessable protos for the functions in the source
 *       file. All functions not listed in the header are to be declared
 *       static.
 *
 *       Each header file is wrapped within the following preprocessor
 *       magic:
 *
 *       <pre>
 * #ifndef LIBMUTTNG_FOO_BAR_H
 * #define LIBMUTTNG_FOO_BAR_H
 * ...
 * #endif</pre>
 *
 *       whereby the identifier is constructed from the filename under
 *       the <code>src/</code> directory,
 *       <code>libmuttng/foo/bar.h</code> in this case.
 *
 *  @section coding_misc Misc.
 *
 *    For debugging, make the code print debug info by semantic, not
 *    just any number.
 */
