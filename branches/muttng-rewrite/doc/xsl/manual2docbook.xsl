<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="./manual2docbook_tags.xsl"/>

  <xsl:template match="section">
    <sect1 id="{@id}">
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>

  <xsl:template match="subsection">
    <sect2 id="{@id}">
      <xsl:apply-templates/>
    </sect2>
  </xsl:template>

  <xsl:template match="subsubsection">
    <sect3 id="{@id}">
      <title><xsl:value-of select="@title"/></title>
      <xsl:apply-templates/>
    </sect3>
  </xsl:template>

</xsl:stylesheet>
