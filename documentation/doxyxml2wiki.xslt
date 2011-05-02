<?xml version="1.0" encoding="UTF-8"?>

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" encoding="UTF-8" indent="no"/>
  <xsl:strip-space elements="*"/>

  <xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

  <!-- use 'contents' to display only a contents list -->
  <xsl:param name="display">documentation</xsl:param>
  <xsl:param name="contents">contents</xsl:param>

  <xsl:template name="tostartcase">
    <xsl:param name="text" select="text()"/>
    <xsl:value-of select="translate( substring( $text, 1, 1 ), $lowercase, $uppercase )"/><xsl:value-of select="translate( substring( $text, 2 ), $uppercase, $lowercase )"/>
  </xsl:template>

  <!-- Formatting for compounds -->
  <xsl:template match="compounddef">
    <xsl:choose>
      <xsl:when test="$display='documentation'">
        <xsl:text>#sidebar </xsl:text><xsl:value-of select="$contents"/><xsl:text>
</xsl:text>
        <xsl:text>#summary </xsl:text><xsl:value-of select="briefdescription/para/text()"/><xsl:text>
</xsl:text>
        <xsl:text>#labels Documentation, </xsl:text><xsl:call-template name="tostartcase">
          <xsl:with-param name="text"><xsl:value-of select="@kind"/></xsl:with-param>
        </xsl:call-template>

= <xsl:value-of select="compoundname/text()"/> =
<xsl:apply-templates select="briefdescription/para"/>

<xsl:if test="@kind='struct' or @kind='class'">
{{{
<xsl:apply-templates select="includes"/><xsl:text>
</xsl:text><xsl:value-of select="@kind"/><xsl:text> </xsl:text><xsl:value-of select="compoundname/text()"/>;
}}}
</xsl:if>
<xsl:apply-templates select="detaileddescription/para"/>
&lt;wiki:toc max_depth="3"/&gt;
<xsl:apply-templates select="sectiondef[@kind='define']"/>
<xsl:apply-templates select="sectiondef[@kind='func']"/>
<xsl:apply-templates select="sectiondef[@kind='public-attrib']"/>
<xsl:apply-templates select="sectiondef[@kind='private-attrib']"/>
<xsl:apply-templates select="sectiondef[@kind='public-func' or @kind='private-func' or @kind='user-defined']"/>
<xsl:apply-templates select="sectiondef[@kind='related']"/>
      </xsl:when>
      <xsl:when test="$display='contents'">
        <xsl:text>  * [</xsl:text><xsl:value-of select="@id"/><xsl:text> </xsl:text><xsl:value-of select="title/text()"/><xsl:text>]
</xsl:text>
        <xsl:for-each select="innerclass">
          <xsl:text>    * [</xsl:text><xsl:value-of select="@refid"/><xsl:text> </xsl:text><xsl:value-of select="text()"/><xsl:text>]
</xsl:text>
        </xsl:for-each>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- Formatting for includes -->
  <xsl:template match="includes">
    <xsl:choose>
      <xsl:when test="@local='no'">
         <xsl:text>#include &lt;</xsl:text><xsl:value-of select="text()"/><xsl:text>&gt;
</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>#include &quot;</xsl:text><xsl:value-of select="text()"/><xsl:text>&quot;
</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Formatting for sections -->
  <xsl:template match="sectiondef">
    <xsl:text>

--------------------------------------------------------------------------------
</xsl:text>
    <xsl:choose>
      <xsl:when test="@kind='user-defined'">== <xsl:value-of select="header/text()"/> ==</xsl:when>
      <xsl:when test="@kind='public-attrib'">== Public Attributes == </xsl:when>
      <xsl:when test="@kind='private-attrib'">== Private Attributes == </xsl:when>
      <xsl:when test="@kind='public-func'">== Public Functions == </xsl:when>
      <xsl:when test="@kind='private-func'">== Private Functions == </xsl:when>
      <xsl:when test="@kind='func'">== Functions == </xsl:when>
      <xsl:when test="@kind='define'">== Defines == </xsl:when>
      <xsl:when test="@kind='related'">== Related == </xsl:when>
      <xsl:otherwise>== <xsl:value-of select="@kind"/> ==</xsl:otherwise>
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

  <!-- Formatting for code & preformatted text -->
  <xsl:template match="preformatted">
    <xsl:text>&lt;pre&gt;</xsl:text><xsl:apply-templates /><xsl:text>&lt;/pre&gt;</xsl:text>
  </xsl:template>

  <!-- Formatting for variables -->
  <xsl:template match="memberdef[@kind='variable']">
    <xsl:text>
</xsl:text>
=== `<xsl:value-of select="definition" />` ===
<xsl:apply-templates select="briefdescription/para"/>
<xsl:apply-templates select="detaileddescription/para"/>
  </xsl:template>

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

  <!-- Formatting for defines -->
  <xsl:template match="memberdef[@kind='define']">
    <xsl:text>

--------------------------------------------------------------------------------
</xsl:text>
=== `<xsl:value-of select="name" />` ===
<xsl:apply-templates select="briefdescription/para"/>
<xsl:choose>
  <xsl:when test="initializer">
    <xsl:text>
{{{
#define </xsl:text><xsl:value-of select="name"/><xsl:text> </xsl:text><xsl:value-of select="initializer"/><xsl:text>;
}}}
</xsl:text>
  </xsl:when>
  <xsl:when test="param">
    <xsl:text>
{{{
</xsl:text><xsl:value-of select="name"/><xsl:text>( </xsl:text><xsl:for-each select="param">
<xsl:value-of select="."/><xsl:if test="name(following-sibling::*[1]) = 'param'"><xsl:text>, </xsl:text></xsl:if>
</xsl:for-each><xsl:text> );
}}}
</xsl:text>
  </xsl:when>
</xsl:choose>
<xsl:apply-templates select="detaileddescription/para"/>
  </xsl:template>

  <!-- Formatting for parameter lists -->
  <xsl:template match="parameterlist[@kind='param']">
    <xsl:text>
==== Parameters ====
</xsl:text>
    <xsl:text>|| *Name* || *Description * ||
</xsl:text>
    <xsl:apply-templates select="parameteritem"/>
  </xsl:template>

  <!-- Formatting for return value lists -->
  <xsl:template match="parameterlist[@kind='retval']">
    <xsl:text>
==== Return Values ====
</xsl:text>
    <xsl:text>|| *Value* || *Description * ||
</xsl:text>
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
      <xsl:when test="@kind='par'">
        <xsl:text>
==== </xsl:text><xsl:value-of select="title/text()" /><xsl:text> ====
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
