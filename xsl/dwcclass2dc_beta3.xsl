<?xml version="1.0" encoding="UTF-8"?>

<!--
Diego Pino ... DwC class Any to OAI_DC March 2014.
This XSL StyleSheet only processes the first ocurrence of every class
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:dcterms="http://purl.org/dc/terms/"
    xmlns:dwr="http://rs.tdwg.org/dwc/dwcrecord/" xmlns:dwc="http://rs.tdwg.org/dwc/terms/"
    xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
    <xsl:output method="xml" indent="yes"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="/">
        <oai_dc:dc xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
            xsi:noNamespaceSchemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc.xsd">

            <xsl:apply-templates select="/dwr:DarwinRecordSet/dwc:Taxon/dwc:taxonRank" mode="title"/>
            <xsl:apply-templates select="/dwr:DarwinRecordSet/dwc:Occurrence/*"/>
            
        </oai_dc:dc>
    </xsl:template>
    <!--   
Occurrence Class processing
 -->
    <xsl:template match="/dwr:DarwinRecordSet/dwc:Occurrence[1]/child::node()">
        <xsl:if test="normalize-space(.)">
            <xsl:choose>
                <xsl:when test="name()='dwc:institutionCode'">
                    <publisher xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </publisher>
                </xsl:when>
                <xsl:when test="name()='dwc:catalogNumber'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:when test="name()='dwc:recordNumber'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:when test="name()='dwc:otherCatalogNumbers'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="anyID"> </xsl:call-template>
                    <xsl:call-template name="anyRemarks"> </xsl:call-template>
                    <xsl:call-template name="anyBy"> </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>


    <!--   
 Taxon Class processing
 -->
    <xsl:template name="taxonterms" match="/dwr:DarwinRecordSet/dwc:Taxon[1]/child::node()">
        <xsl:if test="normalize-space(.)">
            <xsl:choose>
                <xsl:when test="name()='dwc:nameAccordingTo'">
                    <creator xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </creator>
                </xsl:when>
                <xsl:when test="name()='dwc:scientificName'">
                    <subject xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </subject>
                </xsl:when>
                <xsl:when test="name()='dwc:genus'">
                    <subject xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </subject>
                </xsl:when>
                <xsl:when test="name()='dwc:recordNumber'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:when test="name()='dwc:otherCatalogNumbers'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="anyID"> </xsl:call-template>
                    <xsl:call-template name="anyRemarks"> </xsl:call-template>
                    <xsl:call-template name="anyBy"> </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    <!--   
 Event Class processing
 -->
    <xsl:template match="/dwr:DarwinRecordSet/dwc:Event[1]/child::node()">
        <xsl:if test="normalize-space(.)">
            <xsl:choose>
                <xsl:when test="name()='dwc:institutionCode'">
                    <publisher xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </publisher>
                </xsl:when>
                <xsl:when test="name()='dwc:catalogNumber'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>
                <xsl:when test="name()='dwc:recordNumber'">
                    <identifier xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </identifier>
                </xsl:when>

                <xsl:otherwise>
                    <xsl:call-template name="anyID"> </xsl:call-template>
                    <xsl:call-template name="anyRemarks"> </xsl:call-template>
                    <xsl:call-template name="anyBy"> </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    <!--   
 We need to extract the taxon rank(Taxon Class) to decide which taxon term are we using as title
 -->
    <xsl:template match="dwc:taxonRank" mode="title">
        <xsl:if test="normalize-space(.)">
            <xsl:variable name="taxonstring">
            <xsl:choose>               
                <xsl:when test=".='species'">
                    <xsl:value-of select="'scientificName'"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:apply-templates/>
                </xsl:otherwise>
            </xsl:choose>
            </xsl:variable>
            <xsl:apply-templates select="/*/dwc:Taxon[1]/child::node()" mode="title2">
                <xsl:with-param name="titleelement" select="$taxonstring"/>          
            </xsl:apply-templates>
        </xsl:if>
    </xsl:template>
    <xsl:template name="taxontitle" match="dwc:Taxon[1]/child::node()" mode="title2">
        <xsl:param name="titleelement"/>
        <xsl:if test="normalize-space(.)">
            <xsl:choose>
                <xsl:when test="name()=concat('dwc:',$titleelement)">
                    <title xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:value-of select="* | node()"/>
                    </title>
                </xsl:when>
          <xsl:otherwise>
              <xsl:call-template name="taxonterms"/>
          </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    <xsl:template match="//dcterms:*" mode="recordterms">
        <xsl:if test="normalize-space(.)">
            <xsl:choose>
                <xsl:when test="contains(name() , 'rights')">
                    <rights xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </rights>
                </xsl:when>
                <xsl:when test="name()='dcterms:bibliographicCitation'"> </xsl:when>
                <xsl:when test="name()='dcterms:references'"> </xsl:when>
                <xsl:when test="name()='dcterms:modified'">
                    <date xmlns="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </date>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:element name="{local-name()}" namespace="http://purl.org/dc/elements/1.1/">
                        <xsl:apply-templates select="* | node()"/>
                    </xsl:element>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>

    <xsl:template name="anyID">
        <xsl:variable name="isAtEnd">
            <xsl:call-template name="substring-before-last">
                <xsl:with-param name="input" select="name()"/>
                <xsl:with-param name="substr" select="string('ID')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="normalize-space($isAtEnd)">
            <identifier xmlns="http://purl.org/dc/elements/1.1/">
                <xsl:value-of select="normalize-space(.)"/>
            </identifier>
        </xsl:if>
    </xsl:template>
    <xsl:template name="anyRemarks">
        <xsl:variable name="isAtEnd">
            <xsl:call-template name="substring-before-last">
                <xsl:with-param name="input" select="name()"/>
                <xsl:with-param name="substr" select="string('Remarks')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="normalize-space($isAtEnd)">
            <description xmlns="http://purl.org/dc/elements/1.1/">
                <xsl:value-of select="normalize-space(.)"/>
            </description>
        </xsl:if>
    </xsl:template>
    <xsl:template name="anyBy">
        <xsl:variable name="isAtEnd">
            <xsl:call-template name="substring-before-last">
                <xsl:with-param name="input" select="name()"/>
                <xsl:with-param name="substr" select="string('By')"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="normalize-space($isAtEnd)">
            <creator xmlns="http://purl.org/dc/elements/1.1/">
                <xsl:value-of select="normalize-space(.)"/>
            </creator>
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
