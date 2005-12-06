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
      <literal>&lt;<xsl:value-of select="@name"/>&gt;</literal> (<xsl:value-of select="//translations/trans[@id='defbind']"/>: '<literal><xsl:value-of select="@default"/></literal>', <xsl:value-of select="//translations/trans[@id='group']"/>: <xsl:value-of select="//translations/trans[@id=$transid]"/>): <xsl:value-of select="text()"/>
    </listitem>
  </xsl:template>

  <xsl:template match="variable">
    <xsl:variable name="transid" select="concat('option-',@type)"/>
    <sect2 id="option-{translate(@name,'_','-')}">
      <title><literal>$<xsl:value-of select="@name"/></literal></title>
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
    <xsl:value-of select="//translations/trans[@id='initval']"/>: '<literal><xsl:value-of select="."/></literal>'
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
    <literal><xsl:apply-templates/></literal>
  </xsl:template>

  <xsl:template match="tt">
    <literal><xsl:apply-templates/></literal>
  </xsl:template>

  <xsl:template match="ul">
    <itemizedlist>
      <xsl:apply-templates/>
    </itemizedlist>
  </xsl:template>

  <xsl:template match="ol">
    <orderedlist>
      <xsl:apply-templates/>
    </orderedlist>
  </xsl:template>

  <xsl:template match="li">
    <listitem>
      <xsl:apply-templates/>
    </listitem>
  </xsl:template>

  <xsl:template match="varref">
    <link linkend="option-{translate(text(),'_','-')}">
      <literal>$<xsl:value-of select="."/></literal></link>
    <indexterm>
      <primary sortas="{text()}"><literal>$<xsl:value-of select="."/></literal></primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="docref">
    <link linkend="{@href}"><xsl:value-of select="."/></link>
  </xsl:template>

  <xsl:template match="web">
    <ulink url="{text()}"><xsl:value-of select="."/></ulink>
  </xsl:template>

  <xsl:template match="cmdref">
    <link linkend="cmd-{translate(text(),'_','-')}">
      <literal><xsl:value-of select="."/></literal></link>
    <indexterm>
      <primary><literal><xsl:value-of select="."/></literal></primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="funcref">
    <link linkend="func-{translate(text(),'_','-')}">
      <literal>&lt;<xsl:value-of select="."/>&gt;</literal></link>
    <indexterm>
      <primary sortas="{translate(text(),'_','-')}">
        <literal>&lt;<xsl:value-of select="."/>&gt;</literal>
      </primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="man">
    <literal><xsl:value-of select="."/>(<xsl:choose><xsl:when test="@sect"><xsl:value-of select="@sect"/></xsl:when><xsl:otherwise>1</xsl:otherwise></xsl:choose>)</literal>
    <indexterm>
      <primary sortas="{text()}"><literal><xsl:value-of select="."/>(<xsl:choose><xsl:when test="@sect"><xsl:value-of select="@sect"/></xsl:when><xsl:otherwise>1</xsl:otherwise></xsl:choose>)</literal></primary>
    </indexterm>
  </xsl:template>

  <xsl:template match="pre">
    <screen>
<xsl:apply-templates/></screen>
  </xsl:template>

  <!--  <xsl:template match="listing[@lang='cpp']"> -->
  <xsl:template match="listing">
    <example>
      <anchor id="{@id}"/>
      <title><xsl:value-of select="@title"/></title>
      <programlisting><xi:include  href="{concat('./../examples/',@href)}" parse="text" xmlns:xi="http://www.w3.org/2001/XInclude"/></programlisting>
    </example>
  </xsl:template>

  <xsl:template match="inlinelisting">
    <literal><xsl:apply-templates/></literal>
  </xsl:template>

  <xsl:template match="em">
    <emphasis><xsl:apply-templates/></emphasis>
  </xsl:template>

  <xsl:template match="b">
    <emphasis role="bold"><xsl:apply-templates/></emphasis>
  </xsl:template>

  <xsl:template match="enc">
    <literal><xsl:value-of select="text()"/></literal>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='encodings']"/></primary>
      <secondary sortas="{text()}">
        <literal><xsl:value-of select="text()"/></literal>
      </secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="env">
    <literal>$<xsl:value-of select="text()"/></literal>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='environ']"/></primary>
      <secondary sortas="{text()}">
        <literal>$<xsl:value-of select="text()"/></literal>
      </secondary>
    </indexterm>
  </xsl:template>

  <xsl:template match="hdr">
    <literal><xsl:value-of select="text()"/></literal>
    <indexterm>
      <primary><xsl:value-of select="//translations/trans[@id='hdr']"/></primary>
      <secondary sortas="{text()}">
        <literal><xsl:value-of select="text()"/></literal>
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

  <!-- tables {{{ -->
  
  <xsl:template match="tab">
    <anchor id="{@id}"/>
    <table rowsep="1" frame="none">
      <xsl:apply-templates select="cap" mode="tab"/>
      <tgroup cols="{@cols}" align="left" colsep="1" rowsep="1">
        <xsl:apply-templates/>
      </tgroup>
    </table>
  </xsl:template>

  <xsl:template match="th">
    <thead>
      <xsl:apply-templates/>
    </thead>
  </xsl:template>

  <xsl:template match="cap"/>

  <xsl:template match="cap" mode="tab">
    <title>
      <xsl:apply-templates/>
    </title>
  </xsl:template>

  <xsl:template match="tb">
    <tbody>
      <xsl:apply-templates/>
    </tbody>
  </xsl:template>

  <xsl:template match="tr">
    <row>
      <xsl:apply-templates/>
    </row>
  </xsl:template>

  <xsl:template match="td">
    <entry>
      <xsl:apply-templates/>
    </entry>
  </xsl:template>

  <!-- }}} -->

</xsl:stylesheet>
