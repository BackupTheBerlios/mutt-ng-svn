<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="./docbook2html.xsl"/>

  <xsl:template name="user.header.content">
    <span class="tiny">
      <xsl:if test="$l10n.gentext.default.language!='en'">
        <xsl:value-of select="document(concat('./../manual_',$l10n.gentext.default.language,'/trans.xml'))//translations/trans[@id='avail']"/>/</xsl:if><xsl:value-of
        select="document('./../manual_en/trans.xml')//translations/trans[@id='avail']"/>:<br/>
      <a href="http://www.muttng.org/rewrite/en/">English</a> |
      <a href="http://www.muttng.org/rewrite/de/">German</a>
    </span>
  </xsl:template>

</xsl:stylesheet>
