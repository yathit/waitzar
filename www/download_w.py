from locus import FallbackTemplate

class DownloadTemplate(FallbackTemplate):
	template = r'''
        <h1>Download WaitZar</h1>
		<table align="left" border="0" cellpadding="8" cellspacing="0" style="background-color:#444400; border: 6px solid green; color:white;">
			<tr>
				<td width="100" align="center"style="background-color:#FFFF33; border-right: 3px solid green;"><a class="ninjalink" href="http://waitzar.googlecode.com/files/waitzar_release_1.6.zip">
					<img src="img/wz_icon_zipped.png" border="0"/>
					<br><div class="downloadlink">WaitZar_1.6.zip</div>
				</a></td>
				<td valign="top">The latest full release of WaitZar. This zipped folder contains <span style="color:#FF3333;"><b><i>everything you need</i></b></span> to get started: the WaitZar program, the User's Guide, and a sample mywords.txt and config.txt.
								 This realease is fully tested for bugs and misbehaviour. It is considered safe for serious users (e.g., office environments).
				</td>
			</tr>
		</table>

		<br>&nbsp;

		$<hrule_template>



		<h1>Other Downloads</h1>

		The following downloads should be considered "special features" &mdash;unlike the zip file above, you should only download them if you need them.

		<br><table align="left" border="0" cellpadding="8" cellspacing="0" class="glwt mt40">
			<tr>
				<td width="100" align="center" class="glr"><a class="ninjalink" href="http://waitzar.googlecode.com/svn/trunk/release_1.6/WaitZar%20User%27s%20Guide.doc">
					<img src="img/wz_icon_guide.png" border="0"/>
					<br><div class="downloadlink">User's Guide 1.6.exe</div>
				</a></td>
				<td valign="top">The WaitZar User's Guide, which explains how to configure and use the software. Download this if you need any help using WaitZar; it is very informative.
				</td>
			</tr>
		</table>

		<br><table align="left" border="0" cellpadding="8" cellspacing="0" class="glwt mt40">
			<tr>
				<td width="100" align="center" class="glr"><a class="ninjalink" href="http://waitzar.googlecode.com/svn/trunk/nightly/WaitZar.exe">
					<img src="img/wz_icon.png" border="0"/>
					<br><div class="downloadlink">WaitZar.exe</div>
				</a></td>
				<td valign="top">The latest ("nightly") version of WaitZar, including experimental features. Just download it and run it. (Nightly releases are generally stable, and only rarely contain bugs.)
				</td>
			</tr>
		</table>

		<br><table align="left" border="0" cellpadding="8" cellspacing="0" class="glwt mt40">
			<tr>
				<td width="100" align="center" class="glr"><class="ninjalink" href="http://scim-waitzar.googlecode.com/files/scim-waitzar-1.0.0.deb.zip">
					<img src="img/wz_icon_scim.png" border="0"/>
					<br><div class="downloadlink">scim-waitzar 1.0.0.zip</div>
				</a></td>
				<td valign="top">A port of WaitZar to Linux. This will run on any system that supports SCIM. In particular, it has been tested on Debian, Ubuntu, and Fedora.This zip contains several .deb packages for scim-waitzar and libwaitzar; you should install them all at once.
				</td>
			</tr>
		</table>

		<br>&nbsp;

        $<hrule_template>		'''

