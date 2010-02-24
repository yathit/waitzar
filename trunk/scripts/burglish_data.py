#!/usr/bin/python
# -*- coding: utf-8 -*-

#Common words consist of onset + rhyme, with some special cases.
#We use a "-" in the rhyme to indicate where the onset should go
#NOTE: I added an additional entry: {'':['အ']}, to represent cases of removed vowels.
#NOTE: I also re-worked this from a dict to a list, to avoid re-ordering. (Caused "shaun" to appear as "lyaun" in some cases)
COMMON_ONSETS = [(u'zz',[u'စ်']),(u'zw',[u'ဇြ']),(u'zh',[u'စ်']),(u'z',[u'ဇ',u'စ်']),(u'yy',[u'ယ',u'ယ်']),(u'yw',[u'ရြ',u'ယြ']),(u'yh',[u'ယွ']),(u'yh',[u'ယ',u'ယ်']),(u'y',[u'ရ',u'ယ',u'လ်',u'ယ်']),(u'x',[u'ဆ',u'စ']),(u'wh',[u'ဝွ']),(u'w',[u'ဝ']),(u'v',[u'ဗ',u'ဘ']),(u'u',[u'အ']),(u'ty',[u'တ်',u'ၾတ']),(u'tw',[u'တြ']),(u'tt',[u'ဋ']),(u'tr',[u'တ်',u'ၾတ']),(u'thw',[u'သြ']),(u'th',[u'သ']),(u't',[u'တ',u'ဋ',u'ထ']),(u'sy',[u'ၾဆ']),(u'sw',[u'စြ',u'ဆြ']),(u'ss',[u'ဆ']),(u'shw',[u'ရႊ']),(u'sh',[u'ရွ',u'လွ်',u'သွ်']),(u's',[u'စ',u'ဆ']),(u'r',[u'ရ',u'ယ',u'လ်']),(u'q',[u'က']),(u'py',[u'ျပ',u'ပ်']),(u'pw',[u'ပြ']),(u'phy',[u'ျဖ',u'ဖ်']),(u'phw',[u'ဖြ']),(u'ph',[u'ဖ']),(u'p',[u'ပ']),(u'o',[u'အ']),(u'ny',[u'ည',u'ျင',u'ဉ']),(u'nw',[u'ႏြ']),(u'nn',[u'ဏ']),(u'nhy',[u'ညွ',u'ျငွ',u'ဥွ']),(u'nhw',[u'ႏႊ']),(u'nhg',[u'ငွ']),(u'nh',[u'ငွ',u'ႏွ',u'ဏွ']),(u'ngw',[u'ငြ']),(u'ngh',[u'ငွ']),(u'ng',[u'င']),(u'n',[u'န',u'ဏ']),(u'my',[u'ျမ',u'မ်']),(u'mw',[u'မြ',u'ျမြ']),(u'mhy',[u'မွ်',u'ျမွ']),(u'mhw',[u'မႊ',u'ျမႊ']),(u'mh',[u'မွ']),(u'm',[u'မ']),(u'ly',[u'လ်',u'လွ်']),(u'lw',[u'လြ',u'လႊ']),(u'll',[u'ဠ']),(u'lhy',[u'လွ်',u'လ်']),(u'lhw',[u'လႊ']),(u'lh',[u'လွ',u'ဠွ']),(u'l',[u'လ',u'ဠ']),(u'kyw',[u'ၾကြ',u'ကြၽ']),(u'ky',[u'က်',u'ၾက']),(u'kw',[u'ကြ']),(u'khw',[u'ခြ']),(u'kh',[u'ခ']),(u'k',[u'က',u'ခ']),(u'j',[u'ဂ်',u'ျဂ']),(u'i',[u'အ']),(u'hw',[u'ဟြ']),(u'htw',[u'ထြ']),(u'htt',[u'ဌ']),(u'ht',[u'ထ',u'ဌ',u'႒']),(u'hs',[u'ဆ']),(u'hnw',[u'ႏႊ']),(u'hn',[u'ႏွ',u'ဏွ']),(u'hmy',[u'မွ်',u'ျမွ']),(u'hmw',[u'မႊ',u'ျမႊ']),(u'hm',[u'မွ']),(u'hly',[u'လွ်',u'လ်']),(u'hlw',[u'လႊ']),(u'hl',[u'လွ',u'ဠွ']),(u'hdd',[u'ဎ']),(u'hd',[u'ဍ',u'ဎ']),(u'h',[u'ဟ']),(u'gy',[u'ဂ်',u'ျဂ',u'ၾက']),(u'gw',[u'ဂြ']),(u'gh',[u'ဃ']),(u'gg',[u'ဃ']),(u'g',[u'ဂ',u'က',u'ဃ']),(u'fy',[u'ျဖ',u'ဖ်']),(u'fw',[u'ဖြ',u'ဘြ']),(u'f',[u'ဖ']),(u'e',[u'အ']),(u'dw',[u'ဒြ',u'ျဒ']),(u'dr',[u'ဒြ',u'ျဒ']),(u'dd',[u'ဓ']),(u'd',[u'ဒ',u'ဓ',u'တ',u'ဍ',u'ဎ']),(u'chw',[u'ခြၽ',u'ျခြ']),(u'ch',[u'ခ်',u'ျခ']),(u'c',[u'က']),(u'by',[u'ဗ်',u'ျဗ',u'ဘ်']),(u'bw',[u'ဘြ',u'ဗြ',u'ပြ']),(u'b',[u'ဘ',u'ဗ',u'ပ']),(u'a',[u'အ']),(u'-',[u'အ'])];
COMMON_RHYMES = [(u'ynn',[u'-င္း']),(u'yn',[u'-င္',u'-င္း']),(u'ye',[u'-ိုင္း',u'-ိုင္',u'-ိုင့္']),(u'y',[u'-ိုင္',u'-ိုင္း',u'-ိုင့္']),(u'uz',[u'-ြဇ္',u'ေ-ာဇ္']),(u'uu',[u'-ူး']),(u'ut',[u'-ြတ္',u'-ြပ္',u'ေ-ာတ္',u'-ြဋ္']),(u'urt',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'urd',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'urr',[u'-ား']),(u'urk',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'urh',[u'-ာ့']),(u'urd',[u'-ာဒ္',u'-ာ႒္']),(u'urb',[u'-ာဘ္']),(u'ur',[u'-ာ',u'-ား',u'-ာ့']),(u'unt',[u'-ြန္႕',u'-ြံ႕',u'-ြမ့္']),(u'unn',[u'-ြန္း',u'-ြမ္း',u'-န္း',u'-ြဏ္း']),(u'un',[u'-ြန္',u'-ြန္း',u'-ြမ္',u'-ြံ',u'-ြဏ္',u'-ြဏ္း']),(u'umt',[u'-ြမ့္']),(u'umm',[u'-ြမ္း',u'-ြမ္']),(u'um',[u'-ြမ္',u'-ြမ္း']),(u'uh',[u'-ူ႕']),(u'u',[u'-ူ',u'-ု',u'-ူ႕']),(u't',[u'ေ-ာက္']),(u'rr',[u'-ား']),(u'rh',[u'-ာ့']),(u'r',[u'-ာ',u'-ား',u'-ာ့']),(u'oy',[u'-ိြဳင္']),(u'ove',[u'-ုဗ္']),(u'ov',[u'-ုဗ္']),(u'out',[u'ေ-ာက္',u'ေ-ာတ္',u'ေ-ာဂ္']),(u'ount',[u'ေ-ာင့္']),(u'ounh',[u'ေ-ာင့္']),(u'oung',[u'ေ-ာင္',u'ေ-ာင္း']),(u'oun',[u'ေ-ာင္',u'ေ-ာင္း',u'ေ-ာင့္']),(u'oun',[u'ေ-ာင္',u'ေ-ာင္း']),(u'ou',[u'-ိုး',u'-ို']),(u'oth',[u'-ို႕']),(u'ote',[u'-ုတ္',u'-ုပ္',u'-ုက္',u'-ုစ္',u'-ုဇ္',u'-ုဂ္',u'-ုဋ္']),(u'ot',[u'ေ-ာ့',u'-ုတ္',u'-ုပ္',u'-ုက္',u'-ုစ္',u'-ုဇ္',u'-ုဂ္',u'-ုဋ္']),(u'ot',[u'-ို႕',u'ေ-ာ့',u'-ိုယ့္']),(u'ort',[u'ေ-ာ့']),(u'orh',[u'ေ-ာ့']),(u'or',[u'ေ-ာ္',u'ေ-ာ']),(u'ope',[u'-ုပ္',u'-ုတ္']),(u'op',[u'-ုပ္',u'-ုတ္']),(u'op',[u'-ို႕',u'ေ-ာ့',u'-ိုယ့္']),(u'ooz',[u'-ြဇ္',u'ေ-ာဇ္']),(u'oot',[u'-ြတ္',u'-ြပ္']),(u'oont',[u'-ြန့္',u'-ြမ့္']),(u'oonh',[u'-ြန့္',u'-ြမ့္']),(u'oon',[u'-ြန္း',u'-ြန္',u'-ြမ္း',u'-ြမ္']),(u'oomt',[u'-ြမ့္',u'-ြန့္']),(u'oomh',[u'-ြမ့္',u'-ြန့္']),(u'oom',[u'-ြမ္း',u'-ြမ္']),(u'ood',[u'-ြဒ္',u'-ြတ္']),(u'oo',[u'-ိုး',u'-ူး']),(u'ont',[u'-ြန္႔',u'-ြံ႕',u'-ြမ့္',u'-ုန္႕',u'-ံု႕']),(u'one',[u'-ုန္း',u'-ုမ္း',u'-ုံး',u'-ုဥ္း',u'-ုန္',u'-ုမ္',u'-ုံ']),(u'on',[u'-ြန္',u'-ြံ',u'-ံု',u'-ြဏ္']),(u'ol',[u'-ိုလ္',u'ေ-ာ',u'-ိုဠ္']),(u'oke',[u'-ုက္']),(u'ok',[u'-ုက္']),(u'oi',[u'-ိြဳင္']),(u'ohnh',[u'-ုန္႕',u'-ုမ့္']),(u'ohn',[u'-ုန္း',u'-ုမ္း']),(u'ohmh',[u'-ုန္႕',u'-ုမ့္']),(u'ohm',[u'-ုန္း',u'-ုမ္း']),(u'oh',[u'-ို႕',u'ေ-ာ့',u'-ိုယ့္']),(u'oet',[u'-ို႕']),(u'oeh',[u'-ို႕']),(u'oe',[u'-ိုး']),(u'ode',[u'-ုဒ္',u'-ုဎ္']),(u'od',[u'-ုဒ္',u'-ုဎ္']),(u'oav',[u'-ုဗ္']),(u'oat',[u'-ုတ္',u'-ုပ္',u'-ုက္',u'-ုစ္',u'-ုဇ္',u'-ုဂ္',u'-ုဋ္']),(u'oap',[u'-ုပ္',u'-ုတ္']),(u'oant',[u'-ုန္႕',u'-ုမ့္',u'-ံု႔',u'-ုဥ့္']),(u'oann',[u'-ုန္း',u'-ုမ္း',u'-ံုး',u'-ုဥ္း']),(u'oanh',[u'-ုန္႕',u'-ုမ့္',u'-ံု႔',u'-ုဥ့္']),(u'oan',[u'-ုန္',u'-ုမ္',u'-ံု',u'-ုဏ္',u'-ုဥ္',u'-ုလ္']),(u'oak',[u'-ုက္']),(u'oad',[u'-ုဒ္',u'-ုဎ္',u'-ုသ္']),(u'oa',[u'-ြာ']),(u'o',[u'-ို',u'-ိုး',u'-ိုရ္',u'-ိုယ္',u'-ိုဠ္',u'-ိုဟ္']),(u'iz',[u'-ဇ္',u'-ာဇ္']),(u'ite',[u'-ိုက္']),(u'it',[u'-စ္',u'-တ္',u'ေ-တ္',u'ေ-က္',u'-ဋ္',u'ေ-စ္']),(u'is',[u'-စ္',u'ေ-စ္']),(u'is',[u'-စ္',u'-တ္',u'ေ-တ္',u'ေ-က္',u'-ဋ္',u'ေ-စ္']),(u'int',[u'-င့္',u'-ဥ့္']),(u'inn',[u'-င္း',u'-ဥ္း']),(u'ing',[u'-င္း',u'-ဥ္း']),(u'ine',[u'-ိုင္',u'-ိုင္း',u'-ိုဏ္း']),(u'in',[u'-င္',u'-င္း',u'-ဥ္',u'ေ-န္',u'-ဥ္း']),(u'ike',[u'-ိုက္']),(u'ik',[u'-စ္',u'-တ္',u'ေ-တ္',u'ေ-က္',u'-ဋ္',u'ေ-စ္']),(u'ii',[u'-ီး',u'-ည္း',u'-ည့္']),(u'ih',[u'-ည့္']),(u'ide',[u'-ိုဒ္']),(u'id',[u'-စ္']),(u'i',[u'-ိ',u'-ီ',u'-ည္',u'-ည္း',u'-ည့္']),(u'g',[u'ေ-ာင္',u'ေ-ာင္း']),(u'f',[u'-္']),(u'eyy',[u'ေ-း',u'-ည္း']),(u'eyt',[u'ေ-့',u'-ည့္']),(u'eyh',[u'ေ-့',u'-ည့္']),(u'ey',[u'ေ-း',u'-ည္း']),(u'ey',[u'ေ-',u'-ည္',u'ေ-း',u'ေ-့',u'-ည္း']),(u'et',[u'-က္',u'-တ္',u'-ပ္']),(u'ert',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'ert',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'err',[u'-ား']),(u'erk',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'erh',[u'-ာ့']),(u'erd',[u'-ာဒ္',u'-ာ႒္']),(u'erb',[u'-ာဘ္']),(u'er',[u'-ာ',u'-ား',u'-ာ့']),(u'en',[u'-ဲန္း',u'-ဲန္',u'-န္']),(u'elh',[u'-ဲ့',u'-ယ့္',u'-ည့္']),(u'el',[u'-ဲ',u'-ယ္',u'-ည္',u'-ည္း',u'-ဲ့',u'-ည့္']),(u'ek',[u'-က္']),(u'eit',[u'-ိတ္',u'-ိပ္',u'-ိက္',u'-ိဋ္',u'-ိသ္']),(u'eint',[u'-ိန္႕',u'-ိမ့္']),(u'einn',[u'-ိန္း',u'-ိမ္း',u'-ိဏ္း']),(u'einh',[u'-ိန္႕',u'-ိမ့္']),(u'ein',[u'-ိန္',u'-ိမ္',u'-ႎ',u'-ိင္',u'-ိဥ္',u'-ိဏ္',u'-ိလ္']),(u'eih',[u'ေ-့',u'-ဲ့',u'-ယ့္',u'-ည့္']),(u'ei',[u'ေ-း',u'-ဲ',u'-ယ္',u'ေ-',u'-ည္း',u'-ည္']),(u'eh',[u'-ဲ့',u'-ည့္',u'ေ-့',u'-ဲ',u'-ည္']),(u'ee',[u'-ီး',u'-ည္း']),(u'eck',[u'-က္']),(u'ec',[u'-က္']),(u'e`',[u'-ဲ့',u'-ဲ']),(u'e',[u'-ီ',u'-ဲ',u'-ည္',u'-ယ္',u'ေ-',u'ေ-း',u'-ည့္',u'-ဲ့']),(u'ayy',[u'ေ-း',u'-ည္း']),(u'ayt',[u'ေ-့',u'-ည့္']),(u'ayh',[u'ေ-့',u'-ည့္']),(u'aye',[u'ေ-း',u'-ည္း']),(u'ay',[u'ေ-',u'-ည္',u'ေ-း',u'ေ-့',u'-ည္း']),(u'aww',[u'ေ-ာ']),(u'awt',[u'ေ-ာ့']),(u'awn',[u'ေ-ာန္']),(u'awh',[u'ေ-ာ့']),(u'aw',[u'ေ-ာ',u'ေ-ာ္',u'ေ-ာ့',u'ေ-ာဝ္']),(u'ave',[u'-ိဗ္']),(u'av',[u'-ဗ္']),(u'aut',[u'ေ-ာက္',u'ေ-ာတ္']),(u'aunt',[u'ေ-ာင့္']),(u'aunh',[u'ေ-ာင့္']),(u'aung',[u'ေ-ာင္',u'ေ-ာင္း']),(u'aun',[u'ေ-ာင္',u'ေ-ာင္း',u'ေ-ာင့္']),(u'aun',[u'ေ-ာင္',u'ေ-ာင္း']),(u'auk',[u'ေ-ာက္',u'ေ-ာတ္']),(u'au',[u'ေ-ာ']),(u'ath',[u'-သ္']),(u'ate',[u'-ိတ္',u'-ိပ္',u'-ိဇ္',u'-ိစ္',u'-ိက္',u'-ိဋ္',u'-ိသ္']),(u'at',[u'-တ္',u'-က္',u'-ပ္',u'-ဟ္',u'-ဋ္']),(u'art',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'art',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'arr',[u'-ား']),(u'arnn',[u'-ာန္း',u'-ာဏ္း']),(u'arn',[u'-ာန္',u'-ာဏ္',u'-ာဟ္']),(u'arl',[u'-ာယ္',u'-ာည္']),(u'ark',[u'-ာတ္',u'-ာက္',u'-ာဟ္']),(u'arh',[u'-ာ့']),(u'ard',[u'-ာဒ္',u'-ာ႒္']),(u'arb',[u'-ာဘ္']),(u'ar',[u'-ာ',u'-ား',u'-ာ့']),(u'ape',[u'-ိပ္']),(u'ap',[u'-ပ္']),(u'ant',[u'-န္႕',u'-ံ့',u'-မ့္']),(u'ann',[u'-န္း',u'-မ္း',u'-ဏ္း']),(u'an',[u'-န္',u'-ံ',u'-မ္',u'-ဏ္',u'-လ္']),(u'amm',[u'-မ္း']),(u'am',[u'-မ္',u'-မ္း',u'-န္',u'-ံ']),(u'alh',[u'-ယ့္',u'-ဲ့',u'-ည့္',u'-ဲ',u'-ည္']),(u'al',[u'-ယ္',u'-ဲ',u'-ည္',u'-ည္း',u'-ဲ့',u'-ည့္',u'-လ္']),(u'ake',[u'-ိက္',u'-ိတ္']),(u'ak',[u'-က္']),(u'aiv',[u'-ိဗ္']),(u'ait',[u'-ိတ္',u'-ိပ္',u'-ိဇ္',u'-ိစ္',u'-ိက္']),(u'aid',[u'-ိတ္',u'-ိပ္',u'-ိက္',u'-ိဋ္',u'-ိသ္']),(u'aip',[u'-ိပ္']),(u'aint',[u'-ိန္႕',u'-ိမ့္']),(u'ainn',[u'-ိန္း',u'-ိမ္း',u'-ိဏ္း']),(u'ainh',[u'-ိန္႕',u'-ိမ့္']),(u'aing',[u'-ိုင္',u'-ိုင္း']),(u'ain',[u'-ိန္',u'-ိမ္',u'-ႎ',u'-ိင္',u'-ိဥ္',u'-ိဏ္',u'-ိလ္']),(u'aik',[u'-ိက္',u'-ိတ္']),(u'aid',[u'-ိဒ္']),(u'ai',[u'-ိုင္း',u'-ိုင္',u'-ိုဏ္း',u'-ိုင့္',u'ေ-']),(u'ag',[u'-ဂ္']),(u'aeh',[u'ေ-့',u'-ည့္',u'-ဲ့']),(u'ae',[u'-ယ္',u'-ဲ',u'ေ-',u'-ည္',u'ေ-း']),(u'ade',[u'-ိဒ္']),(u'ad',[u'-ဒ္',u'-ဎ္']),(u'ack',[u'-က္']),(u'ac',[u'-က္']),(u'ab',[u'-ဘ္']),(u'a`',[u'-ဲ့',u'-ဲ']),(u'a',[u'-',u'-ာ့']),(u'`',[u'-ဲ့',u'-ဲ'])]

#Special words are generally added as-is. They are either too specific, too general, 
#  or too rare to construct from patterns.
#TODO: Figure out what the "-1" in some entries is used for.
SPECIAL_WORDS = [[u'kyarr',u'က်္ား',-1],[u'kyunote',u'ကြၽႏု္ပ္',-1],[u'nhite',u'၌',-1],[u'hnite',u'၌',-1],[u'shat',u'ယွက္',-1],[u'nyin',u'ညာဥ္',-1],[u'shin',u'ယွဥ္',-1],[u'kyar',u'က်္ာ',-1],[u'yway',u'၍',-1],[u'umm',u'အမ္'],[u'imm',u'အင္းမ္...'],[u'yin',u'ယာဥ္',-1],[u'yin',u'ယ်ာဥ္',-1],[u'ywe',u'၍',-1],[u'd',u'ဒီ'],[u'u',u'ယူ'],[u'own',u'အံုး',-1],[u'it',u'ဧတ္'],[u'el',u'ဧည့္'],[u'ei',u'ဣ'],[u'or',u'ဪ',-1],[u'ei',u'၏'],[u'ei',u'ဤ'],[u'oo',u'ဥ'],[u'ah',u'အ'],[u'aw',u'ဪ'],[u'ay',u'ဧ'],[u'ag',u'ေအာင္'],[u'oo',u'ဦး'],[u'oh',u'အိုး'],[u'r',u'အာ'],[u'ae',u'အဲ'],[u'ei',u'အိ'],[u'ei',u'အီ'],[u'um',u'အမ္'],[u'.',u'။'],[u',',u'၊'],[u',',u'ျပီး'],[u'.',u'ျပီ'],[u'4',u'၎',-1],[u'.',u'ဤ'],[u'.',u'သည္'],[u'.',u'၏'],[u',',u'၌'],[u',',u'၍'],[u',',u'ႏွင့္'],[u'f',u'္',-1],[u'b',u'ျပီ'],[u'o',u'အို'],[u'p',u'ျပီ'],[u'e',u'ဤ'],[u'a',u'အ'],[u'a',u'ေအ',-1],[u'u',u'ဥ'],[u'u',u'ဦး'],[u'h',u'့',-1],[u';',u'း'],[u'eu',u'အူ'],[u'u',u'အူ'],[u'u',u'အု'],[u'a',u'အစ္',-1],[u'tun',u'ထြန္း']];

#Numbers can easily be constructed digitally (e.g., 123 = ၁၂၃). The third row in the array accounts for this.
#However, constructing the spoken name of a sequence requires not only the individual digits (e.g., တစ္),
#  but also connecting words which differ based on digit and some notion of plurality (if it occurs as the last word
#  in a string, etc.). These are represented in the first and second rows, respectively.
#TODO: I will not generate numbers right now, there are too many. Later, when I program this in C++,
#  I will allow for complex numbers by generating them on the fly.
NUMBER_CONSTRUCTOR = [	 \
    [u'တစ္',u'ႏွစ္',u'သံုး',u'ေလး',u'ငါး',u'ေျခာက္',u'ခုႏွစ္',u'ရွစ္',u'ကိုး'],  \
    [[u'ဆယ္',u'ဆယ့္'],[u'ရာ',u'ရာ့'],[u'ေထာင္',u'ေထာင့္'],[u'ေသာင္း',u'ေသာင္း'],[u'သိန္း',u'သိန္း'],[u'သန္း',u'သန္း'],[u'ကုေဋ',u'ကုေဋ']],  \
    [u'၀', u'၁', u'၂', u'၃', u'၄', u'၅', u'၆', u'၇', u'၈', u'၉']  \
];

#Correct common typos; if the first matches, replace with the second.
#Although this is normally only used when dealing with "normalized" Zawgyi, it is useful
#  (and harmless) to apply it after building the "standard" set of results. 
#NOTE: I have re-worked these to a custom format:
#  Each entry in the list contains two elements:
#  [matcher, replaceStr] = [[], u''], where matcher helps determine how to match the pattern, and replaceStr is substituted in after a match.
#  Matcher is defined as a series of strings of the form:
#    (type, matchStr) = (u'X....'), where "X" is the first letter in the string
#  Types can be any of the following; the resultant meaning of "matchStr" is also defined:
#    'M' = "match". Match a single letter from matchStr
#    'N'  = "not". Ensure that no letter from matchStr matches.
#    'O'  = "optional". Either match a single letter from matchStr, or don't match anything.
#    'X'  = "optional not". Either match a single letter NOT in matchStr, or don't match anything
#    'E'   = "erase". Match a single letter from matchStr, then remove it after all matching has completed.
#    'F'   = "find". Match the ENTIRE matchStr. The replaceStr will take its place after all matching has completed.
#  In addition, for the "match" type, the following wildcard is supported.
#    u'*'   = Match any letter. Note that matchStr must contain ONLY the * character for this to work.
#  We might allow * for "erase" types, but at the moment it's not needed.
#  Each entry must contain ONE and ONLY ONE "find" type.
#  Each entry MAY contain ONLY ONE "erase"  type.
#  Note that this system approximates a regex engine, in that it runs on a single pass through the string. It does 
#    not require exponentially additional memory like regexes do.
#  TODO: Either move to a simple regex engine in the future, or simply remove the need for this by outputing Unicode to begin with.
#  TODO: Using the original JavaScript regexes, see if any of these are actually triggered by the Standard Combinations
str1000_1021 = u''.join([unichr(x) for x in range(0x1000, 0x1021+1)])
str1060_107C = u''.join([unichr(x) for x in range(0x1060, 0x107C+1)])
str103B_7E_84 = u'\u103B' + u''.join([unichr(x) for x in range(0x107E, 0x1084+1)])
ZAWGYI_DE_NORMALIZE = [  \
    [[u'N\u103B\u107E', u'M\u1001\u1002\u1004\u1012\u1015\u101D\u1040', u'O'+str1060_107C+u'\u1087\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A', u'F\u102C'], u'\u102B'],  \
    [[u'N\u1031', u'M\u1001\u1002\u1004\u1012\u1015\u101D\u1040', u'N\u1039', u'O'+str1060_107C+u'\u1087\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A', u'F\u102C'], u'\u102B'],  \
    [[u'F\u102B\u1039'], u'\u105A'],  \
    [[u'F\u1014', u'O\u1036\u102D\u1032', u'M'+str1060_107C+u'\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A'], u''],  \
    [[u'M'+str1060_107C+u'\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A\u1014\u103A\u1033\u1034', u'X\u1037', u'F\u1037'], u'\u1095'],  \
    [[u'M'+str1060_107C+u'\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A\u1014\u103A\u1033\u1034', u'X\u1094', u'F\u1094'], u'\u1095'],  \
    [[u'M'+str103B_7E_84, u'M'+str1000_1021+u'\u108F\u1090', u'O\u102D\u103C\u103D\u108A', u'F\u102F'], u'\u1033'],  \
    [[u'M'+str103B_7E_84, u'M'+str1000_1021+u'\u108F\u1090', u'O\u102D\u103C\u103D\u108A', u'F\u1030'], u'\u1034'],  \
    [[u'M\u1008\u1009\u100A\u100B\u100C\u100D\u1020\u1025\u103A'+str1060_107C+u'\u1085\u1091\u1092\u1093', u'O\u102D\u103C\u103D\u108A', u'F\u102F'], u'\u1033'],  \
    [[u'M\u1008\u1009\u100A\u100B\u100C\u100D\u1020\u1025\u103A'+str1060_107C+u'\u1085\u1091\u1092\u1093', u'O\u102D\u103C\u103D\u108A', u'F\u1030'], u'\u1034'],  \
    [[u'F\u103D', u'O\u1036\u102D\u108E\u1032', u'E\u102F'], u'\u1088'],  \
    [[u'F\u103D', u'O\u1036\u102D\u108E\u1032', u'E\u1030'], u'\u1089'],  \
    [[u'F\u101B', u'O\u1036\u102D\u108E\u1032', u'M\u102F\u1030\u1088\u1089'], u'\u1090'],  \
    [[u'F\u103D', u'O\u102D\u108E\u1032\u107D\u103A', u'E\u103D'], u'\u103D'],  \
    [[u'E\u103C\u108A', u'F\u103D'], u'\u108A'],  \
    [[u'E\u103D', u'F\u103C'], u'\u108A'],  \
    [[u'F\u103A\u103C'], u'\u103C\u107D'],  \
    [[u'F\u103D', u'O\u102D\u102E\u103A\u107D', u'E\u103C'], u'\u108A'],  \
    [[u'F\u1087', u'O\u102D\u102E\u103A\u107D', u'E\u103C'], u'\u108A'],  \
    [[u'F\u103A\u103D'], u'\u103D\u103A'],  \
    [[u'M\u103B\u107E', u'X\u103D', u'X\u103D', u'F\u103D'], u'\u1087'],  \
    [[u'M\u101D\u103C\u108A', u'X\u103C', u'X\u103C', u'F\u103C'], u''],  \
    [[u'M\u1025', u'F\u102F'], u''],  \
    [[u'M\u1025', u'F\u1033'], u''],  \
    [[u'F\u1064', u'X\u102D', u'X\u102D', u'E\u102D'], u'\u108B'],  \
    [[u'F\u1064', u'X\u102E', u'X\u102E', u'E\u102E'], u'\u108C'],  \
    [[u'F\u102C\u1095'], u'\u102C\u1037'],  \
    [[u'F\u107E', u'M\u1000\u1003\u1006\u100F\u1010\u1011\u1018\u101A\u101C\u101E\u101F', u'O\u1036\u102D\u108E\u1032', u'M\u103C'], u'\u1082'],  \
    [[u'N\u103B\u107E', u'M\u1001\u1002\u1004\u1012\u1015\u101D\u1040', u'O'+str1060_107C+u'\u1087\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A', u'F\u102C\u1039'], u'\u105A']  \
]

#Used to check for invalid generated combinations. If one of these matches, the string will be rejected.
#NOTE: I modified these. Each replacement regex is now a pair of arrays, of the form
#  [letter1, letter2], where letter1/2 are strings of optional letters (character classes).
#I am trying to move the code away from regexes, to make it port easier to C++
INVALID_COMBINATIONS = [  \
    [u'\u100D\u100B\u100C\u1023', u'\u1087\u103D\u102F\u1030\u1088\u1089\u103C\u108A'],  \
    [u'\u1020', u'\u103C\u108A'],  \
    [str1060_107C+u'\u1092', u'\u1087\u103D\u1088\u1089\u103C\u108A']  \
]

#Some words can be expanded into additional words simply by virtue of substitution. 
#If the first entry matches, try to add the second entry to the dataset.
#NOTE: I re-worded these slightly, to be:
#    [find, neglook, rep], where "find" is the string to find, "neglook" is the negatively-matched 
#    lookahead, and "rep" is the string to replace. "neglook" is assumbed to be a character class, if it is non-empty.
EXPAND_PATTERNS = [  \
    [u'အု', u'', u'ဥ'],  \
    [u'အိ', u'\u102F',u'ဣ'],  \
    [u'ေအာ', u'\u1039\u1037', u'ဩ']  \
]

