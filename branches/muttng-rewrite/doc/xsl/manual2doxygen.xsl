<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text" indent="no"/>

  <xsl:strip-space elements="*"/>

  <xsl:template match="manual">
    <xsl:text>/**
@file manual.h
@brief (AUTO) manual
*/
/**
@page page_manual Mutt-ng Manual

    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
*/
    </xsl:text>
  </xsl:template>

  <xsl:template match="mainpage">
    <xsl:text>/**
@file mainpage.h
@brief (AUTO) mainpage
*/
/**
@mainpage

    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
*/
    </xsl:text>
  </xsl:template>

  <xsl:template match="head"/>

  <xsl:template match="translations"/>

  <xsl:template match="content">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="chapter">
    <xsl:text>@section sect_</xsl:text><xsl:value-of select="@id"/><xsl:text> </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="section">
    <xsl:text>@subsection sect_</xsl:text><xsl:value-of select="@id"/><xsl:text> </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="subsection">
    <xsl:text>@subsubsection sect_</xsl:text><xsl:value-of select="@id"/><xsl:text> </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="subsubsection">
    <xsl:text>@paragraph sect_</xsl:text><xsl:value-of select="@id"/><xsl:text> </xsl:text><xsl:value-of select="@title"/>
    <xsl:apply-templates/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="title">
    <xsl:value-of select="."/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="descriptions">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="listing">
    <xsl:text>@anchor </xsl:text><xsl:value-of select="@id"/><xsl:text>
    </xsl:text>
    <xsl:choose>
      <xsl:when test="@lang='make'">
        <xsl:choose>
          <xsl:when test="@href!=''">
            <xsl:text>@verbinclude </xsl:text><xsl:value-of select="@href"/><xsl:text>
            </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>@verbatim
            </xsl:text>
            <xsl:apply-templates/>
            <xsl:text>
              @endverbatim
            </xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="@lang='muttngrc'">
        <xsl:choose>
          <xsl:when test="@href!=''">
            <xsl:text>@verbinclude </xsl:text><xsl:value-of select="@href"/><xsl:text>
            </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>@verbatim
            </xsl:text>
            <xsl:apply-templates/>
            <xsl:text>
              @endverbatim
            </xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <!--default for all other languages is to use @code..@endcode-->
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="@href!=''">
            <xsl:text>@include </xsl:text><xsl:value-of select="@href"/><xsl:text>
            </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>@code
            </xsl:text>
            <xsl:apply-templates/>
            <xsl:text>
              @endcode
            </xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="inlinelisting">
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:value-of select="."/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="docref">
    <xsl:text>@ref </xsl:text><xsl:value-of select="@href"/><xsl:text> "</xsl:text><xsl:value-of select="."/><xsl:text>" </xsl:text>
  </xsl:template>

  <xsl:template match="context">
    <xsl:text>@subsubsection screen_</xsl:text><xsl:value-of select="@name"/><xsl:text> Screen: </xsl:text><xsl:value-of select="@name"/><xsl:text>
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function">
    <xsl:variable name="transid" select="concat('group-',@group)"/>
    <xsl:text>- &lt;b&gt;&lt;tt&gt;&amp;lt;</xsl:text><xsl:value-of select="@name"/><xsl:text>&amp;gt;&lt;/tt&gt;&lt;/b&gt; (default binding: '@c </xsl:text><xsl:value-of select="@default"/><xsl:text>', group: </xsl:text><xsl:value-of select="//translations/trans[@id=$transid]"/><xsl:text>): </xsl:text><xsl:value-of select="."/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="variable">
    <xsl:text>@subsubsection option_</xsl:text><xsl:value-of select="@name"/><xsl:text> $</xsl:text><xsl:value-of select="@name"/><xsl:text>
      Type: @c </xsl:text><xsl:value-of select="@type"/><xsl:text>&lt;br&gt;</xsl:text>
    <xsl:apply-templates select="init" mode="vardescr"/>
    <xsl:apply-templates select="sig" mode="vardescr"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="init" mode="vardescr">
    <xsl:text>
      Initial value: '@c </xsl:text><xsl:value-of select="."/><xsl:text>'&lt;br&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="sig" mode="vardescr">
    <xsl:text>
      Change signaled: </xsl:text><xsl:value-of select="."/>
    <xsl:text>
    </xsl:text>
  </xsl:template>

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
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="hdr">
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="enc">
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="env">
    <xsl:text>&lt;tt&gt;$</xsl:text><xsl:apply-templates/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="tt">
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="b">
    <xsl:text>&lt;b&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/b&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="ul">
    <xsl:text>&lt;ul&gt;
    </xsl:text>
      <xsl:apply-templates/>
    <xsl:text>
      &lt;/ul&gt;
    </xsl:text>
  </xsl:template>

  <xsl:template match="ol">
    <xsl:text>&lt;ol&gt;
    </xsl:text>
      <xsl:apply-templates/>
    <xsl:text>
      &lt;/ol&gt;
    </xsl:text>
  </xsl:template>

  <xsl:template match="li">
    <xsl:text>&lt;li&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/li&gt;
    </xsl:text>
  </xsl:template>

  <xsl:template match="email">
    <xsl:text>&lt;a href="mailto:</xsl:text><xsl:value-of select="."/><xsl:text>"&gt;&amp;lt;</xsl:text><xsl:value-of select="."/><xsl:text>&amp;gt;&lt;/a&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="web">
    <xsl:text>&lt;a href="</xsl:text><xsl:value-of select="."/><xsl:text>"&gt;</xsl:text><xsl:value-of select="."/><xsl:text>&lt;/a&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="varref">
    <xsl:text>@ref option_</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="cmdref">
    <xsl:text>@ref command_</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="funcref">
    <xsl:text>@ref func_</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="man">
    <xsl:text>&lt;tt&gt;</xsl:text><xsl:value-of select="."/><xsl:text>(</xsl:text><xsl:choose><xsl:when test="@sect"><xsl:value-of select="@sect"/></xsl:when><xsl:otherwise><xsl:text>1</xsl:text></xsl:otherwise></xsl:choose><xsl:text>)&lt;/tt&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="pre">
    <xsl:text>&lt;pre&gt;</xsl:text>
<xsl:apply-templates/><xsl:text>&lt;/pre&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="em">
    <xsl:text>&lt;em&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/em&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="footnote">
    <xsl:text> &lt;em&gt;(</xsl:text><xsl:apply-templates/><xsl:text>)&lt;/em&gt;</xsl:text>
  </xsl:template>

  <!-- tables {{{ -->

  <xsl:template match="cap"/>

  <xsl:template match="cap" mode="tab">
    <xsl:apply-templates/>
  </xsl:template>
  
  <xsl:template match="tab">
    <xsl:text>@anchor </xsl:text><xsl:value-of select="@id"/><xsl:text>
    </xsl:text>
    <xsl:text>@htmlonly
    &lt;p class="title"&gt;</xsl:text>
    <xsl:apply-templates select="cap" mode="tab"/>
    <xsl:text>&lt;/p&gt;</xsl:text>
    <xsl:text>
      &lt;table class="ordinary" rowsep="1" summary="</xsl:text><xsl:apply-templates select="cap" mode="tab"/><xsl:text>"&gt;
    </xsl:text>    
    <xsl:apply-templates/>
    <xsl:text>
      &lt;/table&gt;
      @endhtmlonly
    </xsl:text>
  </xsl:template>

  <xsl:template match="th">
    <xsl:text>&lt;thead&gt;</xsl:text>
      <xsl:apply-templates/>
    <xsl:text>&lt;/thead&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="tb">
    <xsl:text>&lt;tbody&gt;</xsl:text>
      <xsl:apply-templates/>
    <xsl:text>&lt;/tbody&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="tr">
    <xsl:text>&lt;tr&gt;</xsl:text>
      <xsl:apply-templates/>
    <xsl:text>&lt;/tr&gt;
    </xsl:text>
  </xsl:template>

  <xsl:template match="td">
    <xsl:text>&lt;td&gt;</xsl:text>
      <xsl:apply-templates/>
    <xsl:text>&lt;/td&gt;</xsl:text>
  </xsl:template>

  <!-- }}} -->

  <xsl:template match="rfc">
    <xsl:text>&lt;a href="http://www.faqs.org/rfcs/rfc</xsl:text><xsl:value-of select="."/><xsl:text>.html"&gt;RfC </xsl:text><xsl:value-of select="."/><xsl:text>&lt;/a&gt;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
