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
       <secondary><literal><xsl:value-of select="concat('$', @name)"/></literal></secondary>
     </indexterm>
  </xsl:template> 

  <xsl:template match="muttng-doc:hook">
     <literal><link linkend="{concat(@name, '-hook')}"><xsl:value-of select="concat(@name, '-hook')"/></link></literal>
     <indexterm>
       <primary>Hooks</primary>
       <secondary><literal><xsl:value-of select="concat(@name, '-hook')"/></literal></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:command">
     <literal><link linkend="{concat('command-', @name)}"><xsl:value-of select="@name"/></link></literal>
     <indexterm>
       <primary>Configuration Commands</primary>
       <secondary><literal><xsl:value-of select="@name"/></literal></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:pattern">
    <literal>~<xsl:value-of select="@name"/></literal>
    <xsl:if test="@full='1'">/<literal>=<xsl:value-of select="@name"/></literal></xsl:if>
     <indexterm>
       <primary>Patterns</primary>
       <secondary><literal>~<xsl:value-of select="@name"/></literal>/
         <literal>=<xsl:value-of select="@name"/></literal></secondary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:funcdef">
     <literal>&lt;<xsl:value-of select="@name"/>&gt;</literal> (default: <xsl:value-of select="@key"/>)
     <anchor id="func-{@name}"/>
     <indexterm>
       <primary>Functions</primary>
       <secondary><literal>&lt;<xsl:value-of select="@name"/>&gt;</literal></secondary>
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
      <secondary><literal>&lt;<xsl:value-of select="@name"/>&gt;</literal></secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:vardef">
    <sect1 id="{@id}">
      <title><xsl:value-of select="concat('$',@name)"/></title>
      <indexterm>
        <primary>Configuration Variables</primary>
        <secondary><literal>$<xsl:value-of select="@name"/></literal></secondary>
      </indexterm>
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>
 
  <xsl:template match="muttng-doc:varref">
    <xsl:choose>
      <xsl:when test="@link">
        <link linkend="{@link}"><literal><xsl:value-of select="concat('$',@name)"/></literal></link>
      </xsl:when>
      <xsl:otherwise>
        <link linkend="{@name}"><literal><xsl:value-of select="concat('$',@name)"/></literal></link>
      </xsl:otherwise>
    </xsl:choose>
    <indexterm>
      <primary>Configuration Variables</primary>
      <secondary><literal>$<xsl:value-of select="@name"/></literal></secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:web">
    <ulink url="{@url}"><literal>&lt;<xsl:value-of select="@url"/>&gt;</literal></ulink>
  </xsl:template>

  <xsl:template match="muttng-doc:rfc">
    <ulink url="{concat('http://www.faqs.org/rfcs/rfc', @num, '.html')}">RfC <xsl:value-of select="@num"/></ulink>
  </xsl:template>

  <xsl:template match="/">
    <xsl:apply-templates select="*"/>
  </xsl:template>

</xsl:stylesheet>
