<?xml version="1.0" encoding="UTF-8"?>
<!-- DWC -->
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:foxml="info:fedora/fedora-system:def/foxml#"
xmlns:dcterms="http://purl.org/dc/terms/"  xmlns:dwr="http://rs.tdwg.org/dwc/xsd/simpledarwincore/"
xmlns:dwc="http://rs.tdwg.org/dwc/terms/"
     exclude-result-prefixes="dwc foxml">

	<xsl:template match="foxml:datastream[@ID='DwC']/foxml:datastreamVersion[last()]" name="index_DWC">
	    <xsl:param name="content"/>
	    <xsl:param name="prefix">dwc.</xsl:param>
            <xsl:param name="prefixdc">dcterms.</xsl:param>
	    <xsl:param name="suffix">_s</xsl:param>
	    
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





 <xsl:for-each select="$content//dcterms:Location/*">
                    <xsl:choose>
			<xsl:when test="name()='dwc:decimalLongitude'">
<field> 
<xsl:attribute name="name">
<xsl:value-of select="concat($prefix, 'latlong','_p')"/>
</xsl:attribute>
	<xsl:value-of select="../dwc:decimalLatitude"/>
				<xsl:text>,</xsl:text>
			<xsl:value-of select="normalize-space(text())" />
</field>
			</xsl:when>
			<xsl:otherwise>
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
            </xsl:otherwise>
</xsl:choose>            
</xsl:for-each>




		<xsl:for-each select="$content//dwr:SimpleDarwinRecordSet[1]/dwr:SimpleDarwinRecord[1]/*">
 <xsl:choose>
                        <xsl:when test="name()='dwc:decimalLongitude'">
<field> 
<xsl:attribute name="name">
<xsl:value-of select="concat($prefix, 'latlong','_p')"/>
</xsl:attribute>
        <xsl:value-of select="../dwc:decimalLatitude"/>
                                <xsl:text>,</xsl:text>
                        <xsl:value-of select="normalize-space(text())" />
</field>
                        </xsl:when>
                        <xsl:otherwise>

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
 </xsl:otherwise>
</xsl:choose>

            </xsl:for-each>

    </xsl:template>
</xsl:stylesheet>
