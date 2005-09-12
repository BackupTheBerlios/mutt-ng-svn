<?xml version="1.0" encoding="utf-8"?>

<!-- written for mutt-ng by: -->
<!-- Felix Meinhold <felix.meinhold@gmx.net> -->

<!-- some more things were added/ajusted by: -->
<!-- Rocco Rutte <pdmef@cs.tu-berlin.de> -->

<!-- see "Hacking Documentation" chapter of the Fine Manual for -->
<!-- usage details -->
  
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:muttng-doc="http://mutt-ng.berlios.de/doc/#NS"
  >
  
  <xsl:output 
    method="xml" 
    doctype-public="-//OASIS//DTD DocBook XML V4.2//EN"
    doctype-system="http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd"
    indent="yes"
    output-encoding="utf-8"
  />
  
  <xsl:strip-space elements="*"/>

 
  <!-- as default, copy each node -->
  <xsl:template match="/ | node() | @* | comment() | processing-instruction()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="muttng-doc:man">
    <literal>
      <xsl:choose>
        <xsl:when test="@sect">
          <xsl:value-of select="concat(@name,'(',@sect,')')"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat(@name,'(1)')"/>
        </xsl:otherwise>
      </xsl:choose>
    </literal>
  </xsl:template>
 
  <xsl:template match="muttng-doc:envvar">
     <literal><xsl:value-of select="concat('$', @name)"/></literal>
     <indexterm>
       <primary>Environment Variables</primary>
       <secondary><xsl:value-of select="concat('$', @name)"/></secondary>
     </indexterm>
  </xsl:template> 

  <xsl:template match="muttng-doc:hook">
     <literal><xsl:value-of select="concat(@name, '-hook')"/></literal>
     <indexterm>
       <primary>Hooks</primary>
       <secondary><xsl:value-of select="concat(@name, '-hook')"/></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:command">
     <literal><xsl:value-of select="@name"/></literal>
     <indexterm>
       <primary>Configuration Commands</primary>
       <secondary><xsl:value-of select="@name"/></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:pattern">
     <literal>~<xsl:value-of select="@name"/>/=<xsl:value-of select="@name"/></literal>
     <indexterm>
       <primary>Patterns</primary>
       <secondary>~<xsl:value-of select="@name"/>/=<xsl:value-of select="@name"/></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:funcdef">
     <literal>&lt;<xsl:value-of select="@name"/>&gt;</literal> (default: <xsl:value-of select="@key"/>)
     <anchor id="func-{@name}"/>
     <indexterm>
       <primary>Functions</primary>
       <secondary><xsl:value-of select="@name"/></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:funcref">
    <!-- currently b0rken as not all defined via :funcdef yet ;-((
    <link linkend="func-{@name}">
    -->
      <xsl:value-of select="concat('&lt;',@name,'&gt;')"/>
    <!--
    </link>
    -->
    <indexterm>
      <primary>Functions</primary>
      <secondary><xsl:value-of select="@name"/></secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:vardef">
    <sect1 id="{@id}">
      <title><xsl:value-of select="concat('$',@name)"/></title>
      <indexterm>
        <primary>Configuration Variables</primary>
        <secondary><xsl:value-of select="@name"/></secondary>
      </indexterm>
      <xsl:value-of select="text()"/>
    </sect1>
  </xsl:template>
 
  <xsl:template match="muttng-doc:varref">
    <link>
      <xsl:choose>
        <xsl:when test="@link">
          <xsl:param name="linkend" value="@link"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:param name="linkend" value="@name"/>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:value-of select="concat('$',@name)"/>
    </link>
    <indexterm>
      <primary>Configuration Variables</primary>
      <secondary><xsl:value-of select="@name"/></secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="/">
    <xsl:apply-templates select="*"/>
  </xsl:template>

</xsl:stylesheet>
