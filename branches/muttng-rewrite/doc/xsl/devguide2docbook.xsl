<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="./manual2docbook_tags.xsl"/>

  <xsl:template match="section">
    <chapter id="{@id}">
      <xsl:apply-templates/>
    </chapter>
  </xsl:template>

  <xsl:template match="subsection">
    <sect1 id="{@id}">
      <xsl:apply-templates/>
    </sect1>
  </xsl:template>

</xsl:stylesheet>
