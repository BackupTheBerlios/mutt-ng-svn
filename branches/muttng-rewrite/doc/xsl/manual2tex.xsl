<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text" indent="no"/>

  <xsl:strip-space elements="*"/>

  <xsl:template match="manual">
    <xsl:text>
      \documentclass[a4paper]{scrbook}
      \usepackage{muttng}
      \parindent0mm
    </xsl:text>
    <xsl:choose>
      <xsl:when test="$l10n.gentext.default.language='de'"><xsl:text>\usepackage[ngerman]{babel}</xsl:text></xsl:when>
      <xsl:when test="$l10n.gentext.default.language='en'"><xsl:text>\usepackage[american]{babel}</xsl:text></xsl:when>
    </xsl:choose>
    <xsl:text>
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
      \end{document}
    </xsl:text>
  </xsl:template>

  <xsl:template match="head">
    <xsl:apply-templates select="title" mode="head"/>
    <xsl:apply-templates select="authors" mode="head"/>
    <xsl:text>
      \begin{document}
      \maketitle
      \newpage
      \tableofcontents
      \newpage
      \listoffigures
      \newpage
      \listoftables
      \newpage
      \lstlistoflistings

    </xsl:text>
  </xsl:template>

  <xsl:template match="title" mode="head">
    <xsl:text>\title{</xsl:text><xsl:value-of select="."/><xsl:text>}
    </xsl:text>
  </xsl:template>

  <xsl:template match="authors" mode="head">
    <xsl:text>\author{</xsl:text>
    <xsl:for-each select="author">
      <xsl:apply-templates/>
      <xsl:if test='following-sibling::author'>
        <xsl:text> \and </xsl:text>
      </xsl:if>
      </xsl:for-each>
    <xsl:text>}
    </xsl:text>
  </xsl:template>

  <xsl:template match="translations"/>

  <xsl:template match="firstname">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="surname">
    <xsl:value-of select="."/><xsl:text> </xsl:text>
  </xsl:template>
 
  <xsl:template match="content">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="chapter">
    <xsl:text>\label{</xsl:text><xsl:value-of select="@id"/><xsl:text>}
    \chapter{</xsl:text><xsl:apply-templates select="title" mode="sect"/><xsl:text>}
    
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="section">
    <xsl:text>\label{</xsl:text><xsl:value-of select="@id"/><xsl:text>}
    \section{</xsl:text><xsl:apply-templates select="title" mode="sect"/><xsl:text>}
    
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="subsection">
    <xsl:text>\label{</xsl:text><xsl:value-of select="@id"/><xsl:text>}
    \subsection{</xsl:text><xsl:apply-templates select="title" mode="sect"/><xsl:text>}
    
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="subsubsection">
    <xsl:text>\label{</xsl:text><xsl:value-of select="@id"/><xsl:text>}
    \subsubsection{</xsl:text><xsl:value-of select="@title"/><xsl:text>}
    
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="title" mode="sect">
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="title"/>

  <xsl:template match="descriptions">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="listing">
    <xsl:text>\lstinputlisting[style=muttng</xsl:text>
    <xsl:choose>
      <xsl:when test="@lang='cpp'"><xsl:text>,language=C++</xsl:text></xsl:when>
      <xsl:when test="@lang='make'"><xsl:text>,language={[GNU]make}</xsl:text></xsl:when>
      <xsl:when test="@lang='muttngrc'"><xsl:text>,language=muttngrc</xsl:text></xsl:when>
    </xsl:choose>
    <xsl:text>,caption=</xsl:text><xsl:value-of select="@title"/>
    <xsl:text>,label=</xsl:text><xsl:value-of select="@id"/>
    <xsl:text>]{</xsl:text><xsl:value-of select="@href"/><xsl:text>}
    </xsl:text>
  </xsl:template>

  <xsl:template match="inlinelisting">
    <xsl:text>\lstinline[style=muttng,basicstyle=\ttfamily</xsl:text>
    <xsl:choose>
      <xsl:when test="@lang='cpp'"><xsl:text>,language=C++</xsl:text></xsl:when>
      <xsl:when test="@lang='make'"><xsl:text>,language={[GNU]make}</xsl:text></xsl:when>
      <xsl:when test="@lang='muttngrc'"><xsl:text>,language=muttngrc</xsl:text></xsl:when>
    </xsl:choose>
    <xsl:text>]{</xsl:text><xsl:value-of select="."/><xsl:text>}
    </xsl:text>
  </xsl:template>

  <xsl:template match="docref">
    <xsl:value-of select="."/><xsl:text> \vref{</xsl:text><xsl:value-of select="@href"/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="context">
    <xsl:text>\label{</xsl:text><xsl:value-of select="@name"/><xsl:text>}</xsl:text>
    <xsl:text>\subsubsection{Screen: </xsl:text><xsl:value-of select="@name"/><xsl:text>}
      \begin{itemize}
      \item dummy
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
      \end{itemize}
    </xsl:text>
  </xsl:template>

  <xsl:template match="function">
    <xsl:variable name="transid" select="concat('group-',@group)"/>
      <xsl:text>\item \textbf{\uglyesc{</xsl:text>
      <xsl:value-of select="@name"/><xsl:text>}} (</xsl:text><xsl:value-of select="//translations/trans[@id='defbind']"/><xsl:text>: '\uglyesc{</xsl:text><xsl:value-of select="@default"/><xsl:text>}', </xsl:text><xsl:value-of select="//translations/trans[@id='group']"/><xsl:text>: </xsl:text><xsl:value-of select="//translations/trans[@id=$transid]"/><xsl:text>): </xsl:text><xsl:value-of select="."/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="variable">
    <xsl:text>\label{option_</xsl:text><xsl:value-of select="@name"/><xsl:text>}
    </xsl:text>
    <xsl:text>\subsubsection{\texttt{\uglyesc{$</xsl:text><xsl:value-of select="@name"/><xsl:text>}}}
    </xsl:text>
    <xsl:text></xsl:text><xsl:value-of select="//translations/trans[@id='type']"/><xsl:text>: \uglyesc{</xsl:text><xsl:value-of select="@type"/><xsl:text>}\\

    </xsl:text>
    <xsl:apply-templates select="init" mode="vardescr"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="init" mode="vardescr">
    <xsl:text></xsl:text><xsl:value-of select="//translations/trans[@id='initval']"/><xsl:text>: '</xsl:text><xsl:if test="text()!=''"><xsl:text>\uglyesc{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text></xsl:if>'\\
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="sig" mode="vardescr"/>

  <xsl:template match="init"/>
  <xsl:template match="sig"/>
 
  <xsl:template match="descr">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="p">
    <xsl:apply-templates/>
    <xsl:text>

    </xsl:text>
  </xsl:template>

  <xsl:template match="val">
    <xsl:text>\uglyesc{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="hdr">
    <xsl:text>\uglyesc{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="enc">
    <xsl:text>\uglyesc{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="env">
    <xsl:text>\uglyesc{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="tt">
    <xsl:text>\uglyesc{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="b">
    <xsl:text>\textbf{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="em">
    <xsl:text>\textit{</xsl:text><xsl:apply-templates/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="ul">
    <xsl:text>\begin{itemize}
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>\end{itemize}
    </xsl:text>
  </xsl:template>

  <xsl:template match="li">
    <xsl:text>\item </xsl:text><xsl:apply-templates/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="email">
    <xsl:text>\mailto{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="web">
    <xsl:text>\web{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="varref">
    <xsl:text>\varref{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="cmdref">
    <xsl:text>\cmdref{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="funcref">
    <xsl:text>\funcref{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="man">
    <xsl:text>\man</xsl:text><xsl:if test="@sect!=1"><xsl:text>[</xsl:text><xsl:value-of select="@sect"/><xsl:text>]</xsl:text></xsl:if><xsl:text>{</xsl:text><xsl:value-of select="."/><xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="pre">
    <xsl:text>\begin{verbatim}
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>\end{verbatim}
    </xsl:text>
  </xsl:template>

  <!--{{{ _slooooooow_ TeX escaping stolen from W3C

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
          <xsl:when test='$c = "\"'>
            <xsl:text>\textbackslash </xsl:text>
          </xsl:when>
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

  }}} -->

</xsl:stylesheet>
