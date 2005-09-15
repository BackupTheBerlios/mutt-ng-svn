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
    doctype-public="-//OASIS//DTD DocBook XML V4.3//EN"
    doctype-system="http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd"
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
       <primary sortas="{@name}"><literal>$<xsl:value-of select="@name"/></literal></primary>
     </indexterm>
  </xsl:template> 

  <xsl:template match="muttng-doc:hook">
     <literal><link linkend="{concat(@name, '-hook')}"><xsl:value-of select="concat(@name, '-hook')"/></link></literal>
     <indexterm>
       <primary sortas="{@name}-hook"><literal><xsl:value-of select="concat(@name, '-hook')"/></literal></primary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:cmddef">
    <xsl:choose>
      <xsl:when test="@noanchor='1'"/>
      <xsl:otherwise>
        <anchor id="command-{translate(@name,'_','-')}"/>
      </xsl:otherwise>
    </xsl:choose>
    Usage: <literal><xsl:value-of select="@name"/></literal> <emphasis><xsl:apply-templates/></emphasis>
    <indexterm>
      <primary sortas="{@name}"><literal><xsl:value-of select="@name"/></literal></primary>
     </indexterm>
   </xsl:template>

   <xsl:template match="muttng-doc:cmdref">
     <link linkend="command-{translate(@name,'_','-')}"><literal><xsl:value-of select="@name"/></literal></link>
     <indexterm>
       <primary sortas="{@name}"><literal><xsl:value-of select="@name"/></literal></primary>
     </indexterm>
   </xsl:template>
 
  <xsl:template match="muttng-doc:pattern">
    <literal>~<xsl:value-of select="@name"/></literal>
    <xsl:if test="@full='1'">/<literal>=<xsl:value-of select="@name"/></literal></xsl:if>
     <indexterm>
       <primary sortas="~{@name}"><literal>~<xsl:value-of select="@name"/></literal>/
         <literal>=<xsl:value-of select="@name"/></literal></primary>
     </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:funcdef">
     <literal>&lt;<xsl:value-of select="@name"/>&gt;</literal> (default: <xsl:apply-templates/>)
     <anchor id="func-{@name}"/>
     <indexterm>
       <primary sortas="{@name}"><literal>&lt;<xsl:value-of select="@name"/>&gt;</literal></primary>
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
      <primary sortas="{@name}"><literal>&lt;<xsl:value-of select="@name"/>&gt;</literal></primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:vardef">
    <sect1 id="{translate(@name,'_','-')}">
      <title><xsl:value-of select="concat('$',@name)"/></title>
      <indexterm>
        <primary sortas="{@name}"><literal>$<xsl:value-of select="@name"/></literal></primary>
      </indexterm>
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>
 
  <xsl:template match="muttng-doc:varref">
    <link linkend="{@name}"><literal><xsl:value-of select="concat('$',translate(@name,'-','_'))"/></literal></link><indexterm>
      <primary sortas="{@name}"><literal>$<xsl:value-of select="translate(@name,'-','_')"/></literal></primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:web">
    <ulink url="{@url}"><literal>&lt;<xsl:value-of select="@url"/>&gt;</literal></ulink>
  </xsl:template>

  <xsl:template match="muttng-doc:rfc">
    <ulink url="{concat('http://www.faqs.org/rfcs/rfc', @num, '.html')}">RfC <xsl:value-of select="@num"/></ulink>
    <indexterm><primary>RfC</primary><secondary><xsl:value-of select="@num"/></secondary></indexterm>
  </xsl:template>

  <xsl:template match="muttng-doc:lstconf">
    <screen>
      <xsl:apply-templates/>
    </screen>
  </xsl:template>

  <xsl:template match="muttng-doc:lstmail">
    <screen>
      <xsl:apply-templates/>
    </screen>
  </xsl:template>

  <xsl:template match="muttng-doc:lstshell">
    <screen>
      <xsl:apply-templates/>
    </screen>
  </xsl:template>

  <xsl:template match="muttng-doc:key">
    <literal>
      <xsl:if test="@mod">
        <xsl:value-of select="concat(@mod,'-')"/>
      </xsl:if>
      <xsl:apply-templates/>
    </literal>
  </xsl:template>

  <xsl:template match="muttng-doc:special">
    <xsl:value-of select="@docbook"/>
  </xsl:template>

</xsl:stylesheet>
