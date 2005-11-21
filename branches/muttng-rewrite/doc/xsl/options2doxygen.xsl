<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text" indent="no" output-encoding="utf-8"/>
  
  <xsl:strip-space elements="*"/>

  <xsl:template match="descriptions">
    <xsl:text>/**
@file muttng/config/vars.h
@brief (AUTO) Option Reference
*/
/**
@page page_options Reference: Configuration Options
    </xsl:text>
    <xsl:apply-templates/>
    <xsl:text>
*/
    </xsl:text>
  </xsl:template>

  <xsl:template match="variable">

    <xsl:text>@section option_</xsl:text><xsl:value-of select="@name"/><xsl:text> $</xsl:text><xsl:value-of select="@name"/><xsl:text>
      Type: @c </xsl:text><xsl:value-of select="@type"/><xsl:text>, Initial value: </xsl:text><xsl:apply-templates select="init"/>
    <xsl:text>

    </xsl:text>
    <xsl:apply-templates select="descr"/>
    <xsl:text>

    </xsl:text>
  </xsl:template>

  <xsl:template match="init">
    <xsl:text>'@c </xsl:text><xsl:value-of select="."/><xsl:text>'</xsl:text>
  </xsl:template>

  <xsl:template match="pre">
    <xsl:text>&lt;pre&gt;</xsl:text>
<xsl:value-of select="."/><xsl:text>&lt;/pre&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="val">
    <xsl:text>@c </xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="li">
    <xsl:text>@c </xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="enc">
    <xsl:text>@c </xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="varref">
    <xsl:text>@ref option_</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="li">
    <xsl:text>- </xsl:text><xsl:value-of select="."/><xsl:text>
    </xsl:text>
  </xsl:template>

  <xsl:template match="man">
    <xsl:choose>
      <xsl:when test="@sect">
        <xsl:text>@c </xsl:text><xsl:value-of select="."/><xsl:text>(</xsl:text><xsl:value-of select="@sect"/><xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>@c </xsl:text><xsl:value-of select="."/><xsl:text>(1)</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="em">
    <xsl:text>&lt;em&gt;</xsl:text><xsl:value-of select="."/><xsl:text>&lt;/em&gt;</xsl:text>
  </xsl:template>
 
</xsl:stylesheet>
