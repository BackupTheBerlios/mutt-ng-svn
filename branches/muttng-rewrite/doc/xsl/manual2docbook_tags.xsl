<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output 
    method="xml" 
    doctype-public="-//OASIS//DTD DocBook XML V4.3//EN"
    doctype-system="http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd"
    indent="yes"
    />

  <xsl:strip-space elements="*"/>

  <!-- as default, copy each node -->
  <xsl:template match="/ | node() | @* | comment() | processing-instruction()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="manual">
    <book>
      <xsl:apply-templates select="head"/>
      <xsl:apply-templates select="content"/>
    </book>
  </xsl:template>

  <xsl:template match="mainpage">
    <book>
      <xsl:apply-templates select="head"/>
      <xsl:apply-templates select="content"/>
    </book>
  </xsl:template>

  <xsl:template match="head">
    <bookinfo>
      <xsl:apply-templates select="title" mode="head"/>
      <xsl:apply-templates select="authors" mode="head"/>
    </bookinfo>
  </xsl:template>

  <xsl:template match="title" mode="head">
    <title><xsl:value-of select="."/></title>
  </xsl:template>

  <xsl:template match="authors" mode="head">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="content">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="descriptions">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="context">
    <xsl:variable name="transid" select="concat('screen-',@name)"/>
    <sect2 id="funcref-screen-{@name}">
      <title><xsl:value-of select="//translations/trans[@id='screen']"/>: <xsl:value-of select="//translations/trans[@id=$transid]"/></title>
      <itemizedlist>
        <xsl:apply-templates select="function"/>
      </itemizedlist>
    </sect2>
  </xsl:template>

  <xsl:template match="function">
    <xsl:variable name="transid" select="concat('group-',@group)"/>
    <listitem>
      <anchor id="func-{@name}"/>
      <code>&lt;<xsl:value-of select="@name"/>&gt;</code> (<xsl:value-of select="//translations/trans[@id='defbind']"/>: '<code><xsl:value-of select="@default"/></code>', <xsl:value-of select="//translations/trans[@id='group']"/>: <xsl:value-of select="//translations/trans[@id=$transid]"/>): <xsl:value-of select="text()"/>
    </listitem>
  </xsl:template>

  <xsl:template match="variable">
    <xsl:variable name="transid" select="concat('option-',@type)"/>
    <sect2 id="option-{translate(@name,'_','-')}">
      <title><code>$<xsl:value-of select="@name"/></code></title>
      <para>
        <xsl:value-of select="//translations/trans[@id='type']"/>: <xsl:value-of select="//translations/trans[@id=$transid]"/>
      </para>
      <para>
        <xsl:apply-templates select="init" mode="vardescr"/>
      </para>
      <para>
        <xsl:apply-templates/>
      </para>
    </sect2>
  </xsl:template>

  <xsl:template match="init" mode="vardescr">
    <xsl:value-of select="//translations/trans[@id='initval']"/>: '<code><xsl:value-of select="."/></code>'
  </xsl:template>

  <xsl:template match="init"/>

  <xsl:template match="sig"/>

  <xsl:template match="translations"/>

  <xsl:template match="descr">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="p">
    <para>
      <xsl:apply-templates/>
    </para>
  </xsl:template>

  <xsl:template match="val">
    <code><xsl:apply-templates/></code>
  </xsl:template>

  <xsl:template match="tt">
    <code><xsl:apply-templates/></code>
  </xsl:template>

  <xsl:template match="ul">
    <itemizedlist>
      <xsl:apply-templates/>
    </itemizedlist>
  </xsl:template>

  <xsl:template match="li">
    <listitem>
      <xsl:apply-templates/>
    </listitem>
  </xsl:template>

  <xsl:template match="varref">
    <link linkend="option-{translate(text(),'_','-')}">
      <code>$<xsl:value-of select="."/></code></link>
  </xsl:template>

  <xsl:template match="docref">
    <link linkend="{@href}"><xsl:value-of select="."/></link>
  </xsl:template>

  <xsl:template match="web">
    <ulink url="{text()}"><xsl:value-of select="."/></ulink>
  </xsl:template>

  <xsl:template match="cmdref">
    <link linkend="cmd-{translate(text(),'_','-')}">
      <code><xsl:value-of select="."/></code></link>
  </xsl:template>

  <xsl:template match="funcref">
    <link linkend="func-{translate(text(),'_','-')}">
      <code>&lt;<xsl:value-of select="."/>&gt;</code></link>
  </xsl:template>

  <xsl:template match="man">
    <code><xsl:value-of select="."/>(<xsl:choose><xsl:when test="@sect"><xsl:value-of select="@sect"/></xsl:when><xsl:otherwise>1</xsl:otherwise></xsl:choose>)</code>
  </xsl:template>

  <xsl:template match="pre">
    <screen>
<xsl:apply-templates/></screen>
  </xsl:template>

  <xsl:template match="listing[@lang='cpp']">
    <example>
      <anchor id="{@id}"/>
      <title><xsl:value-of select="@title"/></title>
      <programlisting><xi:include  href="{concat('./../examples/',@href)}" parse="text" xmlns:xi="http://www.w3.org/2001/XInclude"/></programlisting>
    </example>
  </xsl:template>

  <xsl:template match="em">
    <emphasis><xsl:apply-templates/></emphasis>
  </xsl:template>

  <xsl:template match="b">
    <emphasis role="bold"><xsl:apply-templates/></emphasis>
  </xsl:template>

  <xsl:template match="enc">
    <code><xsl:value-of select="text()"/></code>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='encodings']"/></primary>
      <secondary sortas="{text()}">
        <code><xsl:value-of select="text()"/></code>
      </secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="env">
    <code>$<xsl:value-of select="text()"/></code>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='environ']"/></primary>
      <secondary sortas="{text()}">
        <code>$<xsl:value-of select="text()"/></code>
      </secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="hdr">
    <code><xsl:value-of select="text()"/></code>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='hdr']"/></primary>
      <secondary sortas="{text()}">
        <code><xsl:value-of select="text()"/></code>
      </secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="acknowledgements">
    <chapter id="acknowledgements">
      <title>
        Acknowledgements
      </title>
      <xsl:apply-templates select="mutt" mode="ack"/>
      <xsl:apply-templates select="muttng" mode="ack"/>
    </chapter>
  </xsl:template>

  <xsl:template match="mutt" mode="ack">
    <sect1 id="ack-mutt">
      <title>Mutt</title>
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>

  <xsl:template match="muttng" mode="ack">
    <sect1 id="ack-mutt-ng">
      <title>Mutt-ng</title>
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>

</xsl:stylesheet>
