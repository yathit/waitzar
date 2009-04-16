from locus import FallbackTemplate

class GetinvolvedTemplate(FallbackTemplate):
	template = r'''
        <h1>Get Involved</h1>

        WaitZar is a community project, and it wouldn't be possible without everyone's help. Since WaitZar is a free program, we won't ask for your money; however, we would be happy for you to donate a few minutes of your time to help us out.

        $<hrule_template>

        <h1>How Can I Help?</h1>

		<table border="0" cellpadding="00" cellspacing="0" style="margin-bottom:20px; margin-top:10px;">
			<tr>
				<td width="1" valign="top" align="left"  style="padding-left:10px; padding-top:10px;">
					<a style="text-decoration: none;" href="http://code.google.com/p/waitzar/issues/list">
						<img src="img/wz_bug.png" border="0"/>
					</a>
				</td>
				<td width="100%" valign="top" align="left"  style="padding-top:10px; padding-left: 10px;">
					<a style="" href="http://code.google.com/p/waitzar/issues/list">
						<span style="font-size: 1.25em; color: #00CCFF;">Report a Bug / Ask for a Feature</span>
					</a>
				</td>
			</tr>
			<tr>
				<td colspan="2" style="color:white; padding-left: 10px; padding-bottom:10px; padding-top: 8px;">
					WaitZar is well-tested, but that doesn't mean there are no bugs. If you find a bug white using WaitZar, please click the link above and post a "New Issue" describing your problem in great detail. Similarly, you might be using WaitZar and say "Gee, I wish I could (do something)" &mdash;if so, please post an Issue describing your feature request. Most of WaitZar's best features were suggested by the community, including the "Help" keyboard, cursor tracking, and even the custom dictionary "mywords.txt". We'd love to hear your suggestions.
				</td>
			</tr>
		</table>

		<table border="0" cellpadding="00" cellspacing="0" style="margin-bottom:20px; margin-top:10px;">
			<tr>
				<td width="1" valign="top" align="left"  style="padding-left:10px; padding-top:10px;">
					<a style="text-decoration: none;" href="http://waitzar.googlecode.com/files/wz_usericon.png">
						<img src="http://waitzar.googlecode.com/files/wz_usericon.png" border="0"/>
					</a>
				</td>
				<td width="100%" valign="top" align="left"  style="padding-top:10px; padding-left: 10px;">
					<a style="" href="http://waitzar.googlecode.com/files/wz_usericon.png">
						<span style="font-size: 1.25em; color: #00CCFF;">Use the WaitZar Blog Sticker</span>
					</a>
				</td>
			</tr>
			<tr>
				<td colspan="2" style="color:white; padding-left: 10px; padding-bottom:10px; padding-top: 8px;">
					If you find WaitZar useful, maybe other people would too? Put the WaitZar blog sticker in your forum signature, on your blog, or anywhere else that's relevant, and people who browse to your page will see it and try WaitZar themselves. You can use the following HTML code to link in the sticker:
					<code style="color:#88FF99; font-size:1.2em;">
					<br>&nbsp;<br>&lt;a href="http://www.waitzar.com/"&gt;
					<br>&nbsp;&nbsp;&nbsp;&lt;img border="0" src="http://waitzar.googlecode.com/files/wz_usericon.png"/&gt;
					<br>&lt;/a&gt;
					</code>
				</td>
			</tr>
		</table>

		<table border="0" cellpadding="00" cellspacing="0" style="margin-bottom:20px; margin-top:10px;">
			<tr>
				<td width="1" valign="top" align="left"  style="padding-left:10px; padding-top:10px;">
					<a style="text-decoration: none;" href="http://code.google.com/p/waitzar/source/checkout">
						<img src="img/wz_source.png" border="0"/>
					</a>
				</td>
				<td width="100%" valign="top" align="left"  style="padding-top:10px; padding-left: 10px;">
					<a style="" href="http://code.google.com/p/waitzar/source/checkout">
						<span style="font-size: 1.25em; color: #00CCFF;">Play With the Source</span>
					</a>
				</td>
			</tr>
			<tr>
				<td colspan="2" style="color:white; padding-left: 10px; padding-bottom:10px; padding-top: 8px;">
					WaitZar is not only free, it is also "Open Source", which means that you can re-build the entire project yourself if you want to. Some of our users are quite talented programmers, and have, in the past, submitted patches to fix minor problems with the source code. If you want to contribute, that would be great! We know how much time goes into writing good code, so we treat all patch requests seriously.
				</td>
			</tr>
		</table>


		<table border="0" cellpadding="00" cellspacing="0" style="margin-bottom:20px; margin-top:10px;">
			<tr>
				<td width="1" valign="top" align="left"  style="padding-left:10px; padding-top:10px;">

						<img src="img/wz_friends.png" border="0"/>

				</td>
				<td width="100%" valign="top" align="left"  style="padding-top:10px; padding-left: 10px;">

						<span style="font-size: 1.25em; color: #00CCFF;">Tell Your Friends</span>

				</td>
			</tr>
			<tr>
				<td colspan="2" style="color:white; padding-left: 10px; padding-bottom:10px; padding-top: 8px;">
					Right now, the primary users of WaitZar are Burmese with an interest in computers and technology. In fact, we designed WaitZar to be easy to use for anyone who knows how to send email. So, we'd like you to share your experiences with others. If you have friends who can't be bothered to learn a Burmese keyboard layout, encourage them to try WaitZar. It's easy to use and quick to learn, so it's a great way for casual typists to start sending emails in Burmese.
				</td>
			</tr>
		</table>


        $<hrule_template>