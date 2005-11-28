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

  <xsl:template match="title">
    <xsl:value-of select="."/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="descriptions">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="context">
    <xsl:text>@subsubsection screen_</xsl:text><xsl:value-of select="@name"/><xsl:text> Screen: </xsl:text><xsl:value-of select="@name"/><xsl:text>
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function">
    <xsl:text>- &lt;b&gt;&lt;code&gt;&amp;lt;</xsl:text><xsl:value-of select="@name"/><xsl:text>&amp;gt;&lt;/code&gt;&lt;/b&gt; (default binding: '@c </xsl:text><xsl:value-of select="@default"/><xsl:text>', group: </xsl:text><xsl:value-of select="@group"/><xsl:text>): </xsl:text><xsl:value-of select="."/><xsl:text>
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
    <xsl:text>&lt;code&gt; </xsl:text><xsl:apply-templates/><xsl:text>&lt;/code&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="hdr">
    <xsl:text>&lt;code&gt; </xsl:text><xsl:apply-templates/><xsl:text>&lt;/code&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="enc">
    <xsl:text>&lt;code&gt; </xsl:text><xsl:apply-templates/><xsl:text>&lt;/code&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="tt">
    <xsl:text>&lt;code&gt; </xsl:text><xsl:apply-templates/><xsl:text>&lt;/code&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="b">
    <xsl:text>&lt;b&gt; </xsl:text><xsl:apply-templates/><xsl:text>&lt;/b&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="ul">
    <xsl:text>&lt;ul&gt;
    </xsl:text>
      <xsl:apply-templates/>
    <xsl:text>
      &lt;/ul&gt;
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
    <xsl:text>@c </xsl:text><xsl:value-of select="."/><xsl:text>(</xsl:text><xsl:choose><xsl:when test="@sect"><xsl:value-of select="@sect"/></xsl:when><xsl:otherwise><xsl:text>1</xsl:text></xsl:otherwise></xsl:choose><xsl:text>)</xsl:text>
  </xsl:template>

  <xsl:template match="pre">
    <xsl:text>&lt;pre&gt;</xsl:text>
<xsl:apply-templates/><xsl:text>&lt;/pre&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="em">
    <xsl:text>&lt;em&gt;</xsl:text><xsl:apply-templates/><xsl:text>&lt;/em&gt;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
