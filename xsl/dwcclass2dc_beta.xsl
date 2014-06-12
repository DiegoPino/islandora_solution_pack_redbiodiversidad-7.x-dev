<?xml version="1.0" encoding="UTF-8"?>

<!--
Diego Pino ... DwC complete class to OAI_DC March 2014.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:dcterms="http://purl.org/dc/terms/"
    xmlns:dwr="http://rs.tdwg.org/dwc/dwcrecord/" xmlns:dwc="http://rs.tdwg.org/dwc/terms/"
    xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
    <xsl:output method="xml" indent="yes"/>
    <xsl:template match="/">
        <oai_dc:dc xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
            xsi:noNamespaceSchemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
            <xsl:apply-templates/>
        </oai_dc:dc>
    </xsl:template>
    
    
    <xsl:template match="/dwr:DarwinRecordSet/dwc:Taxon/child::node()">    
        <xsl:variable name="identificadores">
            <xsl:call-template name="anyID">
            </xsl:call-template>
        </xsl:variable>
    </xsl:template>


    <xsl:template name="anyID">
        <xsl:if test="normalize-space(.)">
        <xsl:variable name="identificador">
            <xsl:call-template name="substring-before-last">
                <xsl:with-param name="input" select="name()"/>
                <xsl:with-param name="substr" select="string('ID')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="normalize-space($identificador)">
            <identifier xmlns="http://purl.org/dc/elements/1.1/">
                <xsl:value-of select="normalize-space(.)"/>
            </identifier>
        </xsl:if>
        </xsl:if>
    </xsl:template>
    
  
    <xsl:template name="substring-before-last">
        <xsl:param name="input"/>
        <xsl:param name="substr"/>
        <xsl:if test="$substr and contains($input, $substr)">
            <xsl:variable name="temp" select="substring-after($input, $substr)"/>
            <xsl:value-of select="substring-before($input, $substr)"/>
            <xsl:if test="contains($temp, $substr)">
                <xsl:value-of select="$substr"/>
                <xsl:call-template name="substring-before-last">
                    <xsl:with-param name="input" select="$temp"/>
                    <xsl:with-param name="substr" select="$substr"/>
                </xsl:call-template>
            </xsl:if>
        </xsl:if>
    </xsl:template>

</xsl:stylesheet>
