from locus import FallbackTemplate

class InstructionsTemplate(FallbackTemplate):
	template = r'''
        <map name="amln_map">
          <area alt="Download WaitZar" title="Download WaitZar"
             href="http://waitzar.googlecode.com/svn/trunk/nightly/WaitZar.exe"
             shape=rect coords="232,105,408,123"/>
          <area alt="Download WaitZar" title="Download WaitZar"
             href="http://code.google.com/p/waitzar/"
             shape=rect coords="232,130,408,148"/>
          <area alt="WaitZar User's Guide" title="WaitZar User's Guide"
             href="http://waitzar.googlecode.com/svn/trunk/WaitZar%20User%27s%20Guide.doc"
             shape=rect coords="158,966,341,984"/>
          <area alt="Email Help@WaitZar" title="Email Help@WaitZar"
             href="mailto:help@waitzar.com"
             shape=rect coords="391,966,517,984"/>
        </map>

        <h1>How to Use WaitZar</h1>
        <img src="http://waitzar.googlecode.com/svn/trunk/a_myan_lann_nyyon.png"  usemap="#amln_map"/>
        $<hrule_template>
		'''

