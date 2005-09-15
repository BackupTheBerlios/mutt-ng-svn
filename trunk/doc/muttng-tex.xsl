<?xml version="1.0" encoding="utf-8"?>

<!-- manual2latex.xsl                                   -->

<!-- written for mutt-ng by:                            -->
<!-- Rocco Rutte <pdmef@cs.tu-berlin.de>                -->

<!-- the LaTeX-escaping code at the end is a            -->
<!-- slight modification of mine but mostly             -->
<!-- identical to:                                      -->
<!-- http://www.w3.org/2004/04/xhlt91/                  -->

<!-- NOTE: this is not a generic DocBook to LaTeX but   -->
<!--       it contains just enough to convert the       -->
<!--       mutt-ng manual with some special tags into   -->
<!--       valid LaTeX code; most of the work is done   -->
<!--       muttng.sty anyways...                        -->

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:muttng-doc="http://mutt-ng.berlios.de/doc/#NS">

  <xsl:output method="text" indent="no" output-encoding="utf-8"/>

  <xsl:strip-space elements="*"/>

  <!--{{{ Document header -->

  <xsl:template match="book">
    <xsl:text>
      \documentclass[a4paper]{scrbook}
      \usepackage{muttng}
    </xsl:text>
    <xsl:apply-templates select="bookinfo"/>
    <xsl:apply-templates select="chapter"/>
    <xsl:apply-templates select="appendix"/>
    <xsl:text>
      \printindex
      \end{document}
    </xsl:text>
  </xsl:template>

  <xsl:template match="bookinfo">
    <!-- dump stuff for titlepage -->
    <xsl:text>
      \author{</xsl:text>
    <xsl:for-each select="author">
      <xsl:apply-templates/>
      <xsl:if test='following-sibling::author'>
        <xsl:text> \and </xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>}
      \title{</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}</xsl:text>
    <xsl:text>
      \date{</xsl:text>
    <xsl:apply-templates select="pubdate"/>
    <xsl:text>}</xsl:text>

    <!-- dump stuff for hyperref again -->
    <xsl:text>
      \ifpdf
      \hypersetup{
      pdfauthor={</xsl:text>
    <xsl:for-each select="author">
      <xsl:apply-templates select="email" mode="pdfinfo"/>
      <xsl:text> (</xsl:text>
      <xsl:apply-templates select="firstname" mode="pdfinfo"/>
      <xsl:apply-templates select="surname" mode="pdfinfo"/>
      <xsl:text>)</xsl:text>
      <xsl:if test='following-sibling::author'>
        <xsl:text>, </xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>},
      pdftitle={</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}
      }
      \fi
    </xsl:text>
    <xsl:text>
      \begin{document}
      \maketitle
    </xsl:text>
    <xsl:apply-templates select="abstract"/>
    <xsl:text>
      \tableofcontents
      \listoftables
    </xsl:text>
  </xsl:template>

  <xsl:template match="pubdate">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="abstract">
    <xsl:text>\lowertitleback{</xsl:text>
    <xsl:apply-templates select="para"/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <!--}}}-->

  <!--{{{ Sectioning -->

  <xsl:template match="chapter">
    <xsl:text>
      \chapter{</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}</xsl:text>
    <xsl:if test="@id">
      <xsl:text>\hypertarget{</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>}{}</xsl:text>
    </xsl:if>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="appendix">
    <xsl:text>
      \myappendix
      \chapter{</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}</xsl:text>
    <xsl:if test="@id">
      <xsl:text>\hypertarget{</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>}{}</xsl:text>
    </xsl:if>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="sect1">
    <xsl:text>
      \section{</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}</xsl:text>
    <xsl:if test="@id">
      <xsl:text>\hypertarget{</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>}{}</xsl:text>
    </xsl:if>
    <xsl:text>

    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="sect2">
    <xsl:text>
      \subsection{</xsl:text>
    <xsl:apply-templates select="title" mode="toc"/>
    <xsl:text>}</xsl:text>
    <xsl:if test="@id">
      <xsl:text>\hypertarget{</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>}{}</xsl:text>
    </xsl:if>
    <xsl:text>

    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="title"/>

  <xsl:template match="title" mode="toc">
    <xsl:value-of select="."/>
  </xsl:template>

  <!--}}}-->

  <!--{{{ DocBook misc. -->

  <xsl:template match="firstname">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="surname">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="email">
    <xsl:text>\mailto{</xsl:text><xsl:value-of
      select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="firstname" mode="pdfinfo">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="surname" mode="pdfinfo">
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="email" mode="pdfinfo">
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="para">
    <xsl:apply-templates/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="screen">
    <xsl:text>\begin{verbatim}</xsl:text>
    <xsl:value-of select="text()"/><xsl:text>\end{verbatim}</xsl:text>
  </xsl:template>

  <xsl:template match="link">
    <xsl:text>\hyperlink{</xsl:text>
    <xsl:value-of select="@linkend"/>
    <xsl:text>}{</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <!--}}}-->

  <!--{{{ Text formatting -->

  <xsl:template match="emphasis">
    <xsl:choose>
      <xsl:when test="@role='bold'">
        <xsl:text>\textbf{</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>\textsl{</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="literal">
    <xsl:text>\texttt{</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <!--}}}-->

  <!--{{{ Lists -->

  <xsl:template match="itemizedlist">
    <xsl:text>
      \begin{itemize}
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
      \end{itemize}
    </xsl:text>
  </xsl:template>

  <xsl:template match="orderedlist">
    <xsl:text>
      \begin{enumerate}
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
      \end{enumerate}
    </xsl:text>
  </xsl:template>

  <xsl:template match="variablelist">
    <xsl:text>
      \begin{description}
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
      \end{description}
    </xsl:text>
  </xsl:template>

  <xsl:template match="listitem">
    <xsl:text>\item </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="varlistentry">
    <xsl:text>\item[</xsl:text>
    <xsl:apply-templates select="term"/>
    <xsl:text>] </xsl:text>
    <xsl:apply-templates select="listitem" mode="varterm"/>
  </xsl:template>

  <xsl:template match="term">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="listitem" mode="varterm">
    <xsl:apply-templates/>
  </xsl:template>

  <!--}}}-->

  <!--{{{ Tables -->

  <xsl:template match="table">
    <xsl:text>
      \begin{longtable}{</xsl:text>
    <xsl:value-of select="@texstr"/>
    <xsl:text>}
    </xsl:text>
    <xsl:apply-templates select="title" mode="table"/>
    <xsl:if test="@id">
      <xsl:text>
        \hypertarget{</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>}{}</xsl:text>
    </xsl:if>
    <xsl:apply-templates select="tgroup"/>
    <xsl:text>
      \end{longtable}
    </xsl:text>
  </xsl:template>

  <xsl:template match="tgroup">
    <xsl:apply-templates select="thead"/>
    <xsl:apply-templates select="tbody"/>
    <xsl:apply-templates select="title" mode="table"/>
  </xsl:template>

  <xsl:template match="thead">
    <xsl:apply-templates select="row" mode="head"/>
  </xsl:template>

  <xsl:template match="tbody">
    <xsl:apply-templates select="row" mode="body"/>
  </xsl:template>

  <xsl:template match="title" mode="table">
    <xsl:text>\caption{</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>} \\
    </xsl:text>
  </xsl:template>

  <xsl:template match="entry" mode="table">
    <xsl:apply-templates/>
    <xsl:if test="position()!=last()">
      <xsl:text> &amp; </xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="row" mode="head">
    <xsl:apply-templates select="entry" mode="table"/>
    <xsl:text> \\ \hline\hline
      \endfirsthead
      \endhead
    </xsl:text>
  </xsl:template>

  <xsl:template match="row" mode="body">
    <xsl:apply-templates select="entry" mode="table"/>
    <xsl:if test="position()!=last()">
      <xsl:text> \\ \hline</xsl:text>
    </xsl:if>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <!--}}}-->

  <!--{{{ muttng-doc namespace -->

  <xsl:template match="muttng-doc:man">
    <xsl:text>\man</xsl:text>
     <xsl:if test="@sect">
       <xsl:text>[</xsl:text><xsl:value-of
         select="@sect"/><xsl:text>]</xsl:text>
     </xsl:if>
     <xsl:text>{</xsl:text>
     <xsl:value-of select="@name"/>
     <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:web">
    <xsl:text>\web{</xsl:text><xsl:value-of
      select="@url"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:envvar">
    <xsl:text>\envvar{</xsl:text><xsl:value-of select="@name"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:hook">
    <xsl:text>\hook{</xsl:text><xsl:value-of select="@name"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:cmddef">
    <xsl:text>\cmddef{</xsl:text><xsl:value-of select="@name"/><xsl:text>}{</xsl:text>
    <xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:cmdref">
    <xsl:text>\cmdref{</xsl:text><xsl:value-of select="@name"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:funcref">
    <xsl:text>\funcref{</xsl:text><xsl:value-of select="@name"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:funcdef">
    <xsl:text>\funcdef{</xsl:text><xsl:value-of select="@name"/>
    <xsl:text>}{</xsl:text>
    <xsl:apply-templates select="muttng-doc:key"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:varref">
    <xsl:text>\varref{</xsl:text><xsl:value-of select="translate(@name,'-','_')"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:vardef">
    <xsl:text>\vardef{</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>}
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="muttng-doc:rfc">
    <xsl:text>\rfc{</xsl:text><xsl:value-of select="@num"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:lstconf">
    <xsl:text>\begin{verbatim}</xsl:text>
    <xsl:value-of select="text()"/><xsl:text>\end{verbatim}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:lstmail">
    <xsl:text>\begin{verbatim}</xsl:text>
    <xsl:value-of select="text()"/><xsl:text>\end{verbatim}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:lstshell">
    <xsl:text>\begin{verbatim}</xsl:text>
    <xsl:value-of select="text()"/><xsl:text>\end{verbatim}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:pattern">
    <xsl:text>\pat</xsl:text>
    <xsl:if test="@full='1'">
      <xsl:text>[1]</xsl:text>
    </xsl:if>
    <xsl:text>{</xsl:text><xsl:value-of
      select="@name"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:key">
    <xsl:text>\key{</xsl:text>
    <xsl:if test="@mod">
      <xsl:value-of select="@mod"/><xsl:text>-</xsl:text>
    </xsl:if>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="muttng-doc:special">
    <xsl:value-of select="@latex"/>
  </xsl:template>

  <!--}}}-->

  <!--{{{ _slooooooow_ TeX escaping stolen from W3C -->

  <xsl:template match="text()">
    <xsl:call-template name="esc">
      <xsl:with-param name="c" select='"#"'/>
      <xsl:with-param name="s">
        <xsl:call-template name="esc">
          <xsl:with-param name="c" select='"$"'/>
          <xsl:with-param name="s">
            <xsl:call-template name="esc">
              <xsl:with-param name="c" select='"%"'/>
              <xsl:with-param name="s">
                <xsl:call-template name="esc">
                  <xsl:with-param name="c" select='"&amp;"'/>
                  <xsl:with-param name="s">
                    <xsl:call-template name="esc">
                      <xsl:with-param name="c" select='"~"'/>
                      <xsl:with-param name="s">
                        <xsl:call-template name="esc">
                          <xsl:with-param name="c" select='"_"'/>
                          <xsl:with-param name="s">
                            <xsl:call-template name="esc">
                              <xsl:with-param name="c" select='"^"'/>
                              <xsl:with-param name="s">
                                <xsl:call-template name="esc">
                                  <xsl:with-param name="c" select='"{"'/>
                                  <xsl:with-param name="s">
                                    <xsl:call-template name="esc">
                                      <xsl:with-param name="c" select='"}"'/>
                                      <xsl:with-param name="s">
                                        <xsl:call-template name="esc">
                                          <xsl:with-param name="c" select='"\"'/>
                                          <xsl:with-param name="s" select='.'/>
                                        </xsl:call-template>
                                      </xsl:with-param>
                                    </xsl:call-template>
                                  </xsl:with-param>
                                </xsl:call-template>
                              </xsl:with-param>
                            </xsl:call-template>
                          </xsl:with-param>
                        </xsl:call-template>
                      </xsl:with-param>
                    </xsl:call-template>
                  </xsl:with-param>
                </xsl:call-template>
              </xsl:with-param>
            </xsl:call-template>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="esc">
    <xsl:param name="s"/>
    <xsl:param name="c"/>

    <xsl:choose>
      <xsl:when test='contains($s, $c)'>
        <xsl:value-of select='substring-before($s, $c)'/>

        <xsl:choose>
          <!-- XXX puke -->
          <xsl:when test='$c = "\"'>
            <xsl:text>\textbackslash </xsl:text>
          </xsl:when>
          <!-- XXX puke -->
          <xsl:when test='$c = "~"'>
            <xsl:text>\char126 </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>\</xsl:text><xsl:value-of select='$c'/>
          </xsl:otherwise>
        </xsl:choose>

        <xsl:call-template name="esc">
          <xsl:with-param name='c' select='$c'/>
          <xsl:with-param name='s' select='substring-after($s, $c)'/>
      </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select='$s'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--}}}-->

</xsl:stylesheet>
