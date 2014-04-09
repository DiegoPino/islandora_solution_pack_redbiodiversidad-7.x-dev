<?xml version="1.0" encoding="UTF-8"?>
<!-- DWC -->
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:foxml="info:fedora/fedora-system:def/foxml#"
xmlns:dcterms="http://purl.org/dc/terms/"
xmlns:dwc="http://rs.tdwg.org/dwc/terms/"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:noNamespaceSchemaLocation="http://rs.tdwg.org/dwc/xsd/simpledarwincore/"
     exclude-result-prefixes="dwc foxml">

	<xsl:template match="foxml:datastream[@ID='DwC']/foxml:datastreamVersion[last()]" name="index_DWC">
	    <xsl:param name="content"/>
	    <xsl:param name="prefix">dwc.</xsl:param>
            <xsl:param name="prefixdc">dcterms.</xsl:param>
	    <xsl:param name="suffix">_ms</xsl:param>
	    
	    <!-- Class Records -->
	    <xsl:for-each select="$content//dwc:Taxon/*">
		    <field>
			    <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
	            </xsl:attribute>
			    <xsl:value-of select="normalize-space(text())" />
		    </field>
	    </xsl:for-each>
	<xsl:for-each select="$content//dwc:Identification/*">
                    <field>
                            <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>
      <xsl:for-each select="$content//dwc:Event/*">
                    <field>
                            <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>
 <xsl:for-each select="$content//dwc:Occurrence/*">
                    <field>
                            <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>

 <xsl:for-each select="$content//dwc:MaterialSample/*">
                    <field>
                            <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>
 <xsl:for-each select="$content//dwc:GeologicalContext/*">
                    <field>
			<xsl:attribute name="name">
			  <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                    	</xsl:when>
			<xsl:otherwise>
			 <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
			</xsl:otherwise></xsl:choose>
			</xsl:attribute>
			
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>





 <xsl:for-each select="$content//dcterms:Location/dwc:*">
                    <field>
                            <xsl:attribute name="name">
    <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>

		<xsl:for-each select="$content//SimpleDarwinRecord/*">
                    <field>
                            <xsl:attribute name="name">
  <xsl:choose><xsl:when test="(substring-before(name(),':')='dwc')">
                        <xsl:value-of select="concat($prefix, substring-after(name(),':'),$suffix)"/>
                        </xsl:when>
                        <xsl:otherwise>
                         <xsl:value-of select="concat($prefixdc, substring-after(name(),':'),$suffix)"/>
                        </xsl:otherwise></xsl:choose>
                    </xsl:attribute>
                            <xsl:value-of select="normalize-space(text())" />
                    </field>
            </xsl:for-each>

    </xsl:template>
</xsl:stylesheet>
