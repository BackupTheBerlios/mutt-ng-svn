<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text" indent="no" output-encoding="utf-8"/>
  
  <xsl:strip-space elements="*"/>

  <xsl:template match="descriptions">
    <xsl:text>/**
@file muttng/event/funcs.h
@brief (AUTO) Function Reference
*/
/**
@page page_functions Reference: Functions
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
*/
    </xsl:text>
  </xsl:template>

  <xsl:template match="context">
    <xsl:text>@section screen_</xsl:text><xsl:value-of select="@name"/><xsl:text> Screen: </xsl:text><xsl:value-of select="@name"/><xsl:text>
    </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function">
    <xsl:text>- &lt;b&gt;&lt;code&gt;&amp;lt;</xsl:text><xsl:value-of select="@name"/><xsl:text>&amp;gt;&lt;/code&gt;&lt;/b&gt; (default binding: '@c </xsl:text><xsl:value-of select="@default"/><xsl:text>', group: </xsl:text><xsl:value-of select="@group"/><xsl:text>): </xsl:text><xsl:value-of select="."/><xsl:text>
    </xsl:text>
   
  </xsl:template>
 
</xsl:stylesheet>
