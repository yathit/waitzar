# Introduction #

We now support virtually all of KeyMagic's features. Eventually, I want to work on a specification. But for now, I would like to list some things that bothered me when editing the Zawgyi-One KeyMagic file. Details of KeyMagic in general can be found here:
http://code.google.com/p/keymagic/

Note that **most** of these changes are backwards-compatible with KeyMagic, so existing developers can continue to use the syntax as it exists now.



---



# Smart Backspace #

**Difficulty:** Easy

Backspace should behave more intelligently by default. Zawgyi.kbd has a lot of rules like this:
```
    U200B + U104B + <VK_BACK> => null
```
This requires ALL re-orderings to be reversible, which is kind of extreme. In fact, when most users hit backspace, they really just want to "undo" the last letter they typed, including all of its reordering effects. We can add a header parameter:
```
   "@smart_backspace" = "true"
```
If this is on, then each time a letter is typed, we can push the current string to the stack. When the user types "backspace", just pop the stack and present this to the user. The normal rule about backspace only activating if no rules match VK\_BACK also applies.



---



# Arrays-Of-Strings #

**Difficulty:** Medium


All the useful KeyMagic syntax (like $res[**]) only works on individual characters. Something like this:
```
   $row1K = "1234"
   $row1U = U1041 + U1042 + U1043 + U1044
   $row1K[*] => $row1U[$1]
```
...is a very useful abstraction. Unfortunately, it only applies to strings. Trying this:
```
   //NOTE: Doesn't work!
   $stackedUNI = "\u1004\u103A\u1039" + "\u1039\u1000" + "\u1039\u1001"
   $stackedZG = "\u1064" + "\u1060" + "\u1061"
   $stackedUNI[*] => $stackedZG[$1]
```
...doesn't. Instead of replacing kinzi and stacked ka/kha with its equivalent in Zawgyi, it replaces each individual letter, resulting in either the wrong string entirely or a crash (if it overflows the array). The thing is, it would be nice to be able to use something like this.**

I propose the following syntax to designate an array:
```
   $stackedUNI = ["\u1004\u103A\u1039" + "\u1039\u1000" + "\u1039\u1001"]
```
This is very similar to the current syntax. Regular strings can be automatically converted into arrays, if this helps:
```
   //$row1K = "1234"
   $row1K = ["1" + "2" + "3" + "4"]
   //$row1U = U1041 + U1042 + U1043 + U1044
   $row1U = [U1041 + U1042 + U1043 + U1044]
```
The only problem is that some developers might like the UXXXX syntax:
```
   //NOTE: Doesn't work!
   $stackedUNI = [U1004+U103A+U1039 + U1039+U1000 + U1039+U1001]
```
But I think this won't happen. First of all, UXXX is generally only used in regular strings now. Secondly, most programmers are already familiar with using quotes for strings.

Variables would have to be flattened to include things like:
```
   $kinzi = U1004 + U103A + U1039
   $stackedKA = "\u1039\u1000"
   $stackedKHA = "\u1039\u1001"
   $stackedUNI = [$kinzi + $stackedKA + $stackedKHA]
```
Also, note that in this example, strings are specified with UXXXX, but that's ok since they are contained in variables. However, this should ONLY occur inside of the array brackets; consider something like this:
```
   $num1 = U0031 + U0032 + U0033
   $num2 = U0034 + U0035 + U0036
   $numX = $num1 + $num2
   $numA = [$num1 + $num2]
   $numA2 = [$numX]
```
For this case, $numX should be treated as "123456", while $numA should be treated as ["123", "456"]. In addition, $numA2 should be ["123456"]. Finally, we would need some equivalency rules. Consider:
```
   $num1 = "123"
   $num2 = "456"
   $numA = [$num1 + $num2]
   "a" => $numA
```
...should this replace "a" with the entire flattened array (123456) or just the first element (123)? And what if we WANT to break apart a string? Can we add some new syntax like this:
```
   $kinzi = "\u1004\u103A\u1039"
   $others = "\u102B\u102C\u102F\u1030"
   $postfix = [$kinzi + $[others]]
   //Should expand to: 
   //  $postfix = ["\u1004\u103A\u1039" + "\u102B" + "\u102C" + "\u102F" + "\u1030"]
```
And what about stacking arrays inside of other arrays (via variables)? Anyway, these are just edge cases; I think that the whole point to this example is it should be easy to represent multiple letters as a single entity.



---



# Single-Pass Matching #

**Difficulty:** Easy

We need a switch:
```
   "@single-pass-matching" = "true"
```
If the program matches a single rule, currently it will restart at the beginning of the rules set. With this switch, we:
  * Don't reset (keep matching from the current rule)
  * Don't sort the rules set (we can still group rules that have switches together)
  * Don't stop after the "single letter ASCII" rule
Basically, some developers are comfortable with only one match loop, and it's easy to gain some performance by not constantly resetting.



---



# Better Replacement for Single Letter ASCII #

**Difficulty:** Easy

Consider typing |=> to mean "Only match one rule of this type per letter typed", with => meaning "keep matching until no more rules match". So, |=> represents a keypress, and => represents a normalization rule. For example:
```
   //VARIABLES
   $row3K = "sdfghkl;'"
   $row3U = U103B + U102D + U103A + U102B + U1037 + U102F + U1030 + U1038 + U1012
   
   //KEYS
   'a' |=> $twh
   'j' |=>  $yayit
   $row3K[*] |=> $row3U[$1]

   //NORMS
   $yayit + $cons => $2 + $1
   $twh + $cons => $2 + $1
   $twh + $yayit => $2 + $1
```
In this case, after matching "a", the system would no longer match any more KEYS rules. The NORMS rules would still match. If "single-pass-matching" is enabled, then the system can just jump to the end of the |=> rules (we can sort them, of course).

This reflects what users are actually modeling; "type a key" and then "re-order as appropriate".




---



# Ranges #

**Difficulty:** Easy

We should allow something like this:
```
   $cons = U[1000..1003] + U[1005..101F]
```
This will automatically expand to:
```
   $cons = U1000 + U1001 + U1002 + U1003 + U1005 + U1006 + U1007 + \
            U1008 + U1009 + U100A + U100B + U100C + U100D + U100E + U100F + \
            U1010 + U1011 + U1012 + U1013 + U1014 + U1015 + U1016 + U1017 + \
            U1018 + U1019 + U101A + U101B + U101C + U101D + U101E + U101F
```
Some regular expressions use the minus sign to remove elements from a range, but I think that's not needed now. Just a simple "letters in this range" would be good.



---



# Easier Normalization #

**Difficulty:** Hard

A lot of rules focus on what essentially boils down to normalization. Consider:
```
   $dotBelow + $dotAbove  => $2 + $1
```
The only purpose of this rule is to ensure that $dotAbove comes before $dotBelow. Unfortunately, what if something's in between them (like $legFwd)? You'll need a new rule to handle this case:
```
   $dotBelow + $legFwd + $dotAbove  => $3 + $2 + $1
```
This can become nightmarish if there are multiple optional letters in between the two. And ALL keyboards that do reordering (Zawgyi/Win Innwa/Ayar if they choose to output Unicode) face the same problem. I'd like to add a **normalization pattern**, which is put into effect after no more rules match (or, in single-pass matching, after all |=> rules) and looks something like this:
```
   $consonant = U[1000..1021]
   $stackedCons = ["\u1039\u1000" + "\u1039\u1001" + /*manually expand*/]
   $yayit = U103C
   $twh = U1031
   $kinzi = U1004 + U103A + U1039
   $upperVowel = U102D + U102E + U1032
   $lowerVowel = U102F + U1030
   $circBelow = U103D
   $aspirate = U103E

   NORMALIZATION = {$kinzi + $consonant[*] + $stackedCons[*] + $yayit + $circBelow + $aspirate + $twh + $upperVowel[*] + $lowerVowel[*]}
```
Here, the curly braces are a special array which doesn't merge sub-arrays. That way, $kinzi matches the entire pattern, but $stackedCons only matches U1039 plus ONE consonant.

The main challenge with this is that KeyMagic would have to do a lot of smart processing to reorder correctly. After all, the above normalization gives no indication of segment boundaries. Maybe we can do a mapping?
```
   $consonant = U[1000..1021]
   $stackedCons = ["\u1039\u1000" + "\u1039\u1001" + /*manually expand*/]
   $yayit = U103C
   $twh = U1031
   $kinzi = U1004 + U103A + U1039
   $upperVowel = U102D + U102E + U1032
   $lowerVowel = U102F + U1030
   $circBelow = U103D
   $aspirate = U103E

   $zawgyiPrefix = [$yayit + $twh]
   $zawgyiPostfix = $upperVowel + $lowerVowel + $circBelow + $aspirate

   {$zawgyiPrefix + $consonant[*] + [$kinzi + $[stackedCons] + $[zawgyiPostfix]]} |=NORM=> {$kinzi + $consonant[*] + $stackedCons[*] + $yayit + $circBelow + $aspirate + $twh + $upperVowel[*] + $lowerVowel[*]}
```
In other words, we define a normalization of Zawgyi ("what order can it be typed?") and map it to Unicode ("what order should it be output?"). Note that the $stackedCons syntax is incorrect here; we'd have to be VERY careful with the array syntax here.

So, this is hard for many reasons. But it would really simplify rules files, and should definitely boost performance.



---



# Null Switch #

**Difficulty:** Medium

Currently, if we have a rule like:
```
   'j' => U103C
```
...then there is some inconsistency with Shift/Ctrl/Alt and switches. Consider:
```
   'J' => "something"
   ('my_switch') + 'j' => "else"
```
In this case, the switch will act like a new modifier key. But consider:
```
   'X' => "something"
   ('my_switch') + 'x' => "else"
```
Now, because 'j' is not explicitly captured with the switch, then Shift+J will output nothing (it doesn't match) but 'my\_switch'+j will output U103C (because not specifying a switch will match if no other switch matches). Thus, we are forced to add:
```
   'X' => "something"
   ('my_switch') + 'x' => "else"
   ('my_switch') + 'j' => ""
```
...which is odd ---and also inconsistent, since Shift+J doesn't need its own "null rule". We should instead add a switch that means "no switches". Maybe the empty string?
```
   ('') + 'j' => U103C
```
This isn't too hard; we can implement it as an actual switch, and then just turn it on if nothing else is on. So, a rule like this:
```
   ('') + ('my_switch') + 'j' => "something"
```
...would never match; but that's easy to check when compiling the keyboard.



---



# Switched Ranges #

**Difficulty:** Easy

Adding a switch for multiple keys is tedious:
```
   ('zg_key') + '1' => U100E
   ('zg_key') + '2' => U1039 + U100C
   ('zg_key') + '3' => U1039 + U100B
   //....and so on
```
We should add some way of specifying a range of rules that a switch applies to. So, for example, this:
```
   ('zg_key')<START>
     '1' => '1'
     ('symbol_key') + '2' => '2'
     ('zg_key') + '3' => '3'
     '3' => '3'
   ('zg_key')<END>
   ('symbol_key') + '4' => '3'
```
...expands into:
```
   ('zg_key') + '1' => '1'
   ('zg_key') + ('symbol_key') + '2' => '2'
   ('zg_key') + '3' => '3'
   ('zg_key') + '3' => '3'
   ('symbol_key') + '4' => '3'
```
At compile time, it's easy to just tag a rule with all the switches that apply to it. This really won't complicate things much.



---



# Add a Switch for Shift, Ctrl, Alt #

**Difficulty:** Easy

Some people like the behavior of switches, and might want to do this:
```
   'j' => U103C
   (<VK_SHIFT>) + 'j' => "something"
   (<VK_ALT>) + 'j' => "else"
   (<VK_CTRL>) + (<VK_SHIFT>) + 'j' => "entirely"
```
This would, for example, allow the user to let "ctrl+alt" match the same thing as "alt", but override "ctrl+shift". I wouldn't use it, but there's no reason not to.

We'd have to make sure that these always reflect the current state of Ctrl/Alt/Shift; for example, it shouldn't be possible to turn one of these on or off:
```
   //NOTE: Error!
   'j' => U103C + (<VK_SHIFT>)
```


---



# "Optional" Match #

**Difficulty:** Medium

Consider:
```
   $threeVowels = $lowerVowel + $dotAbove + $dotBelow
   $threeVowels[*] + $upperVowel[*] + $threeVowels[*] + $threeVowels[*] => $2 + $1 + $3 + $4
   $threeVowels[*] + $threeVowels[*] + $upperVowel[*] + $threeVowels[*] => $3 + $1 + $2 + $4
   $threeVowels[*] + $threeVowels[*] + $threeVowels[*] + $upperVowel[*] => $4 + $1 + $2 + $3
   $threeVowels[*] + $upperVowel[*] + $threeVowels[*] => $2 + $1 + $3
   $threeVowels[*] + $threeVowels[*] + $upperVowel[*] => $3 + $1 + $2
   $threeVowels[*] + $upperVowel[*] => $2 + $1
```
This normalization rule could be replaced with:
```
   $threeVowels = $lowerVowel + $dotAbove + $dotBelow
   $?threeVowels[*] + $?threeVowels[*] + $threeVowels[*] + $upperVowel[*] => $4 + $1 + $2 + $3
```
...if it were assumed that $? means "match this, or the empty string if it doesn't exist". This would simplify a lot of reordering rules, although care would have to be taken to make sure that $1 and $2 don't cause any errors.

We'd also have to make sure that our code doesn't short-circuit. In other words, this rule could match on a string that is two, three, or four characters in length. We'd also need to consider greedy matching (match as much as possible) versus non-greedy.

It's a little tricky, but it's much easier than a complete normalization solution, and it can be used instead of the normalization trick.



---



# Key Labels #

**Difficulty:** Easy

It'd be nice to have a pre-determined variable that is used to "label" keys, for the virtual keyboard. For example:
```
   <SYS_KEYLABEL> => "qwerty...."
   <VK_SHIFT & SYS_KEYLABEL> => "QWERTY...."
   <VK_CONTROL & SYS_KEYLABEL> => "------...."
```
...in this case, the string is pre-defined; e.g., the first letter will be used for the "Q" key, etc. This will require arrays, of course, for labeling multi-char. keys.

This is also very useful in determining how to show the characters. For example, if VK\_CONTROL and VK\_ALT aren't used, then the letters can be printed in a larger font size, since there are only two labels on that key.

