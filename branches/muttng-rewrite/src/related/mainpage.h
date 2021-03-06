/**
@file mainpage.h
@brief (AUTO) mainpage
*/
/**
@mainpage

    @section sect_introduction Introduction
    <em>Mutt next generation</em> (short <em>mutt-ng</em>) is a fork of the
        well-known email client <em>mutt</em> with the goal to both incorporate
        all the patches that are floating around in the web, and to fix all the
        other little annoyances of mutt.
      

    
        As we early ran intro problems due to the quality of the original source
        and its documentation, we're currently rewriting some parts from scratch.
        Thus, there's not really something to run and test yet.
      

    
        As one of the things we try to improve is documentation in general,
        the documentation is put here and updated frequently.
      

    
    @section sect_docs Documentation
    @subsection sect_docs-users For users
    
          As the manual will be re-worked completely, too, it is
          still far from being complete. The current manual is
          available in the following formats:
        

    <ul>
    <li><a href="http://mutt-ng.berlios.de/rewrite/en/manual/">http://mutt-ng.berlios.de/rewrite/en/manual/</a>: HTML, multiple files</li>
    <li><a href="http://mutt-ng.berlios.de/rewrite/en/manual/manual.html">http://mutt-ng.berlios.de/rewrite/en/manual/manual.html</a>: HTML, one file</li>
    <li><a href="http://mutt-ng.berlios.de/rewrite/en/manual/manual.pdf">http://mutt-ng.berlios.de/rewrite/en/manual/manual.pdf</a>: PDF</li>
    
      </ul>
    
    @subsection sect_docs-devel For developers
    
          The whole source code will be fully documented via
          doxygen (see <a href="http://www.doxygen.org/">http://www.doxygen.org/</a>.)
        

    
          It's only available in English:
          <a href="http://mutt-ng.berlios.de/rewrite/doxygen/">http://mutt-ng.berlios.de/rewrite/doxygen/</a>

    
    
    @section sect_download Download
    
        The rewritten source is not yet available as a compressed
        archived but only via subversion. To checkout a working
        copy (assuming subversion is installed), run:
      

    <pre>
$ svn checkout http://svn.berlios.de/svnroot/repos/mutt-ng/branches/muttng-rewrite</pre>
    @section sect_contributing Contributing
    
        As the rewrite is in an early stage, please do <em>not</em>
        open bug reports or feature request at our BerliOS which only concern
        the rewrite. Instead, please contact us via the developer's mailing list:
        <a href="mailto:mutt-ng-devel@lists.berlios.de">&lt;mutt-ng-devel@lists.berlios.de&gt;</a> only.
      

    
        There not yet many open tasks which volunteers can accept to solve in order
        to actively join the development:
      

    <ul>
    <li>Since the build process isn't the "common" way of using the Autotools
          but a custom make-based system, we need early feedback on how stable it
          works or if it works at all. Users can test whether the source can be compiled
          on different configurations: different operating systems, different
          compilers (especially the C++ parts.)</li>
    <li>Since one of the main goals is to have much better documentation being
          available in multiple languages, we ask for help with the documentation.
          This includes proof reading (spelling and content) as well as its structure
          (content as well as the build process and its automatation.) For translators
          there maybe will be mailinglist set up.</li>
    <li>Though the application is rewritten by keeping most of the current
          features, we of course ask for suggestions of critics of any kind. We can
          be contacted either via the developer's mailing list at
          <a href="mailto:mutt-ng-devel@lists.berlios.de">&lt;mutt-ng-devel@lists.berlios.de&gt;</a> or via our private mail 
          addresses. </li>
    
      </ul>
    
    @section sect_feeds Newsfeeds
    
        The following newsfeeds or similar sources of information
        can be used to stay up-to-date with the progress of the rewrite:
      

    <ul>
    <li>this page, of course ;-)</li>
    <li>the development blog found at:
          <a href="http://mutt-ng.supersized.org/">http://mutt-ng.supersized.org/</a></li>
    <li>the commit messages for the rewrite branch:
          <a href="http://user.cs.tu-berlin.de/~pdmef/muttng.cgi?c=muttng-rewrite">http://user.cs.tu-berlin.de/~pdmef/muttng.cgi?c=muttng-rewrite</a>
          (which also lists several newsfeeds such as RSS, Atom and MBOX)</li>
    
      </ul>
    
    
*/
    