# Introduction #

Wait Zar 1.8 is done, so now I can focus more on side projects. One thing that I really want to do is formalize Key Magic. That way, there will be less confusion for people implementing it on their own.


# Versioning #

We need to document the current syntax (so people will understand how Wait Zar and Key Magic work), but there are already things that we want to add. Also, there are new features added by dr. Carbon (or, occasionally, myself) that we are not sure about (and hence cannot document completely). Here is what I propose:
  * In the documents listed below, there are three versions of Key Magic. The **current version (1.0)** is written in black text, and describes how Key Magic works now (roughly, December 2010). The <font color='#000099'><b>new version (2.0)</b></font> is written in dark blue, and describes all of the proposed features that dr. Carbon and I agree on. The <font color='#990000'><b>legacy version (0.1)</b></font> is written in dark red, and describes syntax that is either:
    * Not in the current version, but might be supported for compatibility with old source files (e.g., representing "ANY" with "`*`")
    * Currently represented differently in Key Magic and Wait Zar. In this case, we will choose the Key Magic representation to be official, and will document Wait Zar's usage as legacy.
  * If dr. Carbon or I (or anyone else) wants to implement their own features, they can do so at any time. If the feature proves useful, then a formal description can be added to the documentation for versions 2.1, 2.2, etc.
  * Versions 1.1, 1.2, etc. of the standard will have less features in blue and more features in black or red. When all of the 2.0 features have been successfully tested in both Key Magic and Wait Zar, then we will start on 2.1, 2.2, etc.


# Steps #

First, we need to document the **Source File Syntax**. The [Layout Scripting](http://code.google.com/p/keymagic/wiki/LayoutScripting) page on Key Magic's website is a good start, and there's some documentation on this wiki too. So, it should be fairly easy to finish this up and write, say, an EBNF specification of the Key Magic grammar.

I'd like to write a short test project using [Sable CC](http://sablecc.org) which reads layout files. This can be the "reference implementation" of our grammar, even though we probably won't use it (Sable CC is Java-based).

Second, we need to document the **Matching Algorithm**. Essentially, how does Key Magic sort the rules, then how does it go about matching them. This should be done using some formalism, not just using code (our two projects match rules in very different ways). This will probably be the hardest part of the documentation project.

If possible, I'd like to expand our java parser with a library which applies the matching algorithm and has a simple text box for testing rules files. (This should use some high-level modeling construct, not just Java code, so it can be understood formally.)

Thirdly, as we are building the first two documents, there is sure to be some information which is useful, but not directly relevant to the formalization. I'd like to make a short list of this, and include it as a kind of "Key Magic In Practice" document. This document will mostly be a series of quick notes.

We do not need to document the various binary storage formats of keyboards.