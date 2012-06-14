<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsi="http://www.w3.org/2000/10/XMLSchema-instance">
			<xsl:key name="categorykey" match="snippet" use="category"/>
    <xsl:template match="/">
        <html>
            <head >

<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script>
<style type="text/css" media="screen">
div.codewrap {
margin: 1px;
padding: 3px 10px;
cursor: pointer;
position: relative;
background-color:#eeeeee;
}
div.name {
font-weight:bold;
}
</style>
			</head>
			
			
            <body>
			<script type="text/javascript">
jQuery(document).ready(function() {
  jQuery("div.code").hide();
  //toggle the componenet with class msg_body
  jQuery(".codewrap").click(function()
  {
    jQuery(this).next("div.code").slideToggle(500);
  });
});

</script>
            		<a name="top"><h1>XVR Snippets</h1></a>by Emanuele Ruffaldi 2012, managed with jCodeCollector
            		<hr/>
                    <table border="1" width="">
                    <thead><tr><td></td><td>Name</td><td>Tags</td></tr></thead>
				<xsl:for-each select="//snippet[generate-id(.)=generate-id(key('categorykey', category)[1])]">
                <!--<xsl:for-each select="//snippet">-->
                <xsl:sort select="category"/>
				 <xsl:for-each select="key('categorykey', category)">
                <xsl:sort select="name"/>
                    <tr>
					<xsl:if test="position() = 1">
					  <td valign="center" bgcolor="#EEEEEE">
						<xsl:attribute name="rowspan">
						  <xsl:value-of select="count(key('categorykey', category))"/>
						</xsl:attribute>
						<b>
						  <xsl:text>Category </xsl:text><xsl:value-of select="category"/>
						</b>
					  </td>
					</xsl:if>
                    <td class="name index"><xsl:element name="a"><xsl:attribute name="href">#<xsl:value-of select="name"/></xsl:attribute><xsl:value-of select="name"/></xsl:element></td>
                    <td class="tags index"><xsl:for-each select="tag"><span class="tag"><xsl:value-of select="."/></span></xsl:for-each></td>
					</tr>
                </xsl:for-each>
                </xsl:for-each>
                    </table>
					
					<hr/>
                    <table border="1" width="">
                    <thead><tr><td>Name</td><td>Category</td><td>Tags</td><td>Code</td></tr></thead>
                <xsl:for-each select="//snippet">
                <xsl:sort select="category"/>
                <xsl:sort select="name"/>
                    <tr>
                    <td class="name"><xsl:element name="a"><xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute><xsl:value-of select="name"/></xsl:element></td>
                    <td class="category"><xsl:value-of select="category"/></td>
					<td class="tags"><xsl:for-each select="tag"><span class="tag"><xsl:value-of select="."/></span></xsl:for-each></td>
                    <td class="code"><div class="codewrap">Code</div><div class="code brush: js"><pre><xsl:value-of select="code"/></pre></div></td>
					</tr>

                </xsl:for-each>
                    </table>
            		<hr/>
            		<i>Contact: <a href="mailto:pit@sssup.it">Emanuele Ruffaldi</a></i>
                </body>
                </html>
                </xsl:template>
                </xsl:stylesheet>
