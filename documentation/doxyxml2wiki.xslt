<?xml version="1.0" encoding="UTF-8"?>

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" encoding="UTF-8" indent="no"/>
  <xsl:strip-space elements="*"/>

  <xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

  <xsl:template name="tostartcase">
    <xsl:param name="text" select="text()"/>
    <xsl:value-of select="translate( substring( $text, 1, 1 ), $lowercase, $uppercase )"/><xsl:value-of select="translate( substring( $text, 2 ), $uppercase, $lowercase )"/>
  </xsl:template>

  <!-- Formatting for compounds -->
  <xsl:template match="compounddef">
    <xsl:text>#summary </xsl:text><xsl:value-of select="briefdescription/para/text()"/>
    <xsl:text>
</xsl:text>
    <xsl:text>#labels Documentation, </xsl:text><xsl:call-template name="tostartcase">
      <xsl:with-param name="text"><xsl:value-of select="@kind"/></xsl:with-param>
    </xsl:call-template><xsl:text>, </xsl:text><xsl:call-template name="tostartcase">
      <xsl:with-param name="text"><xsl:value-of select="@prot"/></xsl:with-param>
    </xsl:call-template>

= <xsl:value-of select="compoundname/text()"/> =

&lt;wiki:toc max_depth="3"/&gt;

<xsl:apply-templates select="sectiondef"/>
  </xsl:template>

  <!-- Formatting for sections -->
  <xsl:template match="sectiondef">
    <xsl:text>

--------------------------------------------------------------------------------
</xsl:text>
    <xsl:choose>
      <xsl:when test="@kind='user-defined'">== <xsl:value-of select="header/text()"/> ==</xsl:when>
      <xsl:when test="@kind='public-attrib'">== Public Attributes == </xsl:when>
      <xsl:when test="@kind='private-func'">== Private Functions == </xsl:when>
      <xsl:otherwise>== unknown ? ==</xsl:otherwise>
    </xsl:choose>
    <xsl:text>
</xsl:text>
    <xsl:apply-templates select="description/para"/>

    <xsl:apply-templates select="memberdef"/>
  </xsl:template>

  <!-- Formatting for paragraphs -->
  <xsl:template match="para">
<xsl:apply-templates /><xsl:if test="name(following-sibling::*[1])='para'"><xsl:text>

</xsl:text></xsl:if>
  </xsl:template>

  <!-- Formatting for references -->
  <xsl:template match="ref">[<xsl:value-of select="@refid" /><xsl:text> </xsl:text><xsl:apply-templates />]</xsl:template>

  <!-- Formatting for types, declnames, computeroutput (@c) -->
  <xsl:template match="type|declname|computeroutput">`<xsl:apply-templates />`</xsl:template>

  <!-- Formatting for functions -->
  <xsl:template match="memberdef[@kind='function']">
    <xsl:text>

--------------------------------------------------------------------------------
</xsl:text>
=== `<xsl:value-of select="definition" />` ===
<xsl:apply-templates select="briefdescription/para"/>
{{{
<xsl:value-of select="definition"/><xsl:value-of select="argsstring"/>;
}}}

<xsl:apply-templates select="detaileddescription/para"/>
  </xsl:template>

  <!-- Formatting for parameter lists lists -->
  <xsl:template match="parameterlist">
    <xsl:choose>
      <xsl:when test="@kind='param'">
        <xsl:text>
==== Parameters ====
</xsl:text>
        <xsl:text>|| *Name* || *Description * ||
</xsl:text>
      </xsl:when>
      <xsl:when test="@kind='retval'">
        <xsl:text>
==== Return Values ====
</xsl:text>
        <xsl:text>|| *Value* || *Description * ||
</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:apply-templates select="parameteritem"/>
  </xsl:template>

  <!-- Formatting for returns -->
  <xsl:template match="simplesect">
    <xsl:choose>
      <xsl:when test="@kind='return'">
        <xsl:text>
==== Return ====
</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:apply-templates select="para"/>
  </xsl:template>

  <!-- Formatting for parameteritems -->
  <xsl:template match="parameteritem">
    <xsl:text>|| </xsl:text><xsl:value-of select="parameternamelist/parametername" /><xsl:text> || </xsl:text><xsl:apply-templates select="parameterdescription/para" /><xsl:text> ||
</xsl:text>
  </xsl:template>
</xsl:transform>
